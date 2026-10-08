#define CROW_MAIN

#include "web/coin_form_parser.hpp"
#include <crow.h>
#include <crow/mustache.h>
#include "config/app_config.hpp"
#include "coins/coin_repository.hpp"
#include <cstdint>
#include <cstdlib>
#include <format>
#include <fstream>
#include <sstream>
#include <optional>
#include <string>
#include <string_view>

namespace {

// Every HTML response declares its charset in the HTTP header —
// <meta charset> alone is only a fallback hint.
crow::response html_response(crow::mustache::rendered_template body) {
    crow::response res{std::move(body)};
    res.set_header("Content-Type", "text/html; charset=utf-8");
    return res;
}

} // namespace

int main() {
    crow::SimpleApp app;

    //Runtime override wins; compile-time path is the fallback (dev default)
    const char* env_template_dir = std::getenv("COINAPP_TEMPLATE_DIR");
    crow::mustache::set_global_base(env_template_dir ? env_template_dir
                                                     : COINAPP_TEMPLATE_DIR);

    auto config = config::EnvironmentLoader::load();

    CROW_ROUTE(app, "/static/css/style.css")([] {
        std::ifstream file{std::string{COINAPP_TEMPLATE_DIR} + "/../static/css/style.css"};
        if (!file) {
            return crow::response(404, "style.css not found");
        }
        std::ostringstream content;
        content << file.rdbuf();
        crow::response res{content.str()};
        res.set_header("Content-Type", "text/css; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/health")([] {
        return "OK";
    });

    CROW_ROUTE(app, "/")([] {
        crow::mustache::context ctx;
        ctx["title"] = "Coin App";
        ctx["message"] = "Backend, templates, htmx and PostgreSQL are working.";

        auto page = crow::mustache::load("index.html");
        return html_response(page.render(ctx));
    });

    CROW_ROUTE(app, "/coins")([] {
        auto config = config::EnvironmentLoader::load();
        coins::CoinRepository repo{static_cast<std::string>(config)};

        auto result = repo.list_all();

        if (!result) {
            switch (result.error()) {
                case coins::CoinRepositoryError::ConnectionFailed:
                    return crow::response(503, "Database unavailable");
                case coins::CoinRepositoryError::QueryFailed:
                    return crow::response(500, "Could not load coins");
                case coins::CoinRepositoryError::InvalidData:
                    return crow::response(400, "Invalid request data");
            }
        }

        auto coin_list = crow::json::wvalue::list();
        for (const auto& coin : result.value()) {
            crow::json::wvalue item;
            item["title"] = coin.title;
            item["country"] = coin.country;
            item["year"] = coin.year;
            item["metal"] = coin.metal;
            item["quantity"] = coin.quantity;
            item["purchase_price"] = std::format("{:.2f}", coin.purchase_price);
            item["purchase_currency"] = coin.purchase_currency;
            item["dealer"] = coin.dealer;
            
            // optionals: only present in the context when recorded
            if (coin.mint_mark)         item["mint_mark"]       = *coin.mint_mark;
            if (coin.grade)             item["grade"]           = *coin.grade;
            if (coin.fineness)          item["fineness"]        = std::format("{:.4f}", *coin.fineness);
            if (coin.total_weight_g)    item["total_weight_g"]  = std::format("{:.3f}", *coin.total_weight_g);
            if (coin.location)          item["location"]        = *coin.location;

            coin_list.push_back(std::move(item));
        }

        crow::mustache::context ctx;
        ctx["coins"] = std::move(coin_list);

        auto partial = crow::mustache::load("partials/coin_list.html");
        return html_response(partial.render(ctx));
    });

    CROW_ROUTE(app, "/api/references/search")([](const crow::request& req) {
        const char* q = req.url_params.get("q");

        crow::json::wvalue out;

        if (q == nullptr || std::string_view{q}.empty()) {
            out["matches"] = crow::json::wvalue::list();
            return crow::response{out};
        }

        auto config = config::EnvironmentLoader::load();
        coins::CoinRepository repo{static_cast<std::string>(config)};

        auto result = repo.search_references(q);

        if (!result) {
            switch (result.error()) {
                case coins::CoinRepositoryError::ConnectionFailed:
                    return crow::response(503, "Database unavailable");
                case coins::CoinRepositoryError::QueryFailed:
                    return crow::response(500, "Search failed");
                case coins::CoinRepositoryError::InvalidData:
                    return crow::response(400, "Invalid request data");
            }
        }

        auto matches = crow::json::wvalue::list();
        for (const auto& m : result.value()) {
            crow::json::wvalue item;
            item["id"]     = m.id;
            item["title"]  = m.title;
            item["country"] = m.country;
            item["metal"]   = m.metal;
            matches.push_back(std::move(item));
        }

        out["matches"] = std::move(matches);
        return crow::response{out};
    });

    CROW_ROUTE(app, "/coins").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        auto data = web::parse_new_coin_form(req.get_body_params());

        if (!data) {
            return crow::response(400, data.error());
        }

        auto config = config::EnvironmentLoader::load();
        coins::CoinRepository repo{static_cast<std::string>(config)};

        auto result = repo.add_coin(data.value());
        if (!result) {
            switch (result.error()) {
                case coins::CoinRepositoryError::ConnectionFailed:
                    return crow::response(503, "Database unavailable");
                case coins::CoinRepositoryError::QueryFailed:
                    return crow::response(500, "Could not add coin");
                case coins::CoinRepositoryError::InvalidData:
                    return crow::response(400, "Invalid or incomplete coin data");
            }
        }   

        crow::response res{303};
        res.set_header("Location", "/");
        return res;
    });

    CROW_ROUTE(app, "/coins/new")([] {
        auto page = crow::mustache::load("coin_form.html");
        return html_response(page.render());
    });

    //HTML-fragment variant of the reference search (HTMX, Option A)
    CROW_ROUTE(app, "/partials/references/search")([](const crow::request& req) {
        const char* q = req.url_params.get("q");

        crow::mustache::context ctx;
        auto matches = crow::json::wvalue::list();

        if(q != nullptr && !std::string_view{q}.empty()) {
            auto config = config::EnvironmentLoader::load();
            coins::CoinRepository repo{static_cast<std::string>(config)};
            if (auto result = repo.search_references(q)) {
                for (const auto& m : result.value()) {
                    crow::json::wvalue item;
                    item["id"]     = m.id;
                    item["title"]  = m.title;
                    item["country"] = m.country;
                    item["metal"]   = m.metal;
                    matches.push_back(std::move(item));
                }
            }
        }

        ctx["matches"] = std::move(matches);
        auto partial = crow::mustache::load("partials/reference_options.html");
        return html_response(partial.render(ctx));
    });

    app.port(static_cast<std::uint16_t>(config.web_port)).multithreaded().run();
}


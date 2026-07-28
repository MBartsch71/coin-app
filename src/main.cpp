#define CROW_MAIN
#include <crow.h>
#include <crow/mustache.h>
#include "config/app_config.hpp"
#include "coins/coin_repository.hpp"
#include <cstdint>
#include <format>
#include <string>

int main() {
    crow::SimpleApp app;

    crow::mustache::set_global_base(COINAPP_TEMPLATE_DIR);

    auto config = config::EnvironmentLoader::load();

    CROW_ROUTE(app, "/health")([] {
        return "OK";
    });

    CROW_ROUTE(app, "/")([] {
        crow::mustache::context ctx;
        ctx["title"] = "Coin App";
        ctx["message"] = "Backend, templates, htmx and PostgreSQL are working.";

        auto page = crow::mustache::load("index.html");
        return crow::response{page.render(ctx)};
    });

    CROW_ROUTE(app, "/coins")([] {
        auto config = config::EnvironmentLoader::load();
        coins::CoinRepository repo{static_cast<std::string>(config)};

        auto result = repo.list_all();

        if (!result) {
            switch (result.error()) {
                case coins::CoinRepositoryError::ConnectionFailed:
                    return crow::response(503, "Database anavailable");
                case coins::CoinRepositoryError::QueryFailed:
                    return crow::response(500, "Could not load coins");
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
        return crow::response{partial.render(ctx)};
    });

    app.port(static_cast<std::uint16_t>(config.web_port)).multithreaded().run();
}


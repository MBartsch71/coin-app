#define CROW_MAIN
#include <crow.h>
#include <crow/mustache.h>
#include "database/db_config.hpp"
#include "coins/coin_repository.hpp"
#include <string>

int main() {
    crow::SimpleApp app;

    crow::mustache::set_global_base(COINAPP_TEMPLATE_DIR);
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
        auto config = database::EnvironmentLoader::load();
        coins::CoinRepository repo{static_cast<std::string>(config)};

        auto result = repo.list_all();

        if (!result) {
            switch (result.error()) {
                case coins::CoinRepositoryError::ConnectionFailed:
                    return crow::response(500, "Database anavailable");
                case coins::CoinRepositoryError::QueryFailed:
                    return crow::response(500, "COuld not load coins");
            }
        }

        auto coin_list = crow::json::wvalue::list();
        for (const auto& coin : result.value()) {
            crow::json::wvalue item;
            item["title"] = coin.title;
            item["country"] = coin.country;
            item["year"] = coin.year;
            item["metal"] = coin.metal;
            coin_list.push_back(std::move(item));
        }

        crow::mustache::context ctx;
        ctx["coins"] = std::move(coin_list);

        auto partial = crow::mustache::load("partials/coin_list.html");
        return crow::response{partial.render(ctx)};
    });

    app.port(9000).multithreaded().run();
}


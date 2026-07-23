#define CROW_MAIN
#include <crow.h>
#include <crow/mustache.h>
#include <pqxx/pqxx>
#include <cstdlib>
#include <string>
#include "database/db_config.hpp"

int main() {
    crow::SimpleApp app;

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
        try {
            auto config = database::EnvironmentLoader::load();
            pqxx::connection conn{std::string(config)};

            pqxx::work tx(conn);

            auto result = tx.exec(
                "SELECT title, country, year, denomination, metak "
                "FROM coins "
                "ORDER BY id desc"
            );

            crow::mustache::context ctx;
            auto coins = crow::json::wvalue::list();

            for (const auto& row : result) {
                crow::json::wvalue item;
                item["title"] = row["title"].is_null() ? "" : row["title"].c_str();
                item["country"] = row["country"].is_null() ? "" : row["country"].c_str();
                item["year"] = row["year"].is_null() ? "" : row["year"].c_str();
                item["denomination"] = row["denomination"].is_null() ? "" : row["denomination"].c_str();
                item["metak"] = row["metak"].is_null() ? "" : row["metak"].c_str();

                coins.push_back(std::move(item));
            }
            tx.commit();
            ctx["coins"] = std::move(coins);

            auto partial = crow::mustache::load("partials/coin_list.html");
            return crow::response{partial.render(ctx)};
        } catch (const std::exception& ex) {
            return crow::response(500, std::string("Database error: ") + ex.what()); 
        }
    });

    app.port(9000).multithreaded().run();
}


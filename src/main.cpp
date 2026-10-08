#define CROW_MAIN

#include <crow.h>
#include <crow/mustache.h>
#include "config/app_config.hpp"
#include "coins/coin_repository.hpp"
#include "web/coin_handlers.hpp"
#include "coins/coin_repository.hpp"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

int main() {
    crow::SimpleApp app;

    //Runtime override wins; compile-time path is the fallback (dev default)
    const char* env_template_dir = std::getenv("COINAPP_TEMPLATE_DIR");
    crow::mustache::set_global_base(env_template_dir ? env_template_dir
                                                     : COINAPP_TEMPLATE_DIR);

    auto config = config::EnvironmentLoader::load();
    coins::CoinRepository repo{static_cast<std::string>(config)};

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
        return web::html_response(page.render(ctx));
    });

    CROW_ROUTE(app, "/coins")([&repo] {
        return web::list_coins(repo);
    });

    CROW_ROUTE(app, "/coins").methods(crow::HTTPMethod::Post)(
        [&repo](const crow::request& req) {
            return web::create_coin(req, repo);
    });

    CROW_ROUTE(app, "/coins/new")([] {
        auto page = crow::mustache::load("coin_form.html");
        return web::html_response(page.render());
    });

    CROW_ROUTE(app, "/api/references/search")([&repo](const crow::request& req) {
        return web::search_references_json(req, repo);
    });

    CROW_ROUTE(app, "/partials/references/search")([&repo](const crow::request& req) {
        return web::search_references_partial(req, repo);
    });

    app.port(static_cast<std::uint16_t>(config.web_port)).multithreaded().run();

}



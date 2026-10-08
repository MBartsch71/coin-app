#ifndef COINAPP_WEB_COIN_HANDLERS_HPP
#define COINAPP_WEB_COIN_HANDLERS_HPP

#include "coins/coin_repository.hpp"
#include "web/coin_form_parser.hpp"
#include <crow.h>
#include <crow/mustache.h>

namespace web {

    // Every HTML response declares its charset in the HTTp header -
    // <meta charset> alone is only a fallback hint.
    inline crow::response html_response(crow::mustache::rendered_template body) {
        crow::response res{std::move(body)};
        res.set_header("Content-Type", "text/html; charset=utf-8");
        return res;
    }

    // Get /coins -> rendered coin-list partial, or 500/503
    auto list_coins(const coins::CoinRepository& repo) -> crow::response;

    // POST /coins -> 300 redirect to /, or 400/500/503
    auto create_coin(const crow::request& req,
                     const coins::CoinRepository& repo) -> crow::response;

    // GET /api/references/search?q=... -> JSON {"matches": [...]}
    auto search_references_json(const crow::request& req,
                                const coins::CoinRepository& repo) -> crow::response;

    // GET /partials/references/search?q=... -> HTML <option> fragment (HTMX)
    auto search_references_partial(const crow::request& req,
                                   const coins::CoinRepository& repo) -> crow::response;

}
    
#endif //namespace web
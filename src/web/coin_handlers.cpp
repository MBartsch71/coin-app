#include "web/coin_handlers.hpp"
#include <format>
#include <iostream>
#include <string_view>

namespace web {

    namespace {

        // One mapping from repository errors to HTTP responses.
        // query_msg differentiates the 500 text per handler, as before.
        auto to_response(coins::CoinRepositoryError err, const char* query_msg) 
            -> crow::response {
                switch (err) {
                    case coins::CoinRepositoryError::ConnectionFailed:
                        return crow::response(503, "Database unavailable");
                    case coins::CoinRepositoryError::QueryFailed:
                        return crow::response(500, std::format("{} failed", query_msg));
                    case coins::CoinRepositoryError::InvalidData:
                        return crow::response(400, "Invalid request data");
                }
            return crow::response(500);
        }
            
        // Coin -> mustache/json context item. Optiuonals only appear
        // in the context when recorded.
        auto coin_to_wvalue(const coins::Coin& coin) -> crow::json::wvalue {
            crow::json::wvalue item;
            item["title"] = coin.title;
            item["country"] = coin.country;
            item["year"] = coin.year;
            item["metal"] = coin.metal;
            item["quantity"] = coin.quantity;
            item["purchase_price"] = std::format("{:.2f}", coin.purchase_price);
            item["purchase_currency"] = coin.purchase_currency;
            item["dealer"] = coin.dealer;   

            if (coin.mint_mark)         item["mint_mark"]       = *coin.mint_mark;
            if (coin.grade)             item["grade"]           = *coin.grade;
            if (coin.fineness)          item["fineness"]        = std::format("{:.4f}", *coin.fineness);
            if (coin.total_weight_g)    item["total_weight_g"]  = std::format("{:.3f}", *coin.total_weight_g);
            if (coin.location)          item["location"]        = *coin.location;

            return item;
        }

        auto match_to_wvalue(const coins::ReferenceMatch& m) -> crow::json::wvalue {
            crow::json::wvalue item;
            item["id"]      = m.id;
            item["title"]   = m.title;
            item["country"] = m.country;
            item["metal"]   = m.metal;
            return item;
        }
    } // namespace

    auto list_coins(const coins::CoinRepository& repo) -> crow::response {
        auto result = repo.list_all();
        if (!result) {
            return to_response(result.error(), "Could not load coins");
        }

        auto coin_list = crow::json::wvalue::list();
        for (const auto& coin : result.value()) {
            coin_list.push_back(coin_to_wvalue(coin));
        }

        crow::mustache::context ctx;
        ctx["coins"] = std::move(coin_list);

        auto partial = crow::mustache::load("partials/coin_list.html");
        return html_response(partial.render(ctx));
    }

    auto create_coin(const crow::request& req,
                     const coins::CoinRepository& repo) -> crow::response {

        auto data = parse_new_coin_form(req.get_body_params());
        if (!data) {
            return crow::response(400, data.error());
        }

        auto result = repo.add_coin(*data);
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
    }

    auto search_references_json(const crow::request& req,
                                const coins::CoinRepository& repo) -> crow::response {

        const char* q = req.url_params.get("q");
        crow::json::wvalue out;

        if (q == nullptr || std::string_view{q}.empty()) {
            out["matches"] = crow::json::wvalue::list();
            return crow::response{out};
        }

        auto result = repo.search_references(q);
        if (!result) {
            return to_response(result.error(), "Search failed");
        }

        auto matches = crow::json::wvalue::list();
        for (const auto& m : result.value()) {
            matches.push_back(match_to_wvalue(m));
        }

        out["matches"] = std::move(matches);
        return crow::response{out};
    }

    auto search_references_partial(const crow::request& req,
                                   const coins::CoinRepository& repo) -> crow::response {
        const char* q = req.url_params.get("q");

        crow::mustache::context ctx;
        auto matches = crow::json::wvalue::list();

        if (q != nullptr && !std::string_view{q}.empty()) {
            if (auto result = repo.search_references(q)) {
                for (const auto& m : result.value()) {
                    matches.push_back(match_to_wvalue(m));
                }
            } else {
                // Deliberate UX; search failure renders an empty dropdown,
                // but we still log it server-side.
                std::cerr << "reference search partial failed\n";
            }
        }

        ctx["matches"] = std::move(matches);
        auto partial = crow::mustache::load("partials/reference_options.html");
        return html_response(partial.render(ctx));
    }
}


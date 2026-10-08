#ifndef COINAPP_WEB_COIN_FORM_PARSER_HPP
#define COINAPP_WEB_COIN_FORM_PARSER_HPP

#include "coins/coin.hpp"
#include <crow/query_string.h>
#include <expected>
#include <string>

namespace web {
    
    // Parses the add-coin form body into NewCoinData.
    // On failure the error is a user-facing message; the route maps it to 400.
    auto parse_new_coin_form(const crow::query_string& form) -> std::expected<coins::NewCoinData, std::string>;
}

#endif // COINAPP_WEB_COIN_FORM_PARSER_HPP
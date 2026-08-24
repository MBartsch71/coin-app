#ifndef COINAPP_COIN_HPP
#define COINAPP_COIN_HPP

#include <string>
#include <optional> 

namespace coins {

    struct Coin {
        std::string title;
        std::string country;
        int year;
        std::string metal;
        int quantity;
        double purchase_price;
        std::string purchase_currency;
        std::string dealer;

        std::optional<std::string> mint_mark;
        std::optional<std::string> grade;
        std::optional<double> fineness;
        std::optional<double> total_weight_g; 
        std::optional<std::string> location;
    };

    struct ReferenceMatch {
        int64_t id;
        std::string title;
        std::string country;
        std::string metal;
    };
}

#endif // COINAPP_COIN_HPP
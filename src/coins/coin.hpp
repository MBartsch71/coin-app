#ifndef COINAPP_COIN_HPP
#define COINAPP_COIN_HPP

#include <string>

namespace coins {

    struct Coin {
        std::string title;
        std::string country;
        int year;
        std::string metal;
    };
}

#endif // COINAPP_COIN_HPP
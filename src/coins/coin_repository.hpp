#ifndef COINAPP_COINS_COIN_REPOSITORY_HPP
#define COINAPP_COINS_COIN_REPOSITORY_HPP

#include "coins/coin.hpp"
#include <expected>
#include <string>
#include <vector>

namespace coins {

    enum class CoinRepositoryError {
        ConnectionFailed,
        QueryFailed
    };

    class CoinRepository {
        public:
            explicit CoinRepository(std::string connection_string);

            auto list_all() const -> std::expected<std::vector<Coin>, CoinRepositoryError>;
        
            private:
                std::string connection_string_;
    };
}

#endif // COINAPP_COINS_COIN_REPOSITORY_HPP
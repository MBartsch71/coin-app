#ifndef COINAPP_COINS_COIN_REPOSITORY_HPP
#define COINAPP_COINS_COIN_REPOSITORY_HPP

#include "coins/coin.hpp"
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace coins {

    enum class CoinRepositoryError {
        ConnectionFailed,
        QueryFailed,
        InvalidData
    };

    class CoinRepository {
        public:
            explicit CoinRepository(std::string connection_string);

            auto list_all() const -> std::expected<std::vector<Coin>, CoinRepositoryError>;

            auto search_references(std::string_view query) const -> std::expected<std::vector<ReferenceMatch>, CoinRepositoryError>;

            auto add_coin(const NewCoinData& data) const -> std::expected<int64_t, CoinRepositoryError>;
        
        private:
            std::string connection_string_;
    };
}

#endif // COINAPP_COINS_COIN_REPOSITORY_HPP
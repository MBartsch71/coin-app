#include "coins/coin_repository.hpp"
#include <pqxx/pqxx>
#include <utility>

namespace coins {

    CoinRepository::CoinRepository(std::string connection_string)
        : connection_string_{std::move(connection_string)} {}

    auto CoinRepository::list_all() const -> std::expected<std::vector<Coin>, CoinRepositoryError> {
        try {
            pqxx::connection conn{connection_string_};
            pqxx::work tx{conn};

            auto result = tx.exec(
                "SELECT COALESCE(c.title_override, r.title) AS title, "
                "       r.country, "
                "       c.year, "
                "       r.primary_metal::text AS metal "
                "FROM coins c "
                "JOIN coin_references r ON c.reference_id = r.id "
                "ORDER BY c.id DESC"
            );

            std::vector<Coin> coins;
            coins.reserve(result.size());

            for (const auto& row : result) {
                coins.push_back(Coin{
                    .title = row["title"].c_str(),
                    .country = row["country"].c_str(),
                    .year = row["year"].as<int>(),
                    .metal = row["metal"].c_str()
                });
            }

            return coins;
        } catch (const pqxx::broken_connection&) {
            return std::unexpected{CoinRepositoryError::ConnectionFailed};
        } catch (const std::exception&) {
            return std::unexpected{CoinRepositoryError::QueryFailed};
        }
    } 
}
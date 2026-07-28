#include "coins/coin_repository.hpp"
#include <pqxx/pqxx>
#include <utility>

namespace coins {

    CoinRepository::CoinRepository(std::string connection_string)
        : connection_string_{std::move(connection_string)} {}
    
        namespace {
            // NULL -> nullopt, in one place. row as<T> does the type conversion
            template <typename T>
            std::optional<T> nullable(const pqxx::row& row, const char* column) {
                if (row[column].is_null()) {
                    return std::nullopt;
                }
                return row[column].as<T>();
            }
        } // namespace

    auto CoinRepository::list_all() const -> std::expected<std::vector<Coin>, CoinRepositoryError> {
        try {
            pqxx::connection conn{connection_string_};
            pqxx::work tx{conn};

            auto result = tx.exec(
                "SELECT COALESCE(c.title_override, r.title) AS title, "
                "       r.country, "
                "       c.year, "
                "       r.primary_metal::text AS metal, "
                "       c.quantity, "
                "       c.purchase_price, "
                "       c.purchase_currency, "
                "       COALESCE(c.dealer, '') AS dealer, "
                "       c.mint_mark, "
                "       c.grade, "
                "       r.fineness, "
                "       r.total_weight_g, "
                "       sl.name AS location "
                "FROM coins c "
                "JOIN coin_references r ON c.reference_id = r.id "
                "LEFT JOIN storage_locations sl ON c.storage_location_id = sl.id "
                "ORDER BY c.id DESC"
            );

            std::vector<Coin> coins;
            coins.reserve(result.size());

            for (const auto& row : result) {
                coins.push_back(Coin{
                    .title = row["title"].c_str(),
                    .country = row["country"].c_str(),
                    .year = row["year"].as<int>(),
                    .metal = row["metal"].c_str(),
                    .quantity = row["quantity"].as<int>(),
                    .purchase_price = row["purchase_price"].as<double>(),
                    .purchase_currency = row["purchase_currency"].c_str(),
                    .dealer = row["dealer"].c_str(),
                    .mint_mark = nullable<std::string>(row, "mint_mark"),
                    .grade = nullable<std::string>(row, "grade"),
                    .fineness = nullable<double>(row, "fineness"),
                    .total_weight_g = nullable<double>(row, "total_weight_g"),
                    .location = nullable<std::string>(row, "location"),
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
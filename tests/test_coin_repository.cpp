#include <catch2/catch_test_macros.hpp>
#include "config/app_config.hpp"
#include "coins/coin_repository.hpp"
#include "coin_fixtures.hpp"
#include <pqxx/pqxx>
#include <algorithm>
#include <string>

TEST_CASE("CoinRepository lists coins from database" ,
            "[coin_repository][integration]") {
                auto config = config::EnvironmentLoader::load();
                std::string conn_str = static_cast<std::string>(config);

                pqxx::connection conn{conn_str};
                test_fixtures::clean(conn);
                test_fixtures::insert_sample(conn);

                coins::CoinRepository repo{conn_str};
                auto result = repo.list_all();

                REQUIRE(result.has_value());
                const auto& all = result.value();

                auto it = std::ranges::find_if(all, [](const coins::Coin& c) {
                    return c.title == "TEST_Krügerrand";
                });

                REQUIRE(it != all.end());
                REQUIRE(it->country == "South Africa");
                REQUIRE(it->year == 1967);
                REQUIRE(it->metal == "Gold");

                test_fixtures::clean(conn);
}

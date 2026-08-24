#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
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

                // Phase 0 fields
                REQUIRE(it->country == "South Africa");
                REQUIRE(it->year == 1967);
                REQUIRE(it->metal == "Gold");

                // Phase 1: guaranteed facts
                REQUIRE(it->quantity == 13);
                REQUIRE(it->purchase_price == Catch::Approx(145.00));
                REQUIRE(it->purchase_currency == "CHF");
                REQUIRE(it->dealer == "TEST_Dealer");

                //Phase 1: optional
                REQUIRE(it->mint_mark.has_value());
                REQUIRE(it->mint_mark.value() == "TESTMM");

                REQUIRE(it->grade.has_value());
                REQUIRE(it->grade.value() == "MS65");

                REQUIRE(it->fineness.has_value());
                REQUIRE(it->fineness.value() == Catch::Approx(0.9167));
                
                REQUIRE(it->total_weight_g.has_value());
                REQUIRE(it->total_weight_g.value() == Catch::Approx(33.931));
                
                REQUIRE(it->location.has_value());
                REQUIRE(it->location.value() == "TEST_Vault");
                test_fixtures::clean(conn);
}

TEST_CASE("CoinRepository searches references by title", 
        "[coin_repository][integration]") {

            auto config = config::EnvironmentLoader::load();
            std::string conn_str = static_cast<std::string>(config);

            pqxx::connection conn{conn_str};
            test_fixtures::clean(conn);
            test_fixtures::insert_sample(conn);

            coins::CoinRepository repo{conn_str};
            auto result = repo.search_references("Krüg");

            REQUIRE(result.has_value());
            REQUIRE(result->size() == 1);
            REQUIRE(result->at(0).title == "TEST_Krügerrand");
            REQUIRE(result->at(0).country == "South Africa");
            REQUIRE(result->at(0).metal == "Gold");

            test_fixtures::clean(conn);
}

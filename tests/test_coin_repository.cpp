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

TEST_CASE("CoinRepository adds a coin with a new reference in one transaction",
            "[coin_repository][integration]") {

            auto config = config::EnvironmentLoader::load();
            std::string conn_str = static_cast<std::string>(config);

            pqxx::connection conn{conn_str};
            test_fixtures::clean(conn);

            coins::CoinRepository repo{conn_str};
           
            coins::NewCoinData data;
            data.ref_title = "TEST_MapleLeaf";
            data.ref_country = "Canada";
            data.ref_metal = "Gold";
            data.year = 2023;
            data.quantity = 2;
            data.purchase_price = 2100.50;
            data.purchase_currency = "CHF"; 
            data.dealer = "TEST_Shop";   

            auto result = repo.add_coin(data);
            REQUIRE(result.has_value());

            auto all = repo.list_all();
            REQUIRE(all.has_value());   

            auto it = std::ranges::find_if(all.value(), [](const coins::Coin& c) {
                return c.title == "TEST_MapleLeaf";
            });
            REQUIRE(it != all->end());
            REQUIRE(it->country == "Canada");
            REQUIRE(it->year == 2023);
            REQUIRE(it->quantity == 2);
            REQUIRE(it->purchase_price == Catch::Approx(2100.50));
            REQUIRE(it->dealer == "TEST_Shop");

            test_fixtures::clean(conn);
}

TEST_CASE("CoinRepository adds a coin against an existing reference", "[coin_repository][integration]") {

    auto config = config::EnvironmentLoader::load();
    std::string conn_str = static_cast<std::string>(config);

    pqxx::connection conn{conn_str};
    test_fixtures::clean(conn);
    test_fixtures::insert_sample(conn);

    coins::CoinRepository repo{conn_str};

    auto matches = repo.search_references("Krüg");
    REQUIRE(matches.has_value());
    REQUIRE(matches->size() == 1);

    coins::NewCoinData data;
    data.reference_id = matches->at(0).id;
    data.year = 2021;
    data.quantity = 1;
    data.purchase_price = 1875.00;
    data.purchase_currency = "CHF";
    data.dealer = "TEST_Dealer2";

    auto result = repo.add_coin(data);
    REQUIRE(result.has_value());

    auto all = repo.list_all();
    REQUIRE(all.has_value());
    auto it = std::ranges::find_if(all.value(), [](const coins::Coin& c) {
        return c.dealer == "TEST_Dealer2";
    });
    REQUIRE(it != all->end());
    REQUIRE(it->title == "TEST_Krügerrand");
    REQUIRE(it->year == 2021);

    test_fixtures::clean(conn);
}

TEST_CASE("CoinRepository adds a coin with optional fields", "[coin_repository][integration]") {

    auto config = config::EnvironmentLoader::load();
    std::string conn_str = static_cast<std::string>(config);

    pqxx::connection conn{conn_str};
    test_fixtures::clean(conn);

    int64_t loc_id;
    {
        pqxx::work tx{conn};
        loc_id = tx.exec_params(
            "INSERT INTO storage_locations (name) VALUES ('TEST_Vault2') "
            "RETURNING id").one_row()["id"].as<int64_t>();
        tx.commit();
    }

    coins::CoinRepository repo{conn_str};

    // 1. With optionals set
    coins::NewCoinData data;
    data.ref_title = "TEST_Britannia";
    data.ref_country = "United Kingdom";
    data.ref_metal = "Silver";
    data.year = 2021;
    data.quantity = 10;
    data.purchase_price = 32.75;
    data.purchase_currency = "CHF";
    data.dealer = "TEST_Shop";
    data.mint_mark = "TESTMM21";
    data.grade = "MS65";
    data.storage_location_id = loc_id;

    auto result = repo.add_coin(data);
    REQUIRE(result.has_value());

    auto all = repo.list_all();
    REQUIRE(all.has_value());

    auto it = std::ranges::find_if(all.value(), [](const coins::Coin& c) {
        return c.title == "TEST_Britannia";
    });
    REQUIRE(it != all->end());
    REQUIRE(it->mint_mark.has_value());
    REQUIRE(it->mint_mark.value() == "TESTMM21");
    REQUIRE(it->grade.has_value());
    REQUIRE(it->grade.value() == "MS65");
    REQUIRE(it->location.has_value());
    REQUIRE(it->location.value() == "TEST_Vault2");

    // 2. Without optionls -> stored as NULL, read back as nullopt
    coins::NewCoinData bare;
    bare.ref_title = "TEST_BareCoin";
    bare.ref_country = "Nowhere";
    bare.ref_metal = "Copper";
    bare.year = 2000;
    bare.quantity = 11;
    bare.purchase_price = 1.00;
    bare.purchase_currency = "CHF";
    bare.dealer = "";

    auto result2 = repo.add_coin(bare);
    REQUIRE(result2.has_value());

    auto all2 = repo.list_all();
    REQUIRE(all2.has_value());

    auto it2 = std::ranges::find_if(all2.value(), [](const coins::Coin& c) {
        return c.title == "TEST_BareCoin";
    });
    REQUIRE(it2 != all2->end());
    REQUIRE(!it2->mint_mark.has_value());
    REQUIRE(!it2->grade.has_value());
    REQUIRE(!it2->location.has_value());

}
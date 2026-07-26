#include <catch2/catch_test_macros.hpp>
#include "config/app_config.hpp"
#include "coins/coin_repository.hpp"
#include <pqxx/pqxx>
#include <algorithm>
#include <string>

namespace {

    void clean_fixtures(pqxx::connection& conn) {
        pqxx::work tx(conn);
        tx.exec("DELETE FROM coins WHERE reference_id IN "
                "(SELECT id from coin_references WHERE title like 'TEST_%')");
        tx.exec("DELETE FROM coin_references WHERE title LIKE 'TEST_%'");
        tx.commit();
    }

    void insert_fixture(pqxx::connection& conn) {
        pqxx::work tx(conn);
        auto row = tx.exec(
            "INSERT INTO coin_references (title,country, primary_metal) "
            "VALUES ('TEST_Krügerrand', 'South Africa', 'Gold') RETURNING id"
        ).one_row();
        int reference_id = row["id"].as<int>();

        tx.exec("INSERT INTO coins (reference_id, year) VALUES ("
          +  tx.quote(reference_id) + ", 1967)");
        tx.commit();
    }

}

TEST_CASE("CoinRepository lists coins from database" ,
            "[coin_repository][integration]") {
                auto config = config::EnvironmentLoader::load();
                std::string conn_str = static_cast<std::string>(config);
                pqxx::connection conn{conn_str};

                clean_fixtures(conn);
                insert_fixture(conn);

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

                clean_fixtures(conn);
}

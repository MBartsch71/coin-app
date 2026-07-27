#ifndef COINAPP_TESTS_COIN_FIXTURES_HPP
#define COINAPP_TESTS_COIN_FIXTURES_HPP

#include <pqxx/pqxx>

namespace test_fixtures {

    // Remove all fixture rows (tagged 'TEST_%')
    // Run before AND after each test - self-healing after crashes
    inline void clean(pqxx::connection& conn) {
        pqxx::work tx{conn};
        tx.exec("DELETE FROM coins WHERE reference_id IN "
                "(SELECT id FROM coin_references WHERE title LIKE 'TEST_%')");
        tx.exec("DELETE FROM coin_references WHERE title LIKE 'TEST_%'");
        tx.exec("DELETE FROM storage_locations WHERE name LIKE 'TEST_%'");
        tx.commit();
    }

    // Inserts a FULL fixture coin: catalog reference + owned coin + location.
    // All assertable values carry TEST_ markers or distinctive numbers.
    inline void insert_sample(pqxx::connection& conn) {
        pqxx::work tx{conn};

        auto loc = tx.exec(
            "INSERT INTO storage_locations (name, description) "
            "VALUES ('TEST_Vault', 'fixture location') RETURNING id"
        ).one_row();
        int loc_id = loc["id"].as<int>();

        auto ref = tx.exec(
            "INSERT INTO coin_references "
            "(title, country, primary_metal, composition, fineness, total_weight_g) "
            "VALUES ('TEST_Krügerrand', 'South Africa', 'Gold', "
            "'22 karat gold', 0.9167, 33.931) RETURNING id"
        ).one_row();
        int ref_id = ref["id"].as<int>();

        tx.exec("INSERT INTO coins (reference_id, storage_location_id, year, "
                "mint_mark, grade, quantity, purchase_price, purchase_currency, dealer) "
                "VALUES (" + tx.quote(ref_id) + ", " + tx.quote(loc_id) +
                ", 1967, 'TESTMM', 'MS65', 13, 145.00, 'CHF', 'TEST_Dealer')" 
        );
        tx.commit();
    }

} // namespace test_fixtures

#endif  // COINAPP_TESTS_COIN_FIXTURES_HPP

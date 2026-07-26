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
        tx.commit();
    }

    // Inserts one catalog reference + one owned coin, tagges as fixture data
    inline void insert_sample(pqxx::connection& conn) {
        pqxx::work tx{conn};
        auto row = tx.exec(
            "INSERT INTO coin_references (title, country, primary_metal) "
            "VALUES ('TEST_Krügerrand', 'South Africa', 'Gold') RETURNING id"
        ).one_row();
        int ref_id = row["id"].as<int>();

        tx.exec("INSERT INTO coins (reference_id, year) VALUES ("
                + tx.quote(ref_id) + ", 1967)");
        tx.commit();
    }

} // namespace test_fixtures

#endif  // COINAPP_TESTS_COIN_FIXTURES_HPP

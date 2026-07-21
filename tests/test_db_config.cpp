#include <catch2/catch_test_macros.hpp>
#include "database/db_config.hpp" 
#include <cstdlib>

TEST_CASE("Database connection string formatting", "[db_config]") {
    unsetenv("COINAPP_DB_HOST");
    unsetenv("COINAPP_DB_PORT");
    unsetenv("COINAPP_DB_NAME");
    unsetenv("COINAPP_DB_USER");
    unsetenv("COINAPP_DB_PASSWORD");

    SECTION("Defaults are used when environment is empty") {
        std::string conn = database::get_connection_string();
        REQUIRE(conn == "host=127.0.0.1 port=5432 dbname=coin_catalog user=coinapp password=coinappdev");
    }

    SECTION("Environment variables override defaults correctly") {
        setenv("COINAPP_DB_PORT", "5433", 1);
        setenv("COINAPP_DB_USER", "postgres", 1);
        
        std::string conn = database::get_connection_string();
        REQUIRE(conn == "host=127.0.0.1 port=5433 dbname=coin_catalog user=postgres password=coinappdev");

        unsetenv("COINAPP_DB_PORT");
        unsetenv("COINAPP_DB_USER");
    }
}
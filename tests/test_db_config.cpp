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
        auto config = database::EnvironmentLoader::load();
        REQUIRE(config.host == "127.0.0.1");
        REQUIRE(config.port == "5432");
        REQUIRE(config.dbname == "coin_catalog");
        REQUIRE(config.user == "coinapp");
        REQUIRE(config.password == "coinappdev");
    }

    SECTION("Environment variables override defaults correctly") {
        setenv("COINAPP_DB_PORT", "5433", 1);
        setenv("COINAPP_DB_USER", "postgres", 1);
        
        auto config = database::EnvironmentLoader::load();
        REQUIRE(config.port == "5433");
        REQUIRE(config.user == "postgres");
        REQUIRE(config.host == "127.0.0.1");
        REQUIRE(config.dbname == "coin_catalog");

        unsetenv("COINAPP_DB_PORT");
        unsetenv("COINAPP_DB_USER");
    }

    SECTION("Connection string uses default values") {
        auto config = database::EnvironmentLoader::load();
        std::string conn_str = static_cast<std::string>(config);
        REQUIRE(conn_str == "host=127.0.0.1 port=5432 dbname=coin_catalog user=coinapp password=coinappdev");
    }

    SECTION("Connection string refelcts environment ovverrides") {
        setenv("COINAPP_DB_HOST", "db.example.com",1);
        setenv("COINAPP_DB_PORT", "6533", 1);

        auto config = database::EnvironmentLoader::load();
        std::string conn_str = static_cast<std::string>(config);
        REQUIRE(conn_str == "host=db.example.com port=6533 dbname=coin_catalog user=coinapp password=coinappdev");

        unsetenv("COINAPP_DB_HOST");
        unsetenv("COINAPP_DB_PORT");    
    }

}
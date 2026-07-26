#include <catch2/catch_test_macros.hpp>
#include "database/db_config.hpp" 
#include <cstdlib>
#include <string>

TEST_CASE("Database connection string formatting", "[db_config]") {
    unsetenv("COINAPP_DB_HOST");
    unsetenv("COINAPP_DB_PORT");
    unsetenv("COINAPP_DB_NAME");
    unsetenv("COINAPP_DB_USER");
    unsetenv("COINAPP_DB_PASSWORD");
    unsetenv("COINAPP_PORT");

    SECTION("Defaults are used when environment is empty") {
        auto config = database::EnvironmentLoader::load();
        REQUIRE(config.host == "127.0.0.1");
        REQUIRE(config.port == "5432");
        REQUIRE(config.dbname == "coin_catalog_dev");
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
        REQUIRE(config.dbname == "coin_catalog_dev");

        unsetenv("COINAPP_DB_PORT");
        unsetenv("COINAPP_DB_USER");
    }

    SECTION("Connection string uses default values") {
        auto config = database::EnvironmentLoader::load();
        std::string conn_str = static_cast<std::string>(config);
        REQUIRE(conn_str == "host=127.0.0.1 port=5432 dbname=coin_catalog_dev user=coinapp password=coinappdev");
    }

    SECTION("Connection string reflects environment ovverrides") {
        setenv("COINAPP_DB_HOST", "db.example.com",1);
        setenv("COINAPP_DB_PORT", "6533", 1);

        auto config = database::EnvironmentLoader::load();
        std::string conn_str = static_cast<std::string>(config);
        REQUIRE(conn_str == "host=db.example.com port=6533 dbname=coin_catalog_dev user=coinapp password=coinappdev");

        unsetenv("COINAPP_DB_HOST");
        unsetenv("COINAPP_DB_PORT");    
    }

    SECTION("Web port defaults to 9000") {
        auto config = database::EnvironmentLoader::load();
        REQUIRE(config.web_port== 9000);
    }

    SECTION("Web port can be overridden by environment") {
        setenv("COINAPP_PORT", "9001", 1);

        auto config = database::EnvironmentLoader::load();
        REQUIRE(config.web_port == 9001);

        unsetenv("COINAPP_PORT");
    }

}
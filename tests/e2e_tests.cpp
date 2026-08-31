#include <catch2/catch_test_macros.hpp>
#include <cpr/cpr.h>
#include <pqxx/pqxx>
#include "config/app_config.hpp"
#include "coin_fixtures.hpp"

#include <chrono>
#include <string>
#include <thread>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {

    // RAII guard: starts the real app binary as a subprocess,
    // kills and reaps it when the test ends - pass or fail
    class AppProcess {
        public:
            AppProcess(const char* wrapper, const char* env_file, const char* binary) {
                pid_ = fork();
                if (pid_ == 0) {
                    // Child: the wrapper's exec REPLACES this .process with coin_app,
                    // so pid_ stays valid for the app itself
                    execl(wrapper, wrapper, env_file, binary, static_cast<char*>(nullptr));
                    _exit(127); // only reached if exec failed
                }
            }

            ~AppProcess() {
                if (pid_ > 0) {
                    kill(pid_, SIGTERM);
                    waitpid(pid_, nullptr, 0);
                }
            }

            AppProcess(const AppProcess&) = delete;
            AppProcess& operator=(const AppProcess&) = delete;

        private:
            pid_t pid_ = -1;
    };

    bool wait_until_ready(const std::string& base_url) {
        for (int attempt = 0; attempt < 50; ++attempt) {
            auto r = cpr::Get(cpr::Url{base_url + "/health"},
                              cpr::Timeout{std::chrono::milliseconds{200}});
            if (r.status_code == 200) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds{100});
        }
        return false;
    }
} // namespace

TEST_CASE("E2E: full stack serves requests", "[e2e]") {
    auto config = config::EnvironmentLoader::load();
    const std::string base_url =
        "http://127.0.0.1:" + std::to_string(config.web_port);
    const std::string conn_str = static_cast<std::string>(config);

    //Seed the disposable test database
    {
        pqxx::connection conn{conn_str};
        test_fixtures::clean(conn);
        test_fixtures::insert_sample(conn);
    }

    // Start the real application
    AppProcess app{COINAPP_ENV_WRAPPER, COINAPP_TEST_ENV, COINAPP_BINARY};
    REQUIRE(wait_until_ready(base_url));

    // 1. Health check
    auto health = cpr::Get(cpr::Url{base_url + "/health"});
    REQUIRE(health.status_code == 200);
    REQUIRE(health.text == "OK");

    // 2. The /coins page renders the seeded coin
    auto coins = cpr::Get(cpr::Url{base_url + "/coins"});
    REQUIRE(coins.status_code == 200);

    // Existing field (passing since Phase 0)
    CHECK(coins.text.find("TEST_Krügerrand") != std::string::npos);
    CHECK(coins.text.find("South Africa")    != std::string::npos);
    CHECK(coins.text.find("1967")            != std::string::npos);
    CHECK(coins.text.find("Gold")            != std::string::npos);

    // Phase 1. FULL coin information 
    CHECK(coins.text.find("TEST_Vault") != std::string::npos); 
    CHECK(coins.text.find("MS65") != std::string::npos); 
    CHECK(coins.text.find("TESTMM") != std::string::npos); 
    CHECK(coins.text.find("TEST_Dealer") != std::string::npos); 
    CHECK(coins.text.find("145.00") != std::string::npos); 
    CHECK(coins.text.find("0.9167") != std::string::npos); 
    CHECK(coins.text.find("33.931") != std::string::npos); 

    // Cleanup (the app is killed by RAII at scope end)
    pqxx::connection conn{conn_str};
    test_fixtures::clean(conn);
}

TEST_CASE("E2E: reference search endpoint returns matches", "[e2e]") {
    auto config = config::EnvironmentLoader::load();
    const std::string base_url =
        "http://127.0.0.1:" + std::to_string(config.web_port);
    const std::string conn_str = static_cast<std::string>(config);

    {
        pqxx::connection conn{conn_str};
        test_fixtures::clean(conn);
        test_fixtures::insert_sample(conn);
    }

    AppProcess app{COINAPP_ENV_WRAPPER, COINAPP_TEST_ENV, COINAPP_BINARY};
    REQUIRE(wait_until_ready(base_url));

    auto r = cpr::Get(cpr::Url{base_url + "/api/references/search"},
                      cpr::Parameters{{"q", "Krüg"}});
    REQUIRE(r.status_code == 200);

    CHECK(r.text.find("TEST_Krügerrand") != std::string::npos);
    CHECK(r.text.find("South Africa") != std::string::npos);
    CHECK(r.text.find("Gold") != std::string::npos);

    pqxx::connection conn{conn_str};
    test_fixtures::clean(conn);
}

TEST_CASE("E2E: adding a coin creates reference and collection item", "[e2e]") {
    auto config = config::EnvironmentLoader::load();
    const std::string base_url =
        "http://127.0.0.1:" + std::to_string(config.web_port);
    const std::string conn_str = static_cast<std::string>(config);

    {
        pqxx::connection conn{conn_str};
        test_fixtures::clean(conn);
    }

    AppProcess app{COINAPP_ENV_WRAPPER, COINAPP_TEST_ENV, COINAPP_BINARY};
    REQUIRE(wait_until_ready(base_url));

    auto post = cpr::Post(
        cpr::Url{base_url + "/coins"},
        cpr::Payload{{"ref_title",              "TEST_MapleLeaf"},
                     {"ref_country",            "Canada"},
                     {"year",               "2023"},
                     {"ref_metal",              "Gold"},
                     {"quantity",           "2"},
                     {"purchase_price",     "2100.50"},
                     {"purchase_currency",  "CHF"},
                     {"dealer",             "TEST_Shop"}},
        cpr::Redirect{false}
    );

    REQUIRE((post.status_code == 303 || post.status_code == 200));

    auto coins = cpr::Get(cpr::Url{base_url + "/coins"});
    REQUIRE(coins.status_code == 200);
    CHECK(coins.text.find("TEST_MapleLeaf") != std::string::npos);
    CHECK(coins.text.find("Canada")         != std::string::npos);
    CHECK(coins.text.find("2023")           != std::string::npos);
    CHECK(coins.text.find("TEST_Shop")      != std::string::npos);
    CHECK(coins.text.find("2100.50")        != std::string::npos);

    pqxx::connection conn{conn_str};
    test_fixtures::clean(conn);
}

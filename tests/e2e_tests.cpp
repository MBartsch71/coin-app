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
    CHECK(coins.text.find("1450") != std::string::npos); 
    CHECK(coins.text.find("0.9167") != std::string::npos); 
    CHECK(coins.text.find("33.931") != std::string::npos); 

    // Cleanup (the app is killed by RAII at scope end)
    pqxx::connection conn{conn_str};
    test_fixtures::clean(conn);
}

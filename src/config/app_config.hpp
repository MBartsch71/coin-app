#ifndef COINAPP_APP_CONFIG_HPP
#define COINAPP_APP_CONFIG_HPP
#include <string>

namespace config {
    
    struct AppConfigValues {
        std::string host        = "127.0.0.1";
        std::string port        = "5432";
        std::string dbname      = "coin_catalog_dev";
        std::string user        = "coinapp";
        std::string password    = "coinappdev";
        int web_port            = 9000;

        explicit operator std::string() const;
    };

    class EnvironmentLoader {
        public:
            static AppConfigValues load();
    };
}

#endif // COINAPP_APP_CONFIG_HPP
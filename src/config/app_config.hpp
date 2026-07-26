#ifndef COINAPP_DATABASE_CONFIG_HPP
#define COINAPP_DATABASE_CONFIG_HPP
#include <string>

namespace database {
    
    struct DatabaseConfigValues {
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
            static DatabaseConfigValues load();
    };
}

#endif // COINAPP_DATABASE_CONFIG_HPP
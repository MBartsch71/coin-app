#include "database/db_config.hpp"
#include <cstdlib>

namespace database {
    DatabaseConfigValues::operator std::string() const {
        return  "host=" + host 
              + " port=" + port 
              + " dbname=" + dbname 
              + " user=" + user 
              + " password=" + password;
    }

    DatabaseConfigValues EnvironmentLoader::load() {
        auto resolve = [](const char* name, const char* def) -> std::string{
            if (const char* val = std::getenv(name)) {
                return val;
            }
            return def;
        };

        DatabaseConfigValues config;
        config.host = resolve("COINAPP_DB_HOST", "127.0.0.1");
        config.port = resolve("COINAPP_DB_PORT", "5432");
        config.dbname = resolve("COINAPP_DB_NAME", "coin_catalog_dev");
        config.user = resolve("COINAPP_DB_USER", "coinapp");
        config.password = resolve("COINAPP_DB_PASSWORD", "coinappdev");
        return config;
    }

}
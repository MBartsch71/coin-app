#include "config/app_config.hpp"
#include <cstdlib>
#include <string>

namespace config {
    AppConfigValues::operator std::string() const {
        return  "host=" + host 
              + " port=" + port 
              + " dbname=" + dbname 
              + " user=" + user 
              + " password=" + password;
    }

    AppConfigValues EnvironmentLoader::load() {
        auto resolve = [](const char* name, const char* def) -> std::string{
            if (const char* val = std::getenv(name)) {
                return val;
            }
            return def;
        };

        auto resolve_int = [](const char* name, int def) -> int {
            if (const char* val = std::getenv(name)) {
                try {
                    return std::stoi(val);
                } catch (const std::exception) {
                    return def;
                }
            }
            return def;
        };

        AppConfigValues config;
        config.host     = resolve("COINAPP_DB_HOST", "127.0.0.1");
        config.port     = resolve("COINAPP_DB_PORT", "5432");
        config.dbname   = resolve("COINAPP_DB_NAME", "coin_catalog_dev");
        config.user     = resolve("COINAPP_DB_USER", "coinapp");
        config.password = resolve("COINAPP_DB_PASSWORD", "coinappdev");
        config.web_port = resolve_int("COINAPP_PORT", 9000);
        return config;
    }

}
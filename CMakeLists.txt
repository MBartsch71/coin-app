cmake_minimum_required(VERSION 3.25)

project(coin_app VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Crow REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

add_library(config_lib STATIC
    src/config/app_config.cpp
)

target_include_directories(config_lib PUBLIC src)
target_link_libraries(config_lib PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    PkgConfig::PQXX
)

add_library(coin_lib STATIC
    src/coins/coin_repository.cpp
)

target_include_directories(coin_lib PUBLIC src)
target_link_libraries(coin_lib PRIVATE
    PkgConfig::PQXX
)

add_executable(coin_app
    src/main.cpp
)

target_compile_definitions(coin_app PRIVATE
    COINAPP_TEMPLATE_DIR="${CMAKE_SOURCE_DIR}/templates"
)

target_link_libraries(coin_app PRIVATE 
    Crow::Crow
    config_lib
    coin_lib
)

target_compile_features(coin_app PRIVATE cxx_std_26)

target_compile_options(coin_app PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

enable_testing()
add_subdirectory(tests)
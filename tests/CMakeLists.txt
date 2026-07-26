find_package(Catch2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

add_executable(unit_tests
    test_db_config.cpp
    test_coin_repository.cpp
)

target_link_libraries(unit_tests PRIVATE
    Catch2::Catch2WithMain
    config_lib
    coin_lib
    PkgConfig::PQXX
)

# Fast unit tests - no database needed
# "~[integration]" runs everything NOT taggeg [integration],
# so future unit tests are picked up automatically
add_test(NAME unit COMMAND unit_tests "~[integration]")

# Integration tests - real database, but the DISPOSABLE (test) one.
# CTest injects the test environment itself; no manual sourcing
add_test(NAME integration 
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_with_env.sh
            ${CMAKE_SOURCE_DIR}/config/.env.test
            $<TARGET_FILE:unit_tests> "[integration]"
)
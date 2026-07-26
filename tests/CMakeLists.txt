find_package(Catch2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

# --- cpr: HTTP client for E2E tests (fetched at configre time) ---
include(FetchContent)
set(CPR_USE_SYSTEM_CURL ON CACHE BOOL "" FORCE)
set(CPR_BUILD_TEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git 
    GIT_TAG        1.11.1
)
FetchContent_MakeAvailable(cpr)

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

add_executable(e2e_tests
    e2e_tests.cpp
)

target_link_libraries(e2e_tests PRIVATE
    Catch2::Catch2WithMain
    cpr::cpr
    config_lib
    coin_lib
    PkgConfig::PQXX
)

target_compile_definitions(e2e_tests PRIVATE
    COINAPP_BINARY="$<TARGET_FILE:coin_app>"
    COINAPP_ENV_WRAPPER="${CMAKE_CURRENT_SOURCE_DIR}/run_with_env.sh"
    COINAPP_TEST_ENV="${CMAKE_SOURCE_DIR}/config/.env.test"
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

# End-to-end tests - spawns the real binary against the test env.
add_test(NAME e2e
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_with_env.sh
            ${CMAKE_SOURCE_DIR}/config/.env.test
            $<TARGET_FILE:e2e_tests>
)
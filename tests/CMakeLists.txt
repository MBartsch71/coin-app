find_package(Catch2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

add_executable(unit_tests
    test_db_config.cpp
    test_coin_repository.cpp
)

target_link_libraries(unit_tests PRIVATE
    Catch2::Catch2WithMain
    database_lib
    coin_lib
    PkgConfig::PQXX
)

add_test(NAME unit_tests COMMAND unit_tests)
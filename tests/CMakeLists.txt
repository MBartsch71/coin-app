find_package(Catch2 REQUIRED)

add_executable(unit_tests
    test_db_config.cpp
)

target_link_libraries(unit_tests PRIVATE
    Catch2::Catch2WithMain
    database_lib
)

add_test(NAME unit_tests COMMAND unit_tests)
add_executable(repl_tests
    tokenizer_test.cpp
    parser_test.cpp
    evaluator_test.cpp
    integration_test.cpp
)

repl_set_warnings(repl_tests)

target_link_libraries(repl_tests
    PRIVATE
        repl_core
        Catch2::Catch2WithMain
)

target_include_directories(repl_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME repl_tests COMMAND repl_tests)

add_library(repl_core
    token.cpp
    expression.cpp
    evaluator.cpp
    state.cpp
)

repl_set_warnings(repl_core)

target_include_directories(repl_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_library(linenoise STATIC
    ${PROJECT_SOURCE_DIR}/third_party/linenoise/linenoise.c
)

target_include_directories(linenoise
    PUBLIC
        ${PROJECT_SOURCE_DIR}/third_party/linenoise
)

repl_set_warnings(linenoise)

if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(linenoise PRIVATE -Wno-variadic-macro-arguments-omitted)
endif()

add_executable(repl
    main.cpp
)

repl_set_warnings(repl)

target_link_libraries(repl
    PRIVATE
        repl_core
        linenoise
)

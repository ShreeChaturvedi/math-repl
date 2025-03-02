cmake_minimum_required(VERSION 3.20)

project(repl-calculator VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(REPL_BUILD_TESTS "Build tests" ON)

include(cmake/CompilerWarnings.cmake)

add_subdirectory(src)

if (REPL_BUILD_TESTS)
    include(cmake/Dependencies.cmake)
    enable_testing()
    add_subdirectory(tests)
endif()

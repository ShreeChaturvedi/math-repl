cmake_minimum_required(VERSION 3.20)

project(repl-calculator VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(REPL_BUILD_TESTS "Build tests" ON)

include(GNUInstallDirs)
include(cmake/CompilerWarnings.cmake)

add_subdirectory(src)

if (REPL_BUILD_TESTS)
    include(cmake/Dependencies.cmake)
    enable_testing()
    add_subdirectory(tests)
endif()

install(FILES
    "${PROJECT_SOURCE_DIR}/LICENSE"
    "${PROJECT_SOURCE_DIR}/README.md"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

set(CPACK_PACKAGE_NAME "math-repl")
set(CPACK_PACKAGE_VENDOR "Shree Chaturvedi")
set(CPACK_PACKAGE_CONTACT "https://github.com/ShreeChaturvedi")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Math REPL in C++23")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/ShreeChaturvedi/math-repl")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_FILE_NAME
    "math-repl-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}"
)
if (WIN32)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()
include(CPack)

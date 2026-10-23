cmake_minimum_required(VERSION 3.20)
project(lab1 VERSION 1.0.0 LANGUAGES CXX)

option(LAB1_BUILD_TESTS "Build unit tests" ON)
option(LAB1_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(LAB1_COVERAGE "Enable coverage instrumentation" OFF)
option(LAB1_STATIC "Statically link lab1_cli on Linux" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(FETCHCONTENT_BASE_DIR "${PROJECT_SOURCE_DIR}/.deps" CACHE PATH "FetchContent dependencies directory")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
if(LAB1_STATIC AND LAB1_SANITIZE)
    message(FATAL_ERROR "LAB1_STATIC and LAB1_SANITIZE cannot be enabled together")
endif()
if(LAB1_STATIC AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "LAB1_STATIC is supported only on Linux")
endif()

add_library(lab1_options INTERFACE)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(lab1_options INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
endif()
if(LAB1_SANITIZE)
    target_compile_options(lab1_options INTERFACE -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
    target_link_options(lab1_options INTERFACE -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
endif()
if(LAB1_COVERAGE)
    target_compile_options(lab1_options INTERFACE --coverage -O0 -g)
    target_link_options(lab1_options INTERFACE --coverage)
endif()

add_library(tic_tac_toe STATIC src/tic_tac_toe/tic_tac_toe.cpp)
target_include_directories(tic_tac_toe PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(tic_tac_toe PRIVATE lab1_options)

add_library(polynomial STATIC src/polynomial/polynomial.cpp)
target_include_directories(polynomial PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(polynomial PRIVATE lab1_options)

add_library(lab1_cli_lib STATIC cli/cli.cpp)
target_include_directories(lab1_cli_lib PUBLIC "${PROJECT_SOURCE_DIR}/cli" "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(lab1_cli_lib PUBLIC tic_tac_toe polynomial PRIVATE lab1_options)

add_executable(lab1_cli cli/main.cpp)
target_link_libraries(lab1_cli PRIVATE lab1_cli_lib lab1_options)
if(LAB1_STATIC)
    target_link_options(lab1_cli PRIVATE -static -static-libgcc -static-libstdc++)
endif()

if(LAB1_BUILD_TESTS)
    enable_testing()
    include(FetchContent)
    include(GoogleTest)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(tic_tac_toe_tests tests/test_tic_tac_toe.cpp)
    target_link_libraries(tic_tac_toe_tests PRIVATE tic_tac_toe GTest::gtest_main lab1_options)
    gtest_discover_tests(tic_tac_toe_tests DISCOVERY_MODE PRE_TEST)

    add_executable(polynomial_tests tests/test_polynomial.cpp)
    target_link_libraries(polynomial_tests PRIVATE polynomial GTest::gtest_main lab1_options)
    gtest_discover_tests(polynomial_tests DISCOVERY_MODE PRE_TEST)

    add_executable(cli_tests tests/test_cli.cpp)
    target_link_libraries(cli_tests PRIVATE lab1_cli_lib GTest::gtest_main lab1_options)
    gtest_discover_tests(cli_tests DISCOVERY_MODE PRE_TEST)
endif()

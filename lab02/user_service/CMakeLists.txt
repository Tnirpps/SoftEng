cmake_minimum_required(VERSION 3.23)
project(MaDisk CXX)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#include(DownloadUserver)


find_package(userver COMPONENTS core chaotic s3api)


userver_setup_environment()

set(PROJECT_INC_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/src
)


# ============================= Common sources ============================= 
include_directories(src)

file(GLOB_RECURSE SOURCES src/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX "src/main.cpp")

add_library(
    ${PROJECT_NAME}_objs OBJECT
    ${SOURCES}
)
target_link_libraries(
    ${PROJECT_NAME}_objs
    PUBLIC userver::core userver::s3api
)

# find_package(jwt-cpp REQUIRED)
# target_link_libraries(${PROJECT_NAME}_objs PUBLIC jwt-cpp::jwt-cpp)

# code gen
file(GLOB_RECURSE SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.yaml)
userver_target_generate_chaotic(
    ${PROJECT_NAME}-chgen
    # Generate serializers for responses
    GENERATE_SERIALIZERS
    # Map '/components/schemas/*' JSONSchema types to C++ types in 'Gen::STEM' namespace
    LAYOUT "/components/schemas/([^/]*)/=Gen::{stem}::{0}"
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    SCHEMAS ${SCHEMAS}
    RELATIVE_TO ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC ${PROJECT_NAME}-chgen)
# =========================== Common sources end ===========================


# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

# Unit Tests
file(GLOB_RECURSE TEST_SOURCES tests/unit/*.cpp)
add_executable(${PROJECT_NAME}_unittest ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver::utest)
add_google_tests(${PROJECT_NAME}_unittest)

# Benchmarks
#add_executable(${PROJECT_NAME}_benchmark src/greeting_benchmark.cpp)
#target_link_libraries(${PROJECT_NAME}_benchmark PRIVATE ${PROJECT_NAME}_objs userver::ubench)
#add_google_benchmark_tests(${PROJECT_NAME}_benchmark)

# Functional testing
userver_testsuite_add_simple()

# Install
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT ${PROJECT_NAME}
)
install(
    FILES ${CONFIGS_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)

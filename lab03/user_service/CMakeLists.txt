project(user_service LANGUAGES CXX)

set(PROJECT_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)
include_directories(src)

file(GLOB_RECURSE SOURCES src/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX "src/main.cpp")

add_library(user_service_objs OBJECT ${SOURCES})
target_link_libraries(user_service_objs
    PUBLIC
        userver::core
        userver::s3api
        common
)

# codegen
file(GLOB_RECURSE SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.yaml)
if(SCHEMAS)
    userver_target_generate_chaotic(
        ${PROJECT_NAME}-chgen
        GENERATE_SERIALIZERS
        LAYOUT "/components/schemas/([^/]*)/=Gen::{stem}::{0}"
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
        SCHEMAS ${SCHEMAS}
        RELATIVE_TO ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(user_service_objs PUBLIC ${PROJECT_NAME}-chgen)
endif()

# main binary
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE user_service_objs)

# unit tests
file(GLOB_RECURSE TEST_SOURCES tests/unit/*.cpp)
if(TEST_SOURCES)
    add_executable(${PROJECT_NAME}_unittest ${TEST_SOURCES})
    target_link_libraries(${PROJECT_NAME}_unittest PRIVATE user_service_objs userver::utest)
    add_google_tests(${PROJECT_NAME}_unittest)
    userver_testsuite_add_simple()
endif()

# install configs
include(GNUInstallDirs)
file(GLOB CONFIGS_FILES configs/*.yaml configs/*.json)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME})

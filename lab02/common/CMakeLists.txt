file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(common STATIC ${COMMON_SOURCES})

target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Зависимости — всё, что общее
target_link_libraries(common
    PUBLIC
        userver::core
        userver::s3api
        nlohmann_json::nlohmann_json
        jwt-cpp::jwt-cpp
)

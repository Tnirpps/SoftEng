cmake_minimum_required(VERSION 3.23)
project(MaDisk LANGUAGES CXX)

# Установки, которые нужны всем
find_package(userver REQUIRED COMPONENTS core chaotic s3api)
find_package(nlohmann_json REQUIRED)
find_package(jwt-cpp REQUIRED)

# Подключаем userver окружение (он требует userver_setup_environment)
userver_setup_environment()

# Общая библиотека
add_subdirectory(common)

# Отдельные сервисы
add_subdirectory(user_service)
add_subdirectory(directory_service)

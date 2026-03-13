FROM ghcr.io/userver-framework/ubuntu-22.04-userver:latest


# RUN pip install conan

WORKDIR /app

# Копируем только необходимые файлы и директории
COPY src/ src/
COPY configs/ configs/
COPY schemas/ schemas/
COPY tests/ tests/
# COPY cmake/ cmake/
COPY CMakeLists.txt .
# COPY CMakePresets.json .
# COPY CMakeUserPresets.json .
# COPY conanfile.txt .

# Создаем conanfile без userver для Docker сборки
# RUN sed -i '/userver\/2.16-rc/d' conanfile.txt

# 3.1 Создаём профиль для сборки зависимостей (C++20)
# RUN conan profile detect --name=deps && \
#     sed -i 's/compiler.cppstd=.*/compiler.cppstd=gnu20/' $(conan profile path deps) && \
#     cat $(conan profile path deps)

# 3.2 Создаем профиль app с C++23
# RUN conan profile detect --name=app && \
#     sed -i 's/compiler.cppstd=.*/compiler.cppstd=gnu23/' $(conan profile path app) && \
#     cat $(conan profile path app)

# Генерируем сборку без userver dependency
# RUN conan profile detect
# RUN conan install . --build=missing -pr:b=deps -pr:h=app
# RUN cmake --preset conan-release
# RUN cmake --build --preset conan-release

# 2. Компилируем проект (используем nproc для параллельной сборки)
RUN mkdir build_debug && \
    cd build_debug && \
    cmake -DCMAKE_BUILD_TYPE=Debug .. && \
    make -j$(nproc)

# CMD ["./build/Release/MaDisk", "-c", "./configs/static_config.yaml", "--config_vars", "./configs/config_vars.testing.yaml"]
CMD ["./build_debug/MaDisk", "-c", "./configs/static_config.yaml", "--config_vars", "./configs/config_vars.testing.yaml"]
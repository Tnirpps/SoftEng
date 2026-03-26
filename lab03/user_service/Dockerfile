# образ в котором уже есть userver
FROM ghcr.io/userver-framework/ubuntu-22.04-userver:latest
 
 
RUN pip install conan
# 3.1 Создаём профиль для сборки зависимостей (C++20)
RUN conan profile detect --name=deps && \
     sed -i 's/compiler.cppstd=.*/compiler.cppstd=gnu20/' $(conan profile path deps) && \
     cat $(conan profile path deps)
 
# 3.2 Создаем профиль app с C++23
RUN conan profile detect --name=app && \
     sed -i 's/compiler.cppstd=.*/compiler.cppstd=gnu23/' $(conan profile path app) && \
     cat $(conan profile path app)
 
WORKDIR /app
 
COPY conanfile.txt .
# Создаем conanfile без userver для Docker сборки
RUN sed -i '/userver\/2.16-rc/d' conanfile.txt
 
 
# Копируем только необходимые файлы и директории
COPY src/ src/
COPY configs/ configs/
COPY schemas/ schemas/
COPY tests/ tests/
COPY CMakeLists.txt .
 
 
# Генерируем сборку без userver dependency
RUN conan install . --build=missing -pr:b=deps -pr:h=app
RUN cmake --preset conan-release
RUN cmake --build --preset conan-release
 
CMD ["./build/Release/MaDisk", "-c", "./configs/static_config.yaml", "--config_vars", "./configs/config_vars.testing.yaml"]
# CMD ["./build_debug/MaDisk", "-c", "./configs/static_config.yaml", "--config_vars", "./configs/config_vars.testing.yaml"]
 
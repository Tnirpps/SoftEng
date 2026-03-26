# Multi-stage build for MaDisk monorepo
FROM ghcr.io/userver-framework/ubuntu-22.04-userver:latest AS builder

# Install Conan
RUN pip install conan

# Setup Conan profiles
RUN conan profile detect --name=deps && \
    sed -i 's/compiler.cppstd=.*/compiler.cppstd=gnu20/' $(conan profile path deps) && \
    conan profile detect --name=app && \
    sed -i 's/compiler.cppstd=.*/compiler.cppstd=gnu23/' $(conan profile path app)

WORKDIR /app

# Copy conanfile and remove userver dependency for Docker build
COPY conanfile.txt .
RUN sed -i '/userver\/2.16-rc/d' conanfile.txt

# Copy all source code
COPY common/ common/
COPY user_service/ user_service/
COPY directory_service/ directory_service/
COPY file_service/ file_service/
COPY CMakeLists.txt .

# Install dependencies and build
RUN conan install . --build=missing -pr:b=deps -pr:h=app
RUN cmake --preset conan-release
RUN cmake --build --preset conan-release

# Runtime stage
FROM ghcr.io/userver-framework/ubuntu-22.04-userver:latest AS runtime

WORKDIR /app

# Copy built binaries from builder (binaries are in build/Release/)
COPY --from=builder /app/build/Release/user_service /app/user_service/bin
COPY --from=builder /app/build/Release/directory_service /app/directory_service/bin
COPY --from=builder /app/build/Release/file_service /app/file_service/bin

# Copy configs for all services
COPY --from=builder /app/user_service/configs/ /app/user_service/configs/
COPY --from=builder /app/directory_service/configs/ /app/directory_service/configs/
COPY --from=builder /app/file_service/configs/ /app/file_service/configs/

# Copy schemas for codegen reference (if needed at runtime)
COPY --from=builder /app/user_service/schemas/ /app/user_service/schemas/
COPY --from=builder /app/directory_service/schemas/ /app/directory_service/schemas/
COPY --from=builder /app/file_service/schemas/ /app/file_service/schemas/

# Install curl for healthcheck
RUN apt-get update && apt-get install -y curl && rm -rf /var/lib/apt/lists/*

# Default command - can be overridden by docker-compose
CMD ["./user_service/bin/user_service", "-c", "./user_service/configs/static_config.yaml", "--config_vars", "./user_service/configs/config_vars.testing.yaml"]

# dev: toolchain only, no source. CI and local runs mount the checkout at /workspace,
# so this stage is rebuilt only when this file changes
FROM ubuntu:24.04 AS dev
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        clang \
        libclang-rt-dev \
        cmake \
        ninja-build \
        clang-format \
        clang-tidy \
        ca-certificates \
        nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /workspace/Logger

# build: compile the project inside the image, which also checks the Dockerfile still works
FROM dev AS build
COPY . /workspace
RUN cmake --workflow --preset release

# runtime: just the binary and its config
FROM ubuntu:24.04 AS runtime
WORKDIR /app
COPY --from=build /workspace/Logger/build/Linux/release/logimp /usr/local/bin/logimp
COPY --from=build /workspace/Logger/logger_config.json .
CMD ["logimp"]

# dev: toolchain only, no source. CI and local runs mount the checkout at /workspace,
# so this stage is rebuilt only when this file changes
FROM ubuntu:24.04 AS dev
ARG GTEST_VERSION=v1.14.0
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        clang \
        libclang-rt-dev \
        git \
        ca-certificates \
        nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*
RUN git clone --depth 1 --branch ${GTEST_VERSION} \
        https://github.com/google/googletest.git /opt/googletest
ENV GTEST_DIR=/opt/googletest/googletest
# Keep Linux objects apart from host ones when the checkout is mounted from macOS
ENV OBJ_DIR=./obj/linux
WORKDIR /workspace/Logger

# build: compile the project inside the image, which also checks the Dockerfile still works
FROM dev AS build
COPY . /workspace
RUN make

# runtime: just the binary and its config
FROM ubuntu:24.04 AS runtime
WORKDIR /app
COPY --from=build /workspace/Logger/obj/linux/logimp /usr/local/bin/logimp
COPY --from=build /workspace/Logger/logger_config.json .
CMD ["logimp"]

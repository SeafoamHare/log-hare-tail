FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    build-essential \
    nlohmann-json3-dev

WORKDIR /workspace
COPY . /workspace

RUN cd /workspace/Logger && \
    make

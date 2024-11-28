# 使用官方的 Ubuntu 基礎映像
FROM ubuntu:20.04

# 設定時區為 UTC，避免安裝過程中的時區選擇
ENV TZ=UTC

# 更新系統並安裝必要的包
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    unzip \
    lsb-release \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# 下載並安裝 Google Test
RUN git clone --branch release-1.11.0 https://github.com/google/googletest.git /googletest \
    && cd /googletest \
    && cmake . \
    && make \
    && make install

# 設置工作目錄
WORKDIR /workspace

# 複製當前目錄下的所有檔案到容器中的 /workspace
COPY . /workspace

# 預設的入口命令
CMD ["bash"]

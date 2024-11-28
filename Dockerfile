# 使用官方的 Ubuntu 基礎映像
FROM ubuntu:20.04

# 設定時區為 UTC，避免安裝過程中的時區選擇
ENV TZ=UTC

# 更新系統並安裝必要的包
RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    curl \
    apt install libgtest-dev -y \
    cd /usr/src/gtest \
    cmake CMakeLists.txt \
    make \

# 設置工作目錄
WORKDIR /workspace

# 複製當前目錄下的所有檔案到容器中的 /workspace
COPY . /workspace

# 預設的入口命令
CMD ["bash"]

# 設定變數
CXX = g++
CXXFLAGS = -pthread
INCLUDE = -I./include 
SRC_DIR = ./src
OUT = logimp
OBJ_DIR = ./obj

SRC = $(wildcard $(SRC_DIR)/core/*.cxx) $(wildcard $(SRC_DIR)/formatMessage/*.cxx) 
OBJ = $(SRC:$(SRC_DIR)/%.cxx=$(OBJ_DIR)/%.o) $(OBJ_DIR)/main.o

# 預設目標
all: $(OBJ_DIR)/$(OUT)

# 編譯並鏈接
$(OBJ_DIR)/$(OUT): $(OBJ)
	$(CXX) $(OBJ) $(CXXFLAGS) -o $(OBJ_DIR)/$(OUT)

# # 編譯 source 檔案為 object 檔案
$(OBJ_DIR)/main.o: main.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) -c main.cpp $(INCLUDE) $(CXXFLAGS) -o $(OBJ_DIR)/main.o

# 編譯 source 檔案為 object 檔案，並自動創建目錄
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cxx | $(OBJ_DIR)
	$(CXX) -c $< $(INCLUDE) $(CXXFLAGS) -o $@

# 清除編譯結果
clean:
	rm -rf $(OBJ_DIR)/*.o $(OBJ_DIR)/core/*.o $(OBJ_DIR)/formatMessage/*.o $(OBJ_DIR)/$(OUT) ./log

# 執行程式
run:
	$(OBJ_DIR)/$(OUT)

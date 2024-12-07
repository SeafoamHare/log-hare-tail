# 設定變數
CXX = g++
CXXFLAGS = -pthread
INCLUDE = -I./include
SRC_DIR = ./src
OUT = logimp
OBJ_DIR = ./obj
OBJ = $(OBJ_DIR)/main.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/config_file.o $(OBJ_DIR)/podInfo_Retriever.o $(OBJ_DIR)/Level.o

# 預設目標
all: $(OBJ_DIR)/$(OUT)

# 編譯並鏈接
$(OBJ_DIR)/$(OUT): $(OBJ)
	$(CXX) $(OBJ) $(CXXFLAGS) -o $(OBJ_DIR)/$(OUT)

# 編譯 source 檔案為 object 檔案
$(OBJ_DIR)/main.o: main.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) -c main.cpp $(INCLUDE) $(CXXFLAGS) -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cxx
	@mkdir -p $(OBJ_DIR)  
	$(CXX) -c $< $(INCLUDE) $(CXXFLAGS) -o $@

# 清除編譯結果
clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/$(OUT) -r ./log

run:
	$(OBJ_DIR)/$(OUT)

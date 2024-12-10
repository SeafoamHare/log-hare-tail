# 設定變數
CXX = g++
CXXFLAGS = -pthread
INCLUDE = -I./include
SRC_DIR = ./src
OUT = logimp
OBJ_DIR = ./obj

# Update OBJ to include all object files from src/core and src/formatMessage
OBJ = $(OBJ_DIR)/main.o \
      $(OBJ_DIR)/core/logger.o \
      $(OBJ_DIR)/core/config_file.o \
      $(OBJ_DIR)/core/podInfo_Retriever.o \
      $(OBJ_DIR)/core/LogLevel.o \
      $(OBJ_DIR)/formatMessage/JsonMessageFormatter.o \
      $(OBJ_DIR)/formatMessage/PlainMessageFormatter.o

# 預設目標
all: $(OBJ_DIR)/$(OUT)

# 編譯並鏈接
$(OBJ_DIR)/$(OUT): $(OBJ)
	$(CXX) $(OBJ) $(CXXFLAGS) -o $(OBJ_DIR)/$(OUT)

# 編譯 source 檔案為 object 檔案
$(OBJ_DIR)/main.o: main.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) -c main.cpp $(INCLUDE) $(CXXFLAGS) -o $(OBJ_DIR)/main.o

# Add rules to compile the source files in src/core and src/formatMessage
$(OBJ_DIR)/core/%.o: $(SRC_DIR)/core/%.cxx
	@mkdir -p $(OBJ_DIR)/core
	$(CXX) -c $< $(INCLUDE) $(CXXFLAGS) -o $@

$(OBJ_DIR)/formatMessage/%.o: $(SRC_DIR)/formatMessage/%.cxx
	@mkdir -p $(OBJ_DIR)/formatMessage
	$(CXX) -c $< $(INCLUDE) $(CXXFLAGS) -o $@

# 清除編譯結果
clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/core/*.o $(OBJ_DIR)/formatMessage/*.o $(OBJ_DIR)/$(OUT) -r ./log

# 執行程式
run:
	$(OBJ_DIR)/$(OUT)

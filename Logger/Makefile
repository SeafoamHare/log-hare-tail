# 設定變數
CXX = g++
CXXFLAGS = -pthread
INCLUDE = -I./include
SRC = main.cpp ./src/logger.cxx ./src/config_file.cxx
OUT = logimp
OBJDIR = obj
OBJ = $(OBJDIR)/main.o $(OBJDIR)/logger.o $(OBJDIR)/config_file.o

# 預設目標
all: $(OBJDIR)/$(OUT)

# 編譯並鏈接
$(OBJDIR)/$(OUT): $(OBJ)
	$(CXX) $(OBJ) $(CXXFLAGS) -o $(OBJDIR)/$(OUT)

# 編譯 source 檔案為 object 檔案
$(OBJDIR)/main.o: main.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) -c main.cpp $(INCLUDE) $(CXXFLAGS) -o $(OBJDIR)/main.o

$(OBJDIR)/logger.o: ./src/logger.cxx
	@mkdir -p $(OBJDIR)
	$(CXX) -c ./src/logger.cxx $(INCLUDE) $(CXXFLAGS) -o $(OBJDIR)/logger.o

$(OBJDIR)/config_file.o: ./src/config_file.cxx
	@mkdir -p $(OBJDIR)
	$(CXX) -c ./src/config_file.cxx $(INCLUDE) $(CXXFLAGS) -o $(OBJDIR)/config_file.o

# 清除編譯結果
clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/$(OUT)

run:
	$(OBJDIR)/$(OUT)

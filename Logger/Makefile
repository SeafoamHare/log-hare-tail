CXX = g++
CXXFLAGS = -pthread
INCLUDE = -I./include 
SRC_DIR = ./src
OUT = logimp
OBJ_DIR = ./obj

SRC = $(wildcard $(SRC_DIR)/core/*.cxx) $(wildcard $(SRC_DIR)/formatMessage/*.cxx) 
OBJ = $(SRC:$(SRC_DIR)/%.cxx=$(OBJ_DIR)/%.o) $(OBJ_DIR)/main.o

all: $(OBJ_DIR)/$(OUT)

$(OBJ_DIR)/$(OUT): $(OBJ)
	$(CXX) $(OBJ) $(CXXFLAGS) -o $(OBJ_DIR)/$(OUT)

$(OBJ_DIR)/main.o: main.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) -c main.cpp $(INCLUDE) $(CXXFLAGS) -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cxx
	@mkdir -p $(dir $@)
	$(CXX) -c $< $(INCLUDE) $(CXXFLAGS) -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o $(OBJ_DIR)/core/*.o $(OBJ_DIR)/formatMessage/*.o $(OBJ_DIR)/$(OUT) ./log

run:
	$(OBJ_DIR)/$(OUT)

CXX = g++
CXXFLAGS = -pthread
INCLUDE = -I./include
SRC_DIR = ./src
OUT = logimp
# ?= lets the dev container point this elsewhere, so Linux objects never mix with host ones
OBJ_DIR ?= ./obj

# Warnings are always on; pass WERROR=1 (as CI does) to make them fail the build
WARN_FLAGS = -Wall -Wextra
ifneq ($(WERROR),)
WARN_FLAGS += -Werror
endif

# macOS: HOST_NAME_MAX is Linux-only, and Homebrew headers (nlohmann/json) are not on the default include path
ifeq ($(shell uname -s),Darwin)
CXXFLAGS += -DHOST_NAME_MAX=255
BREW_PREFIX := $(shell brew --prefix 2>/dev/null)
ifneq ($(BREW_PREFIX),)
INCLUDE += -I$(BREW_PREFIX)/include
endif
endif

SRC = $(wildcard $(SRC_DIR)/core/*.cxx) $(wildcard $(SRC_DIR)/formatMessage/*.cxx)
OBJ = $(SRC:$(SRC_DIR)/%.cxx=$(OBJ_DIR)/%.o) $(OBJ_DIR)/main.o

# Write a .d file next to each object listing the headers it includes, so editing a
# header rebuilds every object that uses it (included at the bottom of this file)
DEPFLAGS = -MMD -MP

# Tests: each tests/*.cpp builds into its own executable, because Logger is a
# singleton with thread_local state that must not leak between test files
GTEST_DIR ?= ./third_party/googletest/googletest
TEST_DIR = ./tests
TEST_OBJ_DIR = $(OBJ_DIR)/test
TEST_CXXFLAGS = $(CXXFLAGS) $(SAN_FLAGS) -std=c++17 $(DEPFLAGS)
TEST_INCLUDE = $(INCLUDE) -I$(GTEST_DIR)/include -I$(GTEST_DIR)
TEST_SRC = $(wildcard $(TEST_DIR)/*.cpp)
TEST_BIN = $(TEST_SRC:$(TEST_DIR)/%.cpp=$(TEST_OBJ_DIR)/bin/%)
TEST_CASE_OBJ = $(TEST_SRC:$(TEST_DIR)/%.cpp=$(TEST_OBJ_DIR)/cases/%.o)
TEST_LIB_OBJ = $(SRC:$(SRC_DIR)/%.cxx=$(TEST_OBJ_DIR)/src/%.o)
GTEST_OBJ = $(TEST_OBJ_DIR)/gtest/gtest-all.o $(TEST_OBJ_DIR)/gtest/gtest_main.o

.PHONY: all clean run test test-tsan test-asan check-gtest
# Keep test objects that only appear in pattern rules, otherwise make deletes them as intermediates
.SECONDARY: $(TEST_CASE_OBJ) $(TEST_LIB_OBJ) $(GTEST_OBJ)

all: $(OBJ_DIR)/$(OUT)

$(OBJ_DIR)/$(OUT): $(OBJ)
	$(CXX) $(OBJ) $(CXXFLAGS) -o $(OBJ_DIR)/$(OUT)

$(OBJ_DIR)/main.o: main.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) -c main.cpp $(INCLUDE) $(CXXFLAGS) $(WARN_FLAGS) $(DEPFLAGS) -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cxx
	@mkdir -p $(dir $@)
	$(CXX) -c $< $(INCLUDE) $(CXXFLAGS) $(WARN_FLAGS) $(DEPFLAGS) -o $@

test: $(TEST_BIN)
	@for t in $(TEST_BIN); do echo "=== $$t ==="; $$t $(GTEST_ARGS) || exit 1; done

# Same tests built with ThreadSanitizer, in a separate object dir so normal builds are not mixed in
test-tsan:
	$(MAKE) test TEST_OBJ_DIR=$(OBJ_DIR)/test-tsan SAN_FLAGS="-fsanitize=thread -g -O1"

# AddressSanitizer + UndefinedBehaviorSanitizer; UBSan only prints by default, so make it abort to fail the run
test-asan:
	$(MAKE) test TEST_OBJ_DIR=$(OBJ_DIR)/test-asan SAN_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -g -O1"

$(TEST_OBJ_DIR)/bin/%: $(TEST_OBJ_DIR)/cases/%.o $(TEST_LIB_OBJ) $(GTEST_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $^ $(CXXFLAGS) $(SAN_FLAGS) -o $@

$(TEST_OBJ_DIR)/cases/%.o: $(TEST_DIR)/%.cpp | check-gtest
	@mkdir -p $(dir $@)
	$(CXX) -c $< $(TEST_INCLUDE) $(TEST_CXXFLAGS) $(WARN_FLAGS) -o $@

$(TEST_OBJ_DIR)/src/%.o: $(SRC_DIR)/%.cxx
	@mkdir -p $(dir $@)
	$(CXX) -c $< $(TEST_INCLUDE) $(TEST_CXXFLAGS) $(WARN_FLAGS) -o $@

$(TEST_OBJ_DIR)/gtest/%.o: $(GTEST_DIR)/src/%.cc | check-gtest
	@mkdir -p $(dir $@)
	$(CXX) -c $< $(TEST_INCLUDE) $(TEST_CXXFLAGS) -o $@

check-gtest:
	@test -f $(GTEST_DIR)/src/gtest-all.cc || { \
		echo "googletest not found in $(GTEST_DIR)."; \
		echo "Run: git clone --depth 1 --branch v1.14.0 https://github.com/google/googletest.git third_party/googletest"; \
		exit 1; }

-include $(OBJ:.o=.d) $(TEST_CASE_OBJ:.o=.d) $(TEST_LIB_OBJ:.o=.d)

clean:
	rm -rf $(OBJ_DIR)/*.[od] $(OBJ_DIR)/core/*.[od] $(OBJ_DIR)/formatMessage/*.[od] $(OBJ_DIR)/$(OUT) ./log $(OBJ_DIR)/test $(OBJ_DIR)/test-tsan $(OBJ_DIR)/test-asan

run:
	$(OBJ_DIR)/$(OUT)

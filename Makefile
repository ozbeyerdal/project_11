CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude

BIN_DIR := bin
OBJ_DIR := build
SRC_DIR := src
TEST_DIR := tests

APP := $(BIN_DIR)/microgrid
TEST_APP := $(BIN_DIR)/tests

APP_SRCS :=   $(SRC_DIR)/main.cpp   $(SRC_DIR)/core/JsonLite.cpp   $(SRC_DIR)/core/Microgrid.cpp   $(SRC_DIR)/core/Controller.cpp   $(SRC_DIR)/frontend/Tui.cpp

TEST_SRCS := \
	$(TEST_DIR)/main_tests.cpp \
	$(TEST_DIR)/unit/test_linked_list.cpp \
	$(TEST_DIR)/unit/test_linked_queue.cpp \
	$(TEST_DIR)/unit/test_jsonlite.cpp \
	$(TEST_DIR)/integration/test_integration.cpp \
	$(TEST_DIR)/edge_cases/test_no_renewables.cpp \
	$(SRC_DIR)/core/JsonLite.cpp \
	$(SRC_DIR)/core/Microgrid.cpp \
	$(SRC_DIR)/core/Controller.cpp \
	$(SRC_DIR)/frontend/Tui.cpp

APP_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(APP_SRCS))
TEST_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(TEST_SRCS))

all: $(APP)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(APP): $(BIN_DIR) $(APP_OBJS)
	$(CXX) $(CXXFLAGS) $(APP_OBJS) -o $(APP)

$(TEST_APP): $(BIN_DIR) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) -o $(TEST_APP)

test: $(TEST_APP)
	./$(TEST_APP)

run: $(APP)
	./$(APP) data/input_sample.json --tui

# No external libs are vendored in the submission. This target is kept to match
# the project rubric ("make deps").
deps:
	@echo "No external dependencies to install."

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all test run clean deps

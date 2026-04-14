CC      := clang
CFLAGS  := -Iinclude -Wall -Wextra -g
SRC_DIR := src
TEST_DIR := test
OBJ_DIR := obj
BIN     := verdigris
TEST_BIN := test_lexer

# Source files
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Test files
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(TEST_SRCS:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)

# Filter out main.c for test builds (test has its own main)
LIB_OBJS := $(filter-out $(OBJ_DIR)/main.o, $(OBJS))

.PHONY: all clean rebuild test

all: $(BIN)

# Main executable
$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@

# Test executable
$(TEST_BIN): $(LIB_OBJS) $(TEST_OBJS)
	$(CC) $^ -o $@

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile test files
$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Run tests
test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(OBJ_DIR) $(BIN) $(TEST_BIN) $(BIN).dSYM $(TEST_BIN).dSYM

rebuild: clean all

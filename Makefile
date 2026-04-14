CC       := clang
CFLAGS   := -Iinclude -g -fsanitize=address,undefined -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes
LDFLAGS  := -fsanitize=address,undefined

SRC_DIR  := src
TEST_DIR := test
OBJ_DIR  := obj

BIN         := verdigris
TEST_LEXER  := test_lexer
TEST_DIAG   := test_diagnostic

# ------------------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------------------
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Library objects (everything except main.o)
LIB_OBJS := $(filter-out $(OBJ_DIR)/main.o, $(OBJS))

# ------------------------------------------------------------------------------
# Test files
# ------------------------------------------------------------------------------
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(TEST_SRCS:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)

# ------------------------------------------------------------------------------
# Targets
# ------------------------------------------------------------------------------
.PHONY: all clean rebuild test test-lexer test-diag

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

$(TEST_LEXER): $(LIB_OBJS) $(OBJ_DIR)/test_lexer.o
	$(CC) $^ $(LDFLAGS) -o $@

$(TEST_DIAG): $(LIB_OBJS) $(OBJ_DIR)/test_diagnostic.o $(OBJ_DIR)/diagnostic.o
	$(CC) $^ $(LDFLAGS) -o $@

# ------------------------------------------------------------------------------
# Compilation rules
# ------------------------------------------------------------------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ------------------------------------------------------------------------------
# Testing
# ------------------------------------------------------------------------------
test: test-lexer test-diag

test-lexer: $(TEST_LEXER)
	./$(TEST_LEXER)

test-diag: $(TEST_DIAG)
	./$(TEST_DIAG)

# ------------------------------------------------------------------------------
# Cleanup
# ------------------------------------------------------------------------------
clean:
	rm -rf $(OBJ_DIR)
	rm -f $(BIN) $(TEST_LEXER) $(TEST_DIAG)
	rm -rf *.dSYM

rebuild: clean all

CC = clang
CFLAGS = -Wall -Wextra -Werror -std=c11

inputfile := example/test.txt

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
APP_OBJ = $(OBJ_DIR)/main.o
TEST_SRC_DIR = test/src
TEST_OBJ_DIR = test/obj
TEST_BIN_DIR = test/bin
TEST_SRC_FILES = $(wildcard $(TEST_SRC_DIR)/test_*.c)
TEST_OBJ_FILES = $(patsubst $(TEST_SRC_DIR)/%.c,$(TEST_OBJ_DIR)/%.o,$(TEST_SRC_FILES)) \
								 $(filter-out $(APP_OBJ),$(OBJ_FILES))

TARGET = $(BIN_DIR)/ex01
TEST_TARGET = $(TEST_BIN_DIR)/test_ex01

INC_DIR = ./include

all: $(TARGET) $(TEST_TARGET)

$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	@$(CC) -o $@ $^

$(TEST_TARGET): $(TEST_OBJ_FILES)
	@mkdir -p $(TEST_BIN_DIR)
	@$(CC) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@$(CC) -c -I$(INC_DIR) $(CFLAGS) -o $@ $<

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c
	@mkdir -p $(TEST_OBJ_DIR)
	@$(CC) -c -I$(INC_DIR) $(CFLAGS) -o $@ $<

test: $(TEST_TARGET)
	@./$<

run: $(TARGET)
	@./$< $(inputfile)

repl: $(TARGET)
	@./$<

clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR) $(TEST_OBJ_DIR) $(TEST_BIN_DIR)

.PHONY: all run clean test


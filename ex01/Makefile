CC = clang
# CFLAGS = -Wall -Wextra -Werror -std=c11
CFLAGS =  -std=c11

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

TARGET = $(BIN_DIR)/ex01

INC_DIR = ./include

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	@$(CC) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@$(CC) -c -I$(INC_DIR) $(CFLAGS) -o $@ $<

run: all
	@./$(BIN_DIR)/ex01

clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR)

.PHONY: all run clean


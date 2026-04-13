CC = clang
# CFLAGS = -Wall -Wextra -Werror -std=c11
CFLAGS =  -std=c11

BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
SRC_FILES = $(SRC_DIR)/main.c \
            $(SRC_DIR)/chunk.c \
            $(SRC_DIR)/memory.c

TARGET = $(BIN_DIR)/ex01

INC_DIR = ./include

all: $(SRC_DIR)
	@mkdir -p $(BIN_DIR)
	@$(CC) -I$(INC_DIR) $(CFLAGS) -o $(BIN_DIR)/ex01 $(SRC_FILES)

run: all
	@./$(BIN_DIR)/ex01

clean:
	@rm -rf $(BIN_DIR)

.PHONY: all run clean


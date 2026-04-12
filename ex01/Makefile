CC = clang
CFLAGS = -Wall -Wextra -Werror -std=c11

BIN_DIR = bin
SRC_DIR = src

TARGET = $(BIN_DIR)/ex01

MAIN = $(SRC_DIR)/main.c

INC_DIR = ./include

all: $(SRC_DIR)
	@mkdir -p $(BIN_DIR)
	@$(CC) -I$(INC_DIR) $(MAIN) $(CFLAGS) -o $(BIN_DIR)/ex01

run: all
	@./$(BIN_DIR)/ex01

claen:
	@rm -rf $(BIN_DIR)

.PHONY: all run clean


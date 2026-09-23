CC = clang
CFLAGS = -Wall -Wextra -Werror -std=c11
DEBUG_FLAGS = -g -O0 -DDEBUG_PRINT_CODE -DDEBUG_TRACE_EXECUTION
# Collect on every allocation. Kept apart from GCLOG_FLAGS so that gc-tests can
# stress the collector without GC log lines landing in the expected output.
GC_FLAGS = -DDEBUG_STRESS_GC
GCLOG_FLAGS = -DDEBUG_LOG_GC

inputfile := example/test.txt
logfile := gclog/gc.log

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
APP_OBJ = $(OBJ_DIR)/main.o
DEBUG_OBJ_DIR = debug/obj
DEBUG_BIN_DIR = debug/bin
DEBUG_OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(DEBUG_OBJ_DIR)/%.o,$(SRC_FILES))
DEBUG_TARGET = $(DEBUG_BIN_DIR)/ex01
GC_OBJ_DIR = gc/obj
GC_BIN_DIR = gc/bin
GC_OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(GC_OBJ_DIR)/%.o,$(SRC_FILES))
GC_TARGET = $(GC_BIN_DIR)/ex01
GCLOG_OBJ_DIR = gclog/obj
GCLOG_BIN_DIR = gclog/bin
GCLOG_OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(GCLOG_OBJ_DIR)/%.o,$(SRC_FILES))
GCLOG_TARGET = $(GCLOG_BIN_DIR)/ex01
TEST_SRC_DIR = test/src
TEST_OBJ_DIR = test/obj
TEST_BIN_DIR = test/bin
TEST_SRC_FILES = $(wildcard $(TEST_SRC_DIR)/test_*.c)
TEST_OBJ_FILES = $(patsubst $(TEST_SRC_DIR)/%.c,$(TEST_OBJ_DIR)/%.o,$(TEST_SRC_FILES)) \
								 $(filter-out $(APP_OBJ),$(OBJ_FILES))
TARGET = $(BIN_DIR)/ex01
TEST_TARGET = $(TEST_BIN_DIR)/test_ex01

INC_DIR = ./include
TEST_INC_DIR = ./test/include

all: $(TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJ_FILES)
	@mkdir -p $(DEBUG_BIN_DIR)
	@$(CC) $(DEBUG_FLAGS) -o $@ $^

$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	@$(CC) -o $@ $^

$(TEST_TARGET): $(TEST_OBJ_FILES)
	@mkdir -p $(TEST_BIN_DIR)
	@$(CC) -o $@ $^

$(GC_TARGET): $(GC_OBJ_FILES)
	@mkdir -p $(GC_BIN_DIR)
	@$(CC) -o $@ $^

$(GCLOG_TARGET): $(GCLOG_OBJ_FILES)
	@mkdir -p $(GCLOG_BIN_DIR)
	@$(CC) -o $@ $^

$(DEBUG_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(DEBUG_OBJ_DIR)
	@$(CC) -c -I$(INC_DIR) $(filter-out -Werror -D_FORTIFY_SOURCE=2,$(CFLAGS)) $(DEBUG_FLAGS) -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@$(CC) -c -I$(INC_DIR) $(CFLAGS) -o $@ $<

$(GC_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(GC_OBJ_DIR)
	@$(CC) -c -I$(INC_DIR) $(CFLAGS) $(GC_FLAGS) -o $@ $<

$(GCLOG_OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(GCLOG_OBJ_DIR)
	@$(CC) -c -I$(INC_DIR) $(CFLAGS) $(GC_FLAGS) $(GCLOG_FLAGS) -o $@ $<

$(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c
	@mkdir -p $(TEST_OBJ_DIR)
	@$(CC) -c -I$(INC_DIR) -I$(TEST_INC_DIR) $(CFLAGS) -o $@ $<

test: $(TEST_TARGET)
	@$<

tests: $(TARGET)
	@bash scripts/test_examples.sh $< examples

# Every example, plus the GC-specific ones, run against a build that collects
# on every allocation.
gc-tests: $(GC_TARGET)
	@bash scripts/test_examples.sh $< examples examples/gc

# One file under stress collection with the GC log on, to watch what the
# collector marks, blackens and frees.
gc-run: $(GCLOG_TARGET)
	@./$< $(inputfile)

# The same run, captured to a file. stdout is block-buffered whenever it is
# not a terminal, so redirecting gc-run loses the entire buffer when the
# program crashes -- which is exactly when the log is worth reading. stdbuf
# turns buffering off so every line printed before the crash survives.
gc-log: $(GCLOG_TARGET)
	@mkdir -p $(dir $(logfile))
	@if command -v stdbuf > /dev/null 2>&1; then \
		stdbuf -o0 ./$< $(inputfile) > $(logfile) 2>&1; \
	else \
		echo "warning: stdbuf not found; a crash will truncate $(logfile)" >&2; \
		./$< $(inputfile) > $(logfile) 2>&1; \
	fi; \
	status=$$?; \
	echo "$(logfile): $$(wc -l < $(logfile) | tr -d ' ') lines, exit $$status"

debug: $(DEBUG_TARGET)
	@sh -c 'if [ "$$(uname -s)" = Darwin ]; then exec xcrun lldb "$$1"; else exec lldb "$$1"; fi' sh "$<"

run: $(TARGET)
	@./$< $(inputfile)

repl: $(TARGET)
	@./$<

clean:
	@rm -rf $(BIN_DIR) $(OBJ_DIR) $(TEST_OBJ_DIR) $(TEST_BIN_DIR) \
		$(DEBUG_OBJ_DIR) $(DEBUG_BIN_DIR) $(GC_OBJ_DIR) $(GC_BIN_DIR) \
		$(GCLOG_OBJ_DIR) $(GCLOG_BIN_DIR)

.PHONY: all run clean test tests gc-tests gc-run gc-log repl debug


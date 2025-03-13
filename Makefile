CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./include
LDFLAGS =

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
INCLUDE_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_DIR)/%.o)

TARGET = $(BUILD_DIR)/leancc

.PHONY: all clean test dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(SRC_DIR)
	@mkdir -p $(TEST_DIR)
	@mkdir -p $(INCLUDE_DIR)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

test: dirs $(TEST_OBJS)
	$(CC) $(TEST_OBJS) -o $(BUILD_DIR)/test_runner
	./$(BUILD_DIR)/test_runner

clean:
	rm -rf $(BUILD_DIR)/*

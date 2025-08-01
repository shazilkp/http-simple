# === Compiler & Flags ===
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g

# === Paths ===
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

# === Sources ===
SRC_FILES = $(SRC_DIR)/main.c $(SRC_DIR)/http_handler.c $(SRC_DIR)/socket_handler.c
OBJ_FILES = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# === Target Binary ===
TARGET = $(BIN_DIR)/http-simple

# === Build Rules ===
all: $(TARGET)

# Create binary directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Create object directory
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link all objects
$(TARGET): $(BIN_DIR) $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $(OBJ_FILES)

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
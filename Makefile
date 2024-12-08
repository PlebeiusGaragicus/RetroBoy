# Compiler and tools
GBDK_HOME = ../Resources/gbdk
LCC = $(GBDK_HOME)/bin/lcc

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files and output
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OUTPUT = $(BUILD_DIR)/RetroBoy.gb

# Compiler flags
CFLAGS = -Wa-l -Wl-m -Wl-j -msm83:gb

# Main target
all: $(OUTPUT)

# Make sure the build directory exists and compile
$(OUTPUT): $(SOURCES) | $(BUILD_DIR)
	@echo "Compiling $(SOURCES) to $(OUTPUT)..."
	@echo "Running: $(LCC) $(CFLAGS) -o $(OUTPUT) $(SOURCES)"
	$(LCC) $(CFLAGS) -o $(OUTPUT) $(SOURCES)

$(BUILD_DIR):
	@echo "Creating build directory..."
	mkdir -p $(BUILD_DIR)

# Clean rule
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

.PHONY: all clean
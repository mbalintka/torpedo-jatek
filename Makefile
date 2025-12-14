# ====================================================================
# Makefile for Torpedo Game Client (Linux) and Arduino Upload
#
# Assumes:
# 1. The C client source is named 'torpedo_jatek.c'.
# 2. The Arduino sketch is named 'torpedo.ino'.
# 3. 'arduino-cli' is installed for the 'upload' target.
# ====================================================================

# --- C CLIENT CONFIGURATION ---
CC = gcc
# -Wall: Enable all common warnings
# -g: Include debugging symbols
# -std=c99: Use the C99 standard
# -lutil: Link utility library (sometimes needed for select())
CFLAGS = -Wall -g -std=c99 -lutil
TARGET = torpedo_jatek
SOURCE = torpedo_jatek.c


# --- ARDUINO CONFIGURATION ---
# IMPORTANT: Change these values to match your specific setup.
ARDUINO_CLI = arduino-cli
BOARD = arduino:avr:uno
PORT = /dev/ttyACM0
SKETCH = torpedo.ino


# --- PHONY TARGETS (Actions, not files) ---
.PHONY: all clean run upload

# Default target: compile the executable
all: $(TARGET)

# --------------------------------------------------------------------
# 1. LINUX C CLIENT RULES
# --------------------------------------------------------------------

# Rule to compile the Linux C Client
$(TARGET): $(SOURCE)
	@echo "Compiling C client: $@"
	$(CC) $(CFLAGS) -o $@ $<

# Rule to run the compiled client
run: $(TARGET)
	@echo "--- Starting Torpedo Game Client ---"
	./$(TARGET)

# Rule to clean up generated files
clean:
	@echo "Cleaning up generated files..."
	rm -f $(TARGET)

# --------------------------------------------------------------------
# 2. ARDUINO UPLOAD RULE
# --------------------------------------------------------------------

# Rule to compile and upload the Arduino sketch
upload:
	@echo "--------------------------------------------------------"
	@echo "Uploading $(SKETCH) to $(PORT)..."
	@echo "--------------------------------------------------------"
	# 1. Compile the sketch
	$(ARDUINO_CLI) compile --fqbn $(BOARD) $(SKETCH)
	# 2. Upload the compiled sketch
	$(ARDUINO_CLI) upload -p $(PORT) --fqbn $(BOARD) $(SKETCH)
	@echo ""
	@echo "Upload complete. Use 'make run' to connect the client."
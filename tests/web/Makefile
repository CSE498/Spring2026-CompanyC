# Makefile for native web-UI tests
#
# Canvas, Image, and Textbox all have #ifndef __EMSCRIPTEN__ stubs and can be
# compiled and run with a plain C++ compiler.  WebButton requires Emscripten
# and can only be tested in a browser via the main web demo build.
#
# Usage (from tests/web/):
#   make          # build and run all three native test binaries
#   make build    # build only
#   make clean

SRC_DIR  := ../../source
CXX      := c++
CXXFLAGS := -std=c++23 -O0 -g -Wall -Wextra -Wpedantic
INC      := -I$(SRC_DIR)

BUILD_DIR := build
BINS      := $(BUILD_DIR)/canvas_tests \
             $(BUILD_DIR)/image_tests  \
             $(BUILD_DIR)/textbox_tests

.PHONY: all build test clean

all: test

build: $(BINS)

test: build
	@echo "--- WebCanvas ---"
	$(BUILD_DIR)/canvas_tests
	@echo "--- WebImage ---"
	$(BUILD_DIR)/image_tests
	@echo "--- WebTextbox ---"
	$(BUILD_DIR)/textbox_tests
	@echo "All native web tests passed."

$(BUILD_DIR):
	@mkdir -p $@

$(BUILD_DIR)/canvas_tests: WebCanvasTest.cpp $(SRC_DIR)/tools/WebCanvas.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INC) $^ -o $@

$(BUILD_DIR)/image_tests: WebImageTest.cpp $(SRC_DIR)/tools/WebImage.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INC) $^ -o $@

$(BUILD_DIR)/textbox_tests: WebTextboxTest.cpp $(SRC_DIR)/tools/WebTextbox.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INC) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all norun clean rebuild cmake_force

BUILD_DIR = build
BUILD_MAKEFILE = $(BUILD_DIR)/Makefile
CMAKE_GEN = cmake -S . -B $(BUILD_DIR)

all: $(BUILD_MAKEFILE)
	@echo "--- Building project ---"
	$(MAKE) -C $(BUILD_DIR)

cmake_force:
	@echo "--- Forcing CMake configuration ---"
	$(CMAKE_GEN)

$(BUILD_MAKEFILE): cmake_force

clean:
	@echo "--- Cleaning build directory ---"
	rm -rf $(BUILD_DIR)/
	
rebuild: clean all

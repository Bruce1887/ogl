.PHONY: all debug release norun clean clean-debug clean-release rebuild cmake_force

DEBUG_BUILD_DIR = build/debug
RELEASE_BUILD_DIR = build/release
CMAKE_GEN = cmake -S .

all: debug

debug: $(DEBUG_BUILD_DIR)/Makefile
	@echo "--- Building project in debug mode ---"
	$(MAKE) -C $(DEBUG_BUILD_DIR)

release: $(RELEASE_BUILD_DIR)/Makefile
	@echo "--- Building project in release mode ---"
	$(MAKE) -C $(RELEASE_BUILD_DIR)

$(DEBUG_BUILD_DIR)/Makefile:
	@echo "--- Configuring CMake for Debug ---"
	$(CMAKE_GEN) -B $(DEBUG_BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

$(RELEASE_BUILD_DIR)/Makefile:
	@echo "--- Configuring CMake for Release ---"
	$(CMAKE_GEN) -B $(RELEASE_BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

cmake_force_debug:
	@echo "--- Forcing CMake configuration for Debug ---"
	$(CMAKE_GEN) -B $(DEBUG_BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

cmake_force_release:
	@echo "--- Forcing CMake configuration for Release ---"
	$(CMAKE_GEN) -B $(RELEASE_BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

clean-debug:
	@echo "--- Cleaning debug build directory ---"
	rm -rf $(DEBUG_BUILD_DIR)/

clean-release:
	@echo "--- Cleaning release build directory ---"
	rm -rf $(RELEASE_BUILD_DIR)/

clean: clean-debug clean-release
	@echo "--- Cleaning all build directories ---"

rebuild-debug: clean-debug debug

rebuild-release: clean-release release

rebuild: clean debug release
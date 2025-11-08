.PHONY: all norun clean 

BUILD_MAKEFILE = build/Makefile

CMAKE_GEN = cmake -S . -B build/

# Default target: build (and run) the project
all: $(BUILD_MAKEFILE)
	$(MAKE) -C build/

# Create or update the CMake build system
$(BUILD_MAKEFILE):
	$(CMAKE_GEN)

clean:
	rm -rf build/

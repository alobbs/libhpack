.PHONY: all clean test doc

BUILD_DOCS=ON
BUILD_TESTS=ON

define run-cmake
	mkdir -p build
	cd build ; cmake -DBUILD_DOCS=$(BUILD_DOCS) -DBUILD_TESTS=$(BUILD_TESTS) $(1) .. ; cd ..
endef

all:
	$(call run-cmake)
	$(MAKE) -C build MAKEFLAGS="VERBOSE=1"

debug:
	$(call run-cmake, -DCMAKE_BUILD_TYPE:STRING=Debug)
	$(MAKE) -C build MAKEFLAGS="VERBOSE=1"

clean:
	rm -rf build

doc:
ifeq ($(wildcard ./build/Makefile),)
	$(call run-cmake)
endif
	$(MAKE) -C build doc

test: all
	$(MAKE) -C build test ARGS="-V" | tools/highlight-ctest.py

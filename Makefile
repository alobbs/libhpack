.PHONY: all clean test

all:
	mkdir -p build
	cd build ; cmake .. ; cd ..
	$(MAKE) -C build MAKEFLAGS="VERBOSE=1"

debug:
	mkdir -p build
	cd build ; cmake -DCMAKE_BUILD_TYPE:STRING=Debug .. ; cd ..
	$(MAKE) -C build MAKEFLAGS="VERBOSE=1"

clean:
	rm -rf build

doc:
	$(MAKE) -C build docs docs2

test: all
	$(MAKE) -C build test ARGS="-V" | tools/highlight-ctest.py

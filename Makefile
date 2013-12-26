.PHONY: all clean test

all:
	mkdir -p build
	cd build ; cmake .. ; cd ..
	$(MAKE) -C build MAKEFLAGS="VERBOSE=1"

clean:
	rm -rf build

test: all
	./build/test/test_libhpack

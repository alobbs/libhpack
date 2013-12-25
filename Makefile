.PHONY: all clean test

all:
	mkdir -p build
	cd build ; cmake .. && make VERBOSE=1 ; cd ..

clean:
	rm -rf build

test: all
	./build/test/test_libhpack

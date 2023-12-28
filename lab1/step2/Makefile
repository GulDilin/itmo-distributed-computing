all: clean build tar

# Compile an executable named pa3
build:
	mkdir -p ./build/pa3
	clang -std=c99 -Wall -pedantic *.c -o ./build/pa3.o

tar:
	mkdir -p ./build/pa3
	cp *.c ./build/pa3
	cp *.h ./build/pa3
	cd build && tar cfvz ./pa3.tar.gz ./pa3

clean:
	rm -rf ./build

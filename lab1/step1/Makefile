all: clean build tar

# Compile an executable named pa1
build:
	mkdir -p ./build/pa1
	clang -std=c99 -Wall -pedantic *.c -o ./build/pa1.o

tar:
	mkdir -p ./build/pa1
	cp *.c ./build/pa1
	cp *.h ./build/pa1
	cd build && tar cfvz ./pa1.tar.gz ./pa1

clean:
	rm -rf ./build

all: clean build tar

# Compile an executable named pa4
build:
	mkdir -p ./build/pa4
	export LD_LIBRARY_PATH="${PWD}/lib32"
	clang -std=c99 -Wall -pedantic *.c -o ./build/pa4.o ./lib64/libruntime.so -Wl,-rpath,/c/Users/zheny/projects/edu/itmo-distributed-computing/lab1/step3/lib64

tar:
	mkdir -p ./build/pa4
	cp *.c ./build/pa4
	cp *.h ./build/pa4
	cd build && tar cfvz ./pa4.tar.gz ./pa4

clean:
	rm -rf ./build

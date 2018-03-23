all: ThreadPool_test.out

ThreadPool_test.out: ThreadPool.o
	clang++ --std=c++11 -lpthread ThreadPool.o test/ThreadPool_test.cpp -o ThreadPool_test.out

ThreadPool.o:
	clang++ --std=c++11 -c src/ThreadPool.cpp -o ThreadPool.o

clean:
	rm -rf *.o *.out
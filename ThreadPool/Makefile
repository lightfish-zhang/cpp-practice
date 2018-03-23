all: ThreadPool_test.out

ThreadPool_test.out:
	clang++ --std=c++11 -lpthread test/ThreadPool_test.cpp -o ThreadPool_test.out

clean:
	rm -rf *.o *.out
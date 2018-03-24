all: docker_test.out

docker_test.out:
	clang++ --std=c++11 test/docker_test.cpp -o docker_test.out

clean:
	rm -rf *.o *.out
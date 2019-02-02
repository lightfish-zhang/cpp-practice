#include <iostream>
#include <cassert>
#include <ctime>
#include <vector>

// #include "MemoryPool.hpp"
#include "../src/StackAlloc.hpp"

// 根据电脑性能调整这些值
// 插入元素个数
#define ELEMS 25000000
// 重复次数
#define REPS 50

int main()
{
    clock_t start;

    // 使用默认分配器
    StackAlloc<int, std::allocator<int> > stackDefault;
    start = clock();
    for (int j = 0; j < REPS; j++) {
        assert(stackDefault.empty());
        for (int i = 0; i < ELEMS; i++)
          stackDefault.push(i);
        for (int i = 0; i < ELEMS; i++)
          stackDefault.pop();
    }
    std::cout << "Default Allocator Time: ";
    std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

    // 使用内存池
    // StackAlloc<int, MemoryPool<int> > stackPool;
    // start = clock();
    // for (int j = 0; j < REPS; j++) {
    //     assert(stackPool.empty());
    //     for (int i = 0; i < ELEMS; i++)
    //       stackPool.push(i);
    //     for (int i = 0; i < ELEMS; i++)
    //       stackPool.pop();
    // }
    // std::cout << "MemoryPool Allocator Time: ";
    // std::cout << (((double)clock() - start) / CLOCKS_PER_SEC) << "\n\n";

    return 0;
}
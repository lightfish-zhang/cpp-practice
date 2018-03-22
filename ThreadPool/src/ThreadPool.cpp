#include "./ThreadPool.hpp"

using namespace Practice;

inline ThreadPool::ThreadPool(size_t threadNum)
: stop(false)
{
    // spawn threads
    for(size_t i=0; i<threadNum; ++i){
        workers.emplace_back(
            [this]
            {
                
            }
        );
    }

}
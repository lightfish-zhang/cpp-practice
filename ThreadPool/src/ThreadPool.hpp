#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>               // std::vector
#include <queue>                // std::queue
#include <memory>               // std::make_shared
#include <stdexcept>            // std::runtime_error
#include <thread>               // std::thread
#include <mutex>                // std::mutex,        std::unique_lock
#include <condition_variable>   // std::condition_variable
#include <future>               // std::future,       std::packaged_task
#include <functional>           // std::function,     std::bind
#include <utility>              // std::move,         std::forward

namespace Practice {

class ThreadPool {
public:
    // spawn worker threads in pool
    ThreadPool(size_t threadNum);

    // add thread in pool
    /* 
      F&& 来进行右值引用，而不会发生拷贝行为而再次创建一个执行体
      -> 尾置写法
      std::result_of<F(Args...)>::type　推断出　F的返回类型
      std::future 获取异步任务的结果（简化了这个过程：声明一个全局变量且调用一个线程等待异步任务的结果）
     */
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;

    /* 
     * anthor code , it's not beautifull
     * 
     template<class F, class... Args>
        std::future<typename std::result_of<F(Args...)>::type> 
        enqueue(F&& f, Args&&... args)    
     */

    ~ ThreadPool();

private:
    std::vector< std::thread > workers;
    std::queue< std::function<void()> > tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;

    bool stop;
};

}
#endif
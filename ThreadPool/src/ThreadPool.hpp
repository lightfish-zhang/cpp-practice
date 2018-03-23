/* 
C++中每一个对象所占用的空间大小，是在编译的时候就确定的，
在模板类没有真正的被使用之前，编译器是无法知道，模板类中使用模板类型的对象的所占用的空间的大小的。
只有模板被真正使用的时候，编译器才知道，模板套用的是什么类型，应该分配多少空间。
这也就是模板类为什么只是称之为模板，而不是泛型的缘故。
既然是在编译的时候，根据套用的不同类型进行编译，那么，套用不同类型的模板类实际上就是两个不同的类型,它们分配的空间大小是不一样的。
*/


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

/* 
define
*/
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



/* 
implenent
*/

inline ThreadPool::ThreadPool(size_t threadNum)
: stop(false)
{
    // spawn threads
    for(size_t i=0; i<threadNum; ++i){
        // Practice::ThreadPool *const this
        workers.emplace_back([this](){
            while(true){
                std::function<void()> currentTask;

                // Resource Acquisition is Initialization (RAII)
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this](){
                        return this->stop || !this->tasks.empty();
                    });
                    // if this->stop is true, it mean that not any new task would insert
                    if(this->stop && this->tasks.empty())
                        return;

                    // now , tasks is not empty , it must to be done
                    currentTask = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                currentTask();
            }
        });
    }

}

/* 
线程池的销毁对应构造时究竟创建了什么实例，
销毁线程池之前，我们并不知道当前线程池中的工作线程是否执行完成，
所以必须先创建一个临界区将线程池状态标记为停止，从而禁止新的线程的加入，
最后等待所有执行线程的运行结束，完成销毁，详细实现如下：

*/
inline ThreadPool::~ThreadPool()
{
    // Resource Acquisition is Initialization (RAII)
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }

    // notify all thread that run `condition.wait()`
    condition.notify_all();

    // collect worker threads one by one
    for(std::thread &worker: workers)
        worker.join();
}


/* 
向线程池中添加新任务的实现逻辑主要需要注意这样几点：

支持多个入队任务参数需要使用变长模板参数
为了调度执行的任务，需要包装执行的任务，这就意味着需要对任务函数的类型进行包装、构造
临界区可以在一个作用域里面被创建，最佳实践是使用 RAII 的形式

*/

template<typename F, typename... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    // using 与模板搭配使用，推导出模板函数的返回类型，而 typedef 无法与模板使用
    using return_type = typename std::result_of<F(Args...)>::type;

    // wrapper function, 获取一个任务的智能指针
    auto task = std::make_shared< std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    // 获得 std::future 对象以供实施线程同步
    std::future<return_type> res = task->get_future();

    // 临界区
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // 禁止在线程池停止后加入新的线程
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        // 将线程添加到执行任务队列中
        tasks.emplace([task]{ (*task)(); });
    }

    condition.notify_one();
    return res;
}



}
#endif
#include "./ThreadPool.hpp"

using namespace Practice;

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

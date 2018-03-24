#include <sys/wait.h>   // waitpid
#include <sys/mount.h>  // mount
#include <fcntl.h>      // open
#include <unistd.h>     // execv, sethostname, chroot, fchdir
#include <sched.h>      // clone

// C 标准库
#include <cstring>

// C++ 标准库
#include <string>       // std::string

#define STACK_SIZE (512 * 512) // 定义子进程空间大小

namespace docker {
    

    // 定义一些增加可读性的变量
    typedef int proc_statu;
    proc_statu proc_err  = -1;
    proc_statu proc_exit = 0;
    proc_statu proc_wait = 1;


    // docker 容器启动配置
    typedef struct container_config {
        std::string host_name;      // 主机名
        std::string root_dir;       // 容器根目录
    } container_config;


    // 定义容器类 container，并让它在构造方法中完成对容器的相关配置
    class container {
    private:
        // 可读性增强
        typedef int process_pid;

        // 子进程栈
        char child_stack[STACK_SIZE];

        // 容器配置
        container_config config;

        // bash
        void start_bash() {
            // 在C++中执行C语言函数，需要对传参转换类型
            // 将 C++ std::string 安全的转换为 C 风格的字符串 char *
            // 从 C++14 开始, C++编译器将禁止这种写法 `char *str = "test";`
            std::string bash = "/bin/bash";
            char *c_bash = new char[bash.length()+1];   // +1 用于存放 '\0'
            strcpy(c_bash, bash.c_str());

            char* const child_args[] = { c_bash, NULL };
            execv(child_args[0], child_args);           // 在子进程中执行 /bin/bash
            delete []c_bash;
        }

        // 设置根目录
        void set_rootdir() {

            // chdir 系统调用, 切换到某个目录下
            chdir(this->config.root_dir.c_str());

            // chrrot 系统调用, 设置根目录, 因为刚才已经切换过当前目录
            // 故直接使用当前目录作为根目录即可
            chroot(".");
        }

        // 设置容器主机名
        void set_hostname() {
            // sethostname()/gethostname() These system calls are used to access or to change the hostname of the current processor
            sethostname(this->config.host_name.c_str(), this->config.host_name.length());
        }

        // 设置独立的进程空间
        void set_procsys() {
            // 挂载 proc 文件系统
            mount("none", "/proc", "proc", 0, nullptr);
            mount("none", "/sys", "sysfs", 0, nullptr);
        }

    public:
        container(container_config &config) {
            this->config = config;
        }

        // 启动容器
        int start() {
            int status_code;

            auto setup = [](void *args) -> int {
                auto _this = reinterpret_cast<container *>(args); // 指针类型转换

                // 对容器进行相关配置
                _this->set_rootdir();
                _this->set_hostname();
                _this->set_procsys();

                _this->start_bash();

                return proc_wait;
            };

            // 其中pid、net、ipc、mnt、uts 等命名空间将容器的进程、网络、消息、文件系统和hostname 隔离开

            process_pid child_pid = clone(setup, child_stack+STACK_SIZE, // 移动到栈底
                                // 使用新的 namespace 后，执行需要 sudo 权限
                                CLONE_NEWNS|        // new Mount 设置单独的文件系统
                                // CLONE_NEWNET|    // new Net namespace
                                CLONE_NEWUTS|       // new UTS namespace hostname
                                CLONE_NEWPID|       // new PID namaspace，表现为：在容器里使用 ps 会只看到容器进程（祖先）的子孙进程，且进程id数值较小
                                SIGCHLD,            // 子进程退出时会发出信号给父进程
                                this);
            waitpid(child_pid, &status_code, 0); // 等待子进程的退出
            return status_code;
        }

    };

}
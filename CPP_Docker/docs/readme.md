# system call


## clone

```c
int clone(int (*fn)(void *), void *child_stack, int flags, void *arg);
```

进程主要由四个要素组成：

一段需要执行的程序
进程自己的专用堆栈空间
进程控制块（PCB）
进程专有的 Namespace

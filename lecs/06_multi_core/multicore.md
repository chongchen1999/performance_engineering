# POSIX 线程 (pthread) 完整指南

## 1. 概述

POSIX 线程（pthread）是一个遵循 IEEE POSIX 1003.1c 标准的线程库，为 Unix/Linux 系统提供标准化的多线程编程接口。线程是在同一进程内部并发执行的多个控制流，共享进程的地址空间和资源，但有各自独立的执行上下文。

**pthread 的主要优势**：
- 轻量级：创建和管理线程的开销远小于进程
- 共享内存：所有线程共享相同的地址空间
- 快速上下文切换：线程间切换比进程切换快得多
- 适合并行计算：可以充分利用多核处理器

## 2. 基础知识

### 2.1 编译和链接

使用 pthread 库需要包含头文件并链接库：

```c
#include <pthread.h>

// 编译命令
// gcc program.c -o program -lpthread
```

### 2.2 线程标识符

每个线程都由唯一的线程 ID 标识，类型为 `pthread_t`：

```c
pthread_t thread_id;
pthread_t self_id = pthread_self(); // 获取当前线程ID
```

## 3. 线程生命周期管理

### 3.1 创建线程

```c
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine) (void *), void *arg);
```

**参数详解**：
- `thread`：指向 pthread_t 变量的指针，用于存储新线程 ID
- `attr`：线程属性，可以为 NULL 表示使用默认属性
- `start_routine`：线程执行的函数
- `arg`：传递给线程函数的参数

**返回值**：成功返回 0，失败返回错误码

```c
void *thread_function(void *arg) {
    int *value = (int *)arg;
    printf("线程接收到参数: %d\n", *value);
    // 线程工作...
    return NULL;
}

int main() {
    pthread_t thread;
    int value = 42;
    
    int ret = pthread_create(&thread, NULL, thread_function, &value);
    if (ret != 0) {
        fprintf(stderr, "创建线程失败: %s\n", strerror(ret));
        return 1;
    }
    
    // 主线程继续执行...
    
    return 0;
}
```

### 3.2 终止线程

线程终止的方式：

1. **自然终止**：从启动函数返回
2. **显式终止**：调用 `pthread_exit()`
3. **被取消**：由其他线程调用 `pthread_cancel()`

```c
void pthread_exit(void *retval);
```

**参数**：
- `retval`：线程返回值，可被 `pthread_join()` 获取

```c
void *thread_function(void *arg) {
    // 线程工作...
    pthread_exit((void *)42); // 返回值为整数42
}
```

### 3.3 等待线程

线程连接用于等待其他线程终止并收集其返回值：

```c
int pthread_join(pthread_t thread, void **retval);
```

**参数**：
- `thread`：要等待的线程 ID
- `retval`：用于存储线程返回值的指针，可以为 NULL

```c
void *thread_function(void *arg) {
    return (void *)123; // 返回值
}

int main() {
    pthread_t thread;
    void *ret_val;
    
    pthread_create(&thread, NULL, thread_function, NULL);
    
    // 等待线程结束
    pthread_join(thread, &ret_val);
    
    printf("线程返回值: %ld\n", (long)ret_val);
    
    return 0;
}
```

### 3.4 分离线程

分离状态的线程在终止时会自动释放资源，不需要被连接：

```c
int pthread_detach(pthread_t thread);
```

```c
pthread_t thread;
pthread_create(&thread, NULL, thread_function, NULL);

// 分离线程
pthread_detach(thread);
```

## 4. 线程同步

### 4.1 互斥锁 (Mutex)

互斥锁用于保护共享资源，确保任一时刻只有一个线程可以访问：

#### 初始化和销毁

```c
// 静态初始化
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// 动态初始化
pthread_mutex_t mutex;
pthread_mutex_init(&mutex, NULL);

// 销毁
pthread_mutex_destroy(&mutex);
```

#### 加锁和解锁

```c
// 加锁（阻塞）
pthread_mutex_lock(&mutex);

// 尝试加锁（非阻塞）
int ret = pthread_mutex_trylock(&mutex);

// 解锁
pthread_mutex_unlock(&mutex);
```

#### 互斥锁使用示例

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int shared_counter = 0;

void *increment_counter(void *arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex);
        shared_counter++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t threads[5];
    
    // 创建5个线程
    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, increment_counter, NULL);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("最终计数器值: %d\n", shared_counter);
    pthread_mutex_destroy(&mutex);
    
    return 0;
}
```

### 4.2 条件变量

条件变量用于线程间的通知，当某一条件满足时通知等待的线程：

#### 初始化和销毁

```c
// 静态初始化
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// 动态初始化
pthread_cond_t cond;
pthread_cond_init(&cond, NULL);

// 销毁
pthread_cond_destroy(&cond);
```

#### 等待和通知

```c
// 等待条件（必须先获取互斥锁）
pthread_cond_wait(&cond, &mutex);

// 带超时的等待
struct timespec ts;
clock_gettime(CLOCK_REALTIME, &ts);
ts.tv_sec += 2; // 2秒后超时
pthread_cond_timedwait(&cond, &mutex, &ts);

// 通知一个等待的线程
pthread_cond_signal(&cond);

// 通知所有等待的线程
pthread_cond_broadcast(&cond);
```

#### 条件变量使用示例

```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int data_ready = 0;
int shared_data = 0;

// 生产者线程
void *producer(void *arg) {
    sleep(1); // 模拟工作
    
    pthread_mutex_lock(&mutex);
    shared_data = 42; // 产生数据
    data_ready = 1;   // 设置条件
    
    // 通知消费者
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

// 消费者线程
void *consumer(void *arg) {
    pthread_mutex_lock(&mutex);
    
    // 等待数据准备好
    while (!data_ready) {
        printf("消费者: 等待数据...\n");
        pthread_cond_wait(&cond, &mutex);
    }
    
    // 处理数据
    printf("消费者: 读取数据 %d\n", shared_data);
    
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;
    
    // 创建线程
    pthread_create(&cons_thread, NULL, consumer, NULL);
    pthread_create(&prod_thread, NULL, producer, NULL);
    
    // 等待线程结束
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    
    // 清理
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    
    return 0;
}
```

### 4.3 读写锁

读写锁适用于读多写少的场景，允许多个读取者同时获取锁：

```c
// 初始化
pthread_rwlock_t rwlock;
pthread_rwlock_init(&rwlock, NULL);

// 获取读锁
pthread_rwlock_rdlock(&rwlock);

// 获取写锁
pthread_rwlock_wrlock(&rwlock);

// 尝试获取锁（非阻塞）
pthread_rwlock_tryrdlock(&rwlock);
pthread_rwlock_trywrlock(&rwlock);

// 释放锁
pthread_rwlock_unlock(&rwlock);

// 销毁
pthread_rwlock_destroy(&rwlock);
```

### 4.4 自旋锁

自旋锁在等待过程中持续尝试获取锁，而不是让线程睡眠，适用于短时间持有的锁：

```c
// 初始化
pthread_spinlock_t spinlock;
pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);

// 获取锁
pthread_spin_lock(&spinlock);

// 尝试获取锁（非阻塞）
pthread_spin_trylock(&spinlock);

// 释放锁
pthread_spin_unlock(&spinlock);

// 销毁
pthread_spin_destroy(&spinlock);
```

### 4.5 屏障 (Barrier)

屏障用于同步多个线程到达同一点后再继续执行：

```c
// 初始化，设置需要等待的线程数
pthread_barrier_t barrier;
pthread_barrier_init(&barrier, NULL, NUM_THREADS);

// 等待屏障
pthread_barrier_wait(&barrier);

// 销毁
pthread_barrier_destroy(&barrier);
```

## 5. 线程属性

### 5.1 线程属性设置

```c
// 初始化属性对象
pthread_attr_t attr;
pthread_attr_init(&attr);

// 分离状态
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); // 默认

// 栈大小
size_t stacksize = 2 * 1024 * 1024; // 2MB
pthread_attr_setstacksize(&attr, stacksize);

// 获取当前栈大小
size_t current_stacksize;
pthread_attr_getstacksize(&attr, &current_stacksize);

// 使用属性创建线程
pthread_create(&thread, &attr, thread_function, NULL);

// 销毁属性对象
pthread_attr_destroy(&attr);
```

### 5.2 调度策略和优先级

```c
// 设置调度策略
pthread_attr_setschedpolicy(&attr, SCHED_FIFO); // 先入先出
// 其他选项: SCHED_RR (轮转), SCHED_OTHER (默认)

// 设置优先级
struct sched_param param;
param.sched_priority = 50;
pthread_attr_setschedparam(&attr, &param);

// 设置是否继承创建者的调度属性
pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED); // 使用显式设置
// PTHREAD_INHERIT_SCHED: 继承父线程的调度属性（默认）
```

## 6. 高级特性

### 6.1 线程特定数据 (Thread-Specific Data)

线程特定数据提供每个线程专用的数据存储：

```c
// 创建键
pthread_key_t key;
pthread_key_create(&key, free_function);
// free_function 是可选的，当线程终止时会调用此函数清理数据

// 设置线程特定数据
void *data = malloc(sizeof(int));
*(int *)data = 42;
pthread_setspecific(key, data);

// 获取线程特定数据
void *value = pthread_getspecific(key);

// 删除键
pthread_key_delete(key);
```

### 6.2 一次性初始化

保证初始化代码只执行一次：

```c
pthread_once_t once_control = PTHREAD_ONCE_INIT;

void init_function(void) {
    // 只会执行一次的初始化代码
    printf("初始化完成\n");
}

// 在需要初始化的地方调用
pthread_once(&once_control, init_function);
```

### 6.3 线程取消

允许一个线程请求终止另一个线程：

```c
// 发送取消请求
pthread_cancel(thread_id);

// 设置取消状态（是否可以被取消）
pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);  // 默认
pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL); // 禁用取消

// 设置取消类型
pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);     // 在取消点检查（默认）
pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); // 随时可取消

// 设置取消点
pthread_testcancel();

// 设置清理处理程序
pthread_cleanup_push(cleanup_function, arg);
// ...代码...
pthread_cleanup_pop(execute); // 1: 执行清理，0: 不执行
```

### 6.4 信号处理

```c
// 屏蔽信号
sigset_t set;
sigemptyset(&set);
sigaddset(&set, SIGINT);
pthread_sigmask(SIG_BLOCK, &set, NULL);

// 等待信号
int sig;
sigwait(&set, &sig);
printf("收到信号: %d\n", sig);
```

## 7. 线程安全与同步模式

### 7.1 避免死锁的技巧

1. **按顺序获取锁**：如需多个锁，总是按固定顺序获取
2. **避免嵌套锁**：尽量不要在持有锁的情况下再获取锁
3. **使用带超时的加锁函数**：防止无限等待
4. **使用锁层次结构**：为锁分配层级，只允许获取更高层级的锁

### 7.2 常见同步模式

#### 生产者-消费者模式

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 10

// 环形缓冲区
typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} ring_buffer_t;

// 初始化缓冲区
void ring_buffer_init(ring_buffer_t *rb) {
    rb->in = 0;
    rb->out = 0;
    rb->count = 0;
    pthread_mutex_init(&rb->mutex, NULL);
    pthread_cond_init(&rb->not_empty, NULL);
    pthread_cond_init(&rb->not_full, NULL);
}

// 插入元素
void ring_buffer_put(ring_buffer_t *rb, int item) {
    pthread_mutex_lock(&rb->mutex);
    
    // 等待缓冲区不满
    while (rb->count == BUFFER_SIZE) {
        pthread_cond_wait(&rb->not_full, &rb->mutex);
    }
    
    rb->buffer[rb->in] = item;
    rb->in = (rb->in + 1) % BUFFER_SIZE;
    rb->count++;
    
    // 通知消费者
    pthread_cond_signal(&rb->not_empty);
    pthread_mutex_unlock(&rb->mutex);
}

// 获取元素
int ring_buffer_get(ring_buffer_t *rb) {
    pthread_mutex_lock(&rb->mutex);
    
    // 等待缓冲区不空
    while (rb->count == 0) {
        pthread_cond_wait(&rb->not_empty, &rb->mutex);
    }
    
    int item = rb->buffer[rb->out];
    rb->out = (rb->out + 1) % BUFFER_SIZE;
    rb->count--;
    
    // 通知生产者
    pthread_cond_signal(&rb->not_full);
    pthread_mutex_unlock(&rb->mutex);
    
    return item;
}

// 清理缓冲区
void ring_buffer_destroy(ring_buffer_t *rb) {
    pthread_mutex_destroy(&rb->mutex);
    pthread_cond_destroy(&rb->not_empty);
    pthread_cond_destroy(&rb->not_full);
}
```

#### 读者-写者模式

使用 pthread_rwlock 实现优先读者的读写锁：

```c
pthread_rwlock_t rwlock;
pthread_rwlock_init(&rwlock, NULL);

// 读者
void *reader(void *arg) {
    int id = *(int *)arg;
    
    while (1) {
        pthread_rwlock_rdlock(&rwlock);
        printf("读者 %d: 开始读取\n", id);
        // 读取操作...
        sleep(1);
        printf("读者 %d: 结束读取\n", id);
        pthread_rwlock_unlock(&rwlock);
        
        sleep(rand() % 3);
    }
    
    return NULL;
}

// 写者
void *writer(void *arg) {
    int id = *(int *)arg;
    
    while (1) {
        pthread_rwlock_wrlock(&rwlock);
        printf("写者 %d: 开始写入\n", id);
        // 写入操作...
        sleep(2);
        printf("写者 %d: 结束写入\n", id);
        pthread_rwlock_unlock(&rwlock);
        
        sleep(rand() % 5);
    }
    
    return NULL;
}
```

## 8. 线程池实现

线程池是一种线程复用的模式，避免频繁创建和销毁线程的开销：

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 4
#define MAX_QUEUE 256

// 任务定义
typedef struct {
    void (*function)(void *);
    void *argument;
} task_t;

// 线程池结构
typedef struct {
    pthread_t threads[NUM_THREADS];
    task_t queue[MAX_QUEUE];
    int queue_size;
    int queue_head;
    int queue_tail;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_not_empty;
    pthread_cond_t queue_not_full;
    int shutdown;
} threadpool_t;

// 工作线程函数
void *worker(void *arg) {
    threadpool_t *pool = (threadpool_t *)arg;
    task_t task;
    
    while (1) {
        // 获取队列锁
        pthread_mutex_lock(&(pool->queue_lock));
        
        // 等待队列非空或关闭信号
        while (pool->queue_size == 0 && !pool->shutdown) {
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->queue_lock));
        }
        
        // 检查是否关闭
        if (pool->shutdown) {
            pthread_mutex_unlock(&(pool->queue_lock));
            pthread_exit(NULL);
        }
        
        // 从队列中获取任务
        task.function = pool->queue[pool->queue_head].function;
        task.argument = pool->queue[pool->queue_head].argument;
        
        pool->queue_head = (pool->queue_head + 1) % MAX_QUEUE;
        pool->queue_size--;
        
        // 通知队列不再满
        pthread_cond_signal(&(pool->queue_not_full));
        
        // 释放队列锁
        pthread_mutex_unlock(&(pool->queue_lock));
        
        // 执行任务
        (*(task.function))(task.argument);
    }
    
    pthread_exit(NULL);
}

// 初始化线程池
threadpool_t *threadpool_init() {
    threadpool_t *pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    
    // 初始化结构
    pool->queue_size = 0;
    pool->queue_head = 0;
    pool->queue_tail = 0;
    pool->shutdown = 0;
    
    // 初始化同步原语
    pthread_mutex_init(&(pool->queue_lock), NULL);
    pthread_cond_init(&(pool->queue_not_empty), NULL);
    pthread_cond_init(&(pool->queue_not_full), NULL);
    
    // 创建工作线程
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&(pool->threads[i]), NULL, worker, (void *)pool);
    }
    
    return pool;
}

// 添加任务到线程池
int threadpool_add_task(threadpool_t *pool, void (*function)(void *), void *argument) {
    // 获取队列锁
    pthread_mutex_lock(&(pool->queue_lock));
    
    // 等待队列非满
    while (pool->queue_size == MAX_QUEUE && !pool->shutdown) {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->queue_lock));
    }
    
    // 检查是否关闭
    if (pool->shutdown) {
        pthread_mutex_unlock(&(pool->queue_lock));
        return -1;
    }
    
    // 添加任务到队列
    pool->queue[pool->queue_tail].function = function;
    pool->queue[pool->queue_tail].argument = argument;
    
    pool->queue_tail = (pool->queue_tail + 1) % MAX_QUEUE;
    pool->queue_size++;
    
    // 通知队列非空
    pthread_cond_signal(&(pool->queue_not_empty));
    
    // 释放队列锁
    pthread_mutex_unlock(&(pool->queue_lock));
    
    return 0;
}

// 销毁线程池
int threadpool_destroy(threadpool_t *pool) {
    if (pool == NULL) {
        return -1;
    }
    
    // 获取队列锁
    pthread_mutex_lock(&(pool->queue_lock));
    
    // 设置关闭标志
    pool->shutdown = 1;
    
    // 唤醒所有工作线程
    pthread_cond_broadcast(&(pool->queue_not_empty));
    pthread_cond_broadcast(&(pool->queue_not_full));
    
    // 释放队列锁
    pthread_mutex_unlock(&(pool->queue_lock));
    
    // 等待所有工作线程结束
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    // 销毁同步原语
    pthread_mutex_destroy(&(pool->queue_lock));
    pthread_cond_destroy(&(pool->queue_not_empty));
    pthread_cond_destroy(&(pool->queue_not_full));
    
    // 释放内存
    free(pool);
    
    return 0;
}
```

## 9. 性能优化

### 9.1 减少锁竞争

1. **细粒度锁**：使用多个锁保护不同的资源部分
2. **无锁数据结构**：使用原子操作实现无锁数据结构
3. **读写分离**：使用读写锁，允许多读单写

### 9.2 减少开销

1. **线程池**：重用线程，避免频繁创建和销毁
2. **适当线程数**：线程数接近 CPU 核心数，避免过度线程化
3. **数据局部性**：尽量让一个线程处理连续的数据，改善缓存命中率

### 9.3 优化同步原语选择

| 同步原语 | 适用场景 |
|---------|----------|
| 互斥锁   | 一般保护，中等等待时间 |
| 自旋锁   | 短时间等待，高竞争环境 |
| 读写锁   | 读多写少的场景 |
| 原子操作 | 简单计数器或标志位 |
| 条件变量 | 线程间通知，长时间等待 |

## 10. 调试技巧

### 10.1 死锁检测

1. **使用工具**：Valgrind、Helgrind
2. **超时检测**：使用带超时的锁函数
3. **锁层次协议**：维护锁的层次结构，避免环形等待

### 10.2 竞态条件检测

1. **线程静态分析工具**：ThreadSanitizer
2. **锁记录**：记录获取和释放锁的时间和位置
3. **测试放大技术**：在关键点添加随机延迟

## 11. 不常用 API 简略介绍

### 11.1 线程属性相关

```c
pthread_attr_setguardsize()    // 设置线程栈末尾的保护区大小
pthread_attr_setstack()        // 设置线程使用的栈地址和大小
pthread_attr_setscope()        // 设置线程竞争范围（系统/进程）
```

### 11.2 互斥锁高级特性

```c
pthread_mutex_timedlock()      // 带超时的互斥锁获取
pthread_mutexattr_setprotocol()// 设置互斥量协议（避免优先级倒置）
pthread_mutexattr_setprioceiling() // 设置互斥量优先级天花板
```

### 11.3 高级同步

```c
pthread_barrierattr_init()     // 初始化屏障属性
pthread_spin_init()            // 初始化自旋锁
pthread_rwlockattr_init()      // 初始化读写锁属性
```

### 11.4 线程和信号

```c
pthread_kill()                 // 向指定线程发送信号
pthread_sigmask()              // 设置线程信号掩码
```

### 11.5 执行一次性初始化

```c
pthread_once()                 // 执行一次性初始化
```

### 11.6 CPU 亲和性

```c
pthread_setaffinity_np()       // 设置线程 CPU 亲和性
pthread_getaffinity_np()       // 获取线程 CPU 亲和性
```

### 11.7 线程名称

```c
pthread_setname_np()           // 设置线程名称（调试用）
pthread_getname_np()           // 获取线程名称
```

## 12. 最佳实践

1. **正确使用 pthread_join()/pthread_detach()**：避免资源泄漏
2. **总是检查 pthread 函数返回值**：及早发现错误
3. **正确配对锁的获取和释放**：使用代码块或函数范围
4. **条件变量总是与互斥锁一起使用**：防止竞态条件
5. **避免线程取消点上的资源泄漏**：使用 cleanup handlers
6. **优先使用线程安全函数**：如 strtok_r 替代 strtok
7. **最小化锁的作用域**：减少持有锁的时间，提高并发性
8. **避免 HTTP 浏览器进程时频繁创建和销毁线程**：使用线程池
9. **合理设置线程栈大小**：避免栈溢出或过度分配
10. **使用 valgrind/helgrind 等工具检测线程问题**

## 13. 完整示例：多线程文件搜索

此示例实现了一个多线程文件搜索程序，它接受一个目录和一个关键字，然后并行搜索目录中的所有文件：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH_LEN 1024
#define MAX_FILES 1000
#define NUM_THREADS 4

// 任务结构
typedef struct {
    char filepath[MAX_PATH_LEN];
    char *keyword;
} search_task_t;

// 共享数据
typedef struct {
    search_task_t tasks[MAX_FILES];
    int task_count;
    int next_task;
    int files_found;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int all_tasks_added;
} shared_data_t;

shared_data_t shared_data;

// 搜索文件中的关键词
void search_file(const char *filepath, const char *keyword) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        return;
    }
    
    char line[1024];
    int line_number = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        if (strstr(line, keyword)) {
            pthread_mutex_lock(&shared_data.mutex);
            printf("找到匹配: %s (行 %d): %s", filepath, line_number, line);
            shared_data.files_found++;
            pthread_mutex_unlock(&shared_data.mutex);
        }
    }
    
    fclose(file);
}

// 添加目录中的所有文件到任务队列
void add_directory_files(const char *dirname, const char *keyword) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char filepath[MAX_PATH_LEN];
    
    if ((dir = opendir(dirname)) == NULL) {
        perror("无法打开目录");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 "." 和 ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(filepath, MAX_PATH_LEN, "%s/%s", dirname, entry->d_name);
        
        if (stat(filepath, &statbuf) == -1) {
            continue;
        }
        
        if (S_ISDIR(statbuf.st_mode)) {
            // 递归添加子目录
            add_directory_files(filepath, keyword);
        } else if (S_ISREG(statbuf.st_mode)) {
            // 添加文件到任务队列
            pthread_mutex_lock(&shared_data.mutex);
            
            if (shared_data.task_count < MAX_FILES) {
                strcpy(shared_data.tasks[shared_data.task_count].filepath, filepath);
                shared_data.tasks[shared_data.task_count].keyword = strdup(keyword);
                shared_data.task_count++;
                
                // 唤醒等待的工作线程
                pthread_cond_signal(&shared_data.cond);
            }
            
            pthread_mutex_unlock(&shared_data.mutex);
        }
    }
    
    closedir(dir);
}

// 工作线程函数
void *worker(void *arg) {
    while (1) {
        pthread_mutex_lock(&shared_data.mutex);
        
        // 等待任务或所有任务完成
        while (shared_data.next_task >= shared_data.task_count && 
               !shared_data.all_tasks_added) {
            pthread_cond_wait(&shared_data.cond, &shared_data.mutex);
        }
        
        // 检查是否所有任务都完成
        if (shared_data.next_task >= shared_data.task_count && 
            shared_data.all_tasks_added) {
            pthread_mutex_unlock(&shared_data.mutex);
            break;
        }
        
        // 获取任务
        search_task_t task = shared_data.tasks[shared_data.next_task];
        shared_data.next_task++;
        
        pthread_mutex_unlock(&shared_data.mutex);
        
        // 执行搜索
        search_file(task.filepath, task.keyword);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("用法: %s <目录> <关键词>\n", argv[0]);
        return 1;
    }
    
    // 初始化共享数据
    shared_data.task_count = 0;
    shared_data.next_task = 0;
    shared_data.files_found = 0;
    shared_data.all_tasks_added = 0;
    pthread_mutex_init(&shared_data.mutex, NULL);
    pthread_cond_init(&shared_data.cond, NULL);
    
    // 添加目录文件到任务队列
    add_directory_files(argv[1], argv[2]);
    
    // 标记所有任务都已添加
    pthread_mutex_lock(&shared_data.mutex);
    shared_data.all_tasks_added = 1;
    pthread_cond_broadcast(&shared_data.cond);
    pthread_mutex_unlock(&shared_data.mutex);
    
    printf("添加了 %d 个文件到搜索队列\n", shared_data.task_count);
    
    // 创建工作线程
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, worker, NULL);
    }
    
    // 等待所有线程完成
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // 打印结果
    printf("搜索完成，在 %d 个文件中找到匹配\n", shared_data.files_found);
    
    // 清理
    pthread_mutex_destroy(&shared_data.mutex);
    pthread_cond_destroy(&shared_data.cond);
    
    // 释放关键词内存
    for (int i = 0; i < shared_data.task_count; i++) {
        free(shared_data.tasks[i].keyword);
    }
    
    return 0;
}
```

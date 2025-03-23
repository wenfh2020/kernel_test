#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 4

// 线程函数
void *thread_func(void *arg) {
    int id = (int)(long)arg;  // 将 void* 转换为 int
    printf("Thread %d started\n", id);

    // 模拟 CPU 密集型任务
    int i;
    for (i = 0; i < 100000000; i++) {
        // 空循环，消耗 CPU 时间
        printf("Thread %d......\n", id);
    }

    printf("Thread %d finished\n", id);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int i;

    // 创建多个线程
    for (i = 0; i < NUM_THREADS; i++) {
        int ret = pthread_create(&threads[i], NULL, thread_func, (void *)(long)i);
        if (ret != 0) {
            fprintf(stderr, "Error creating thread %d: %d\n", i, ret);
            exit(EXIT_FAILURE);
        }
    }

    // 等待所有线程完成
    for (i = 0; i < NUM_THREADS; i++) {
        int ret = pthread_join(threads[i], NULL);
        if (ret != 0) {
            fprintf(stderr, "Error joining thread %d: %d\n", i, ret);
            exit(EXIT_FAILURE);
        }
    }

    printf("All threads finished\n");
    return 0;
}
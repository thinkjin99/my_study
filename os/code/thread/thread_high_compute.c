#include <stdio.h>
#include <pthread.h>
#include <time.h>

#define SIZE 1000000
#define NUM_THREADS 8

// 데이터 구조체
typedef struct
{
    int *array;
    int start;
    int end;
} ThreadData;

// 스레드 함수
void *squareElements(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    for (int i = data->start; i < data->end; ++i)
    {
        data->array[i] *= data->array[i];
    }

    return NULL;
}

int main()
{
    int dataArray[SIZE];

    // 배열 초기화
    for (int i = 0; i < SIZE; ++i)
    {
        dataArray[i] = i + 1;
    }

    // 싱글 스레드 버전
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < SIZE; ++i)
    {
        dataArray[i] *= dataArray[i];
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Single-threaded execution time: %f seconds\n",
           ((double)end.tv_sec - start.tv_sec) +
               ((double)end.tv_nsec - start.tv_nsec) / 1e9);

    // 멀티 스레드 버전
    pthread_t threads[NUM_THREADS];
    ThreadData threadData[NUM_THREADS];

    struct timespec start_mt, end_mt;
    clock_gettime(CLOCK_MONOTONIC, &start_mt);

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threadData[i].array = dataArray;
        threadData[i].start = i * (SIZE / NUM_THREADS);
        threadData[i].end = (i + 1) * (SIZE / NUM_THREADS);

        pthread_create(&threads[i], NULL, squareElements, (void *)&threadData[i]);
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_mt);
    printf("Multi-threaded execution time: %f seconds\n",
           ((double)end_mt.tv_sec - start_mt.tv_sec) +
               ((double)end_mt.tv_nsec - start_mt.tv_nsec) / 1e9);

    return 0;
}

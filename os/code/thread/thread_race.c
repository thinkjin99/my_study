#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *runner(void *param)
{
    int *tid = (int *)param;
    for (int i = 1; i <= 1000; i++)
    {
        printf("%d is running...\n", *tid);
        usleep(1);
    }
    pthread_exit(0);
}

int main()
{
    pthread_t tid, tid2;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, runner, &tid);
    pthread_create(&tid2, &attr, runner, &tid2);

    pthread_join(tid, NULL);
    pthread_join(tid2, NULL);
}
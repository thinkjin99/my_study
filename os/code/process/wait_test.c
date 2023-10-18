#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>

int main()
{
    int pid;
    int *test = (int *)malloc(sizeof(int) * 3);
    memset(test, 0, 3);
    pid = fork();
    if (pid == 0)
    {
        printf("Child %d is start.\n", getpid());
        sleep(1);
        printf("Child %d ended. \n", getpid());
    }
    else
    {
        printf("Parent %d is start.\n", getpid());
        printf("Parent %d ended. \n", getpid());
        // wait(NULL);
    }
    return 0;
}
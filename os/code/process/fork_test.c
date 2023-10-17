#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

int main()
{
    int pid;
    int *test = (int *)malloc(sizeof(int) * 3);
    memset(test, 0, 3);
    pid = fork();
    if (pid == 0)
    {
        test[0]++;
        printf("Child %d\n", test[0]);
        printf("Child end %d\n", getpid());
    }
    else
    {
        test[0]++;
        printf("Parent %d \n", test[0]);
        printf("Parent end %d\n", getpid());
        wait(NULL);
    }
    return 0;
}
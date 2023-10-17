#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    pid_t childPid;
    childPid = fork();
    if (childPid > 0)
    {
        printf("Parent is running\n");
        sleep(20);
    }
    else if (childPid == 0)
    {
        printf("Child is running\n");
        printf("Child is End\n");
    }
    return 0;
}
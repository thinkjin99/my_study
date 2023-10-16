#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int pid;
    int status;
    printf("Hello, my pid is %d", getpid()); // \n을 추가해 버퍼를 비우자

    pid = fork();
    if (pid == 0)
    {
        printf("\nI was forked! :D");
        sleep(1);
    }
    else
    {
        wait(&status);
        printf("\n%d was forked!", pid);
    }
    return 0;
}
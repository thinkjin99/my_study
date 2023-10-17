#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{

    pid_t childPid;
    int i;

    childPid = fork();

    if (childPid > 0)
    { // Parent 프로세스
        printf("Parent PID : %ld, pid : %d\n", (long)getpid(), childPid);
        sleep(2);
        printf("Parent 종료\n");
        exit(0);
    }
    else if (childPid == 0)
    { // Child 코드
        printf("Child 시작\n");

        for (i = 0; i < 10; i++)
        {
            printf("Child PID : %ld Parent PID : %ld\n", (long)getpid(), (long)getppid());
            sleep(1);
        }

        printf("Child 종료\n");
        exit(0);
    }
    else
    { // fork 실패
        perror("fork fail! \n");
        return -1;
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int foo(){
    int a = 3;
    
}
int main()
{
    int pid;
    int *test = (int *)malloc(sizeof(int) * 3);
    test[0] = 0;
    pid = fork();
    if (pid == 0)
    {
        test[0] += 10;
        printf("Child %d\n", test[0]);
        printf("Child end %d\n", getpid());
    }
    else
    {
        test[0] += 10;
        printf("Parent %d \n", test[0]);
        printf("Parent end %d\n", getpid());
    }
    return 0;
}
#include <unistd.h> //execl 때문에 사용한 라이브러리
#include <stdio.h>  //printf 때문에 사용한 라이브러리

int main()
{
    printf("execute is\n");
    execl("/bin/ls", "ls", "-al", NULL);
    printf("code still alive..?");
    perror("execl is failed\n"); // 에러 코드 출력
    return 0;
}
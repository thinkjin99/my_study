#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int budget1 = 100000;
int budget2 = 100000;

void *send_money(void *param)
{
    printf("송금 시작\n");
    int *money = (int *)param;
    budget1 -= *money / 2;
    printf("budget 1: 출금중: %d\n", budget1); //10만원에서 절반 정도 출금 작업이 진행
    usleep(100);
    budget1 -= *money / 2;
    printf("budget 1: 출금: %d\n", budget1);
    budget2 += *money; // 2번 계좌로 전송
    printf("budget 2: 입금: %d\n", budget2);
    printf("송금 완료\n");
    pthread_exit(0);
}

void *get_money(void *param)
{
    printf("출금 시작\n");
    int *money = (int *)param;
    budget1 -= *money;
    printf("budget 1: 출금: %d\n", budget1);
    printf("출금 완료\n");
    pthread_exit(0);
}

int main()
{
    int money = 100000;
    pthread_t tid, tid2;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, send_money, &money);
    pthread_create(&tid2, &attr, get_money, &money);

    pthread_join(tid, NULL);
    pthread_join(tid2, NULL);
}
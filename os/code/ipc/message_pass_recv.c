#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// 메시지 구조체 정의
struct msg_buffer
{
    long msg_type;
    char message[100];
};

int main()
{
    // 메시지 큐 키 생성
    key_t key = ftok("progfile", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    struct msg_buffer message;

    // 메시지 큐에서 메시지 수신
    if (msgrcv(msgid, &message, sizeof(message), 1, 0) != -1)
        printf("메시지 수신: %s\n", message.message);
    return 0;
}
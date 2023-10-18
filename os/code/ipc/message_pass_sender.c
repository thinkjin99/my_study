#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    message.msg_type = 1;
    strcpy(message.message, "안녕, 메시지 큐!");

    // 메시지 큐에 메시지 전송

    msgsnd(msgid, &message, sizeof(message), 0);
    printf("메시지 전송: %s\n", message.message);

    return 0;
}

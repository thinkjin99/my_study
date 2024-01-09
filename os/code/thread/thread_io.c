#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>

#define NUM_THREADS 5
#define MAX_BUFFER_SIZE 1024

// 구조체 정의
typedef struct
{
    int thread_id;
    char *hostname;
} ThreadData;

// 스레드 함수
void *sendHttpRequest(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    // 소켓 생성
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        pthread_exit(NULL);
    }

    // 호스트 정보 얻기
    struct hostent *server = gethostbyname(data->hostname); // domain -> ip
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        close(sockfd);
        pthread_exit(NULL);
    }

    // 서버 주소 구조체 초기화
    struct sockaddr_in server_addr;
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length); // 획득한 ip 주소 addr객체에 복사
    server_addr.sin_port = htons(80);

    // 서버에 연결
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting to server");
        close(sockfd);
        pthread_exit(NULL);
    }

    // HTTP GET 요청 메시지 생성
    char request_message[MAX_BUFFER_SIZE];
    snprintf(request_message, sizeof(request_message),
             "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", data->hostname);

    // 서버로 요청 전송
    if (write(sockfd, request_message, strlen(request_message)) < 0)
    {
        perror("Error writing to socket");
        close(sockfd);
        pthread_exit(NULL);
    }

    // 서버로부터 응답 수신 및 출력
    char response_buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = read(sockfd, response_buffer, sizeof(response_buffer) - 1)) > 0)
    {
        response_buffer[bytes_received] = '\0';
        printf("Thread %d - Received response:\n%s\n", data->thread_id, response_buffer);
    }

    // 소켓 닫기
    close(sockfd);

    pthread_exit(NULL);
}

int main()
{
    const char *hostname = "www.naver.com";
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

    // 스레드 생성 및 실행
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        thread_data[i].thread_id = i;
        thread_data[i].hostname = (char *)hostname;
        if (pthread_create(&threads[i], NULL, sendHttpRequest, (void *)&thread_data[i]) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }

    // 모든 스레드의 종료를 기다림
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

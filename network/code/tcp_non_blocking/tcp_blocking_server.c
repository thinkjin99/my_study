#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define PORT 8000
#define MAX_MESSAGE_LEN 128
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int clients[MAX_CLIENTS] = {
    0,
};

int client_cnt = 0;

struct listen_socket
{
    int fd;
};

void *accept_connection(void *args)
{
    // pthread_mutex_lock(&mutex);

    struct listen_socket *sc = (struct listen_socket *)args;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    int listen_fd = sc->fd;
    int new_fd = accept(listen_fd, (struct sockaddr *)&client_address, &client_address_len);
    printf("New connection accepted\n");
    if (new_fd < 0)
    {
        perror("Connection failed");
        exit(0);
    }
    else
    {
        clients[client_cnt++] = new_fd;
    }
    // pthread_mutex_unlock(&mutex);
    return NULL;
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);
    int num_clients = 0;

    // 서버 소켓 생성
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        printf("setsockopt(SO_REUSEADDR) failed");

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // 서버 바인딩
    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));

    // 서버 리스닝
    listen(server_socket, 5);
    printf("Server listening on port %d...\n", PORT); // 서버 설정
    char buffer[MAX_MESSAGE_LEN] = {
        0,
    };

    while (1)
    {
        // client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len); // 클라이언트 커넥션 할당
        pthread_t tid;
        struct listen_socket sc;
        sc.fd = server_socket;
        if (pthread_create(&tid, NULL, accept_connection, &sc) != 0)
        {
            perror("Thread not created");
            exit(0);
        };

        sleep(1);
        pthread_cancel(tid);
        pthread_join(tid, NULL);

        for (int i = 0; i < client_cnt; i++)
        {
            memset(buffer, 0, MAX_MESSAGE_LEN);
            // select를 추가
            read(clients[i], buffer, MAX_MESSAGE_LEN);
            printf("Client:%d says %s\n", clients[i], buffer);
            write(clients[i], buffer, MAX_MESSAGE_LEN);
        }
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define BUF_SIZE 1024

// Define client structure
typedef struct
{
    int sockfd;
    char buffer[BUF_SIZE];
} Client;

// Initialize client
void initClient(Client *client, int sockfd)
{
    client->sockfd = sockfd;
    memset(client->buffer, 0, sizeof(client->buffer));
}

// Receive data from a client
void doEcho(Client *client)
{
    ssize_t bytesRead = recv(client->sockfd, client->buffer, sizeof(client->buffer), 0);

    if (bytesRead > 0)
    {
        client->buffer[bytesRead] = '\0';
        printf("Received data from client %d: %s\n", client->sockfd, client->buffer);
        send(client->sockfd, client->buffer, bytesRead, 0);
    }

    else if (bytesRead == 0)
    {
        printf("Connection closed by client %d\n", client->sockfd);
        close(client->sockfd);
    }
    else
    {
        perror("Error in recv");
        close(client->sockfd);
    }
}

int main()
{
    int serverSockfd, clientSockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int epollfd;
    struct epoll_event events[MAX_EVENTS];

    // Create socket
    serverSockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Bind socket to an address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(12345);
    bind(serverSockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    // Listen for incoming connections
    listen(serverSockfd, SOMAXCONN);

    // Create epoll instance
    epollfd = epoll_create1(0);
    struct epoll_event event;
    event.events = EPOLLIN;                                  // 해당 fd에 읽을 수 있는 데이터가 존재할 때 발생하는 이벤트
    event.data.fd = serverSockfd;                            // 리스닝 소켓 등록
    epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSockfd, &event); // 인스턴스의 구독 리스트에 추가

    printf("Server is listening on port 12345\n");

    // Client array to store connected clients
    Client clients[MAX_EVENTS];

    while (1)
    {
        int numEvents = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < numEvents; ++i)
        {
            if (events[i].data.fd == serverSockfd)
            {
                // Accept a new connection
                clientSockfd = accept(serverSockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
                printf("Accepted a new connection from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

                // Initialize and add the new client to epoll
                initClient(&clients[clientSockfd], clientSockfd);
                event.events = EPOLLIN;
                event.data.fd = clientSockfd;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientSockfd, &event);
            }
            else
            {
                // Receive data from an existing client
                doEcho(&clients[events[i].data.fd]);
            }
        }
    }

    // Close sockets and epoll (this part is unreachable in this example)
    close(serverSockfd);
    close(epollfd);

    return 0;
}

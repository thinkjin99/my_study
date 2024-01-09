#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define HOST "www.naver.com"
#define PORT 80
#define HTTP_MSG "GET / HTTP/1.1\r\nHost: www.naver.com\r\n\r\n"

void sync_select()
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    send(client_socket, HTTP_MSG, strlen(HTTP_MSG), 0);

    char buffer[32];
    ssize_t bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    close(client_socket);
}

int main()
{
    sync_select();
    return 0;
}

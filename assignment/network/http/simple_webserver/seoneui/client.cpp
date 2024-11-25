#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;

const int BUFFER_SIZE = 1024;
int PORT = 8080;

int main()
{
    int clientSockcet;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], server_reply[BUFFER_SIZE];
    char buffer[1024];

    clientSockcet = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSockcet < 0)
    {
        cout << "소켓 생성 실패" << endl;
    }

    cout << "소켓 생성 성공" << endl;

    // 서버 주소 설정
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(clientSockcet, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        cout << "연결 실패" << endl;
        exit(1);
    }
    while (1)
    {
        strcpy(message, "GET /index.html HTTP/1.1\r\n\r\n");

        if (send(clientSockcet, message, strlen(message), 0) < 0)
        {
            cout << "전송 실패" << endl;
            exit(1);
        }

        read(clientSockcet, buffer, 1024);
        cout << buffer << endl;
    }
    return 0;
}
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

const char* SERVER_IP = "127.0.0.1";
const int PORT = 8080;
const int BUFFER_SIZE = 1024;

using namespace std;

int main() {
    // 소켓 생성
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1) {
        cerr << "소켓 생성에 실패했습니다." << endl;
        return 1;
    }

    // 서버 주소 설정
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT); // 서버의 포트 번호
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP); // 서버의 IP 주소

    while (true) {
        // 사용자로부터 메시지 입력
        cout << "전송할 메시지 입력 (또는 'bye' 입력하여 종료): ";
        string message;
        getline(cin, message);

        // 서버에 메시지 전송
        ssize_t bytesSent = sendto(clientSocket, message.c_str(), message.size(), 0,
                                   (struct sockaddr *)&serverAddress, sizeof(serverAddress));

        if (bytesSent == -1) {
            std::cerr << "데이터 전송에 실패했습니다." << std::endl;
            break;
        }

        // 서버로부터 메시지 수신
        char buffer[BUFFER_SIZE];
        sockaddr_in serverResponseAddress;
        socklen_t serverResponseAddressLength = sizeof(serverResponseAddress);

        ssize_t bytesRead = recvfrom(clientSocket, buffer, sizeof(buffer), 0,
                                     (struct sockaddr *)&serverResponseAddress, &serverResponseAddressLength);

        if (bytesRead == -1) {
            cerr << "데이터 수신에 실패했습니다." << endl;
            break;
        }

        buffer[bytesRead] = '\0';

        cout << "서버에서의 응답: " << buffer << endl;

        // 'bye'를 입력하면 클라이언트 종료
        if (message == "bye") {
            break;
        }
    }

    // 소켓 종료
    close(clientSocket);

    return 0;
}

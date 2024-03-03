#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

using namespace std;

int main() {
    // 소켓 생성
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1) {
        std::cerr << "소켓 생성에 실패했습니다." << std::endl;
        return 1;
    }

    // 서버 주소 설정
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT); // 포트 번호 설정
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // 소켓 바인딩
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        cerr << "바인딩에 실패했습니다." << endl;
        close(serverSocket);
        return 1;
    }

    cout << "UDP 에코 서버" << endl;

    while (true) {
        // 클라이언트로부터 메시지 수신
        char buffer[BUFFER_SIZE];
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        ssize_t bytesRead = recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                                     (struct sockaddr *)&clientAddress, &clientAddressLength);

        if (bytesRead == -1) {
            cerr << "데이터 수신에 실패했습니다." << endl;
            break;
        }

        buffer[bytesRead] = '\0';

        cout << "클라이언트 : " << buffer << endl;

        // 클라이언트에게 메시지 전송
        ssize_t bytesSent = sendto(serverSocket, buffer, bytesRead, 0,
                                   (struct sockaddr *)&clientAddress, sizeof(clientAddress));

        if (bytesSent == -1) {
            cerr << "데이터 전송에 실패했습니다." << endl;
            break;
        }

        // 클라이언트가 "bye"를 입력했을 때 종료
        if (strcmp(buffer, "bye") == 0) {
            // 서버에서 클라이언트로 "bye" 전송
            ssize_t bytesSent = sendto(serverSocket, buffer, bytesRead, 0,
                           (struct sockaddr *)&clientAddress, sizeof(clientAddress));
            close(serverSocket);
            break;
        }
    }

    // 소켓 종료
    close(serverSocket);

    return 0;
}

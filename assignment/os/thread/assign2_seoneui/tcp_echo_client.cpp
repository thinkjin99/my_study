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
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "소켓 생성에 실패했습니다." << std::endl;
        return 1;
    }

    // 서버 주소 설정
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT); // 서버의 포트 번호
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP); // 서버의 IP 주소

    // 서버에 연결
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        cerr << "서버에 연결할 수 없습니다." << endl;
        close(clientSocket);
        return 1;
    }

    cout << "서버에 연결되었습니다." << endl;

    // 통신 루프
    while (true) {
        // 사용자로부터 메시지 입력
        cout << "보내기(또는 'bye' 입력하여 종료): ";
        string message;
        getline(cin, message);

        // 서버에 메시지 전송
        send(clientSocket, message.c_str(), message.size(), 0);

        // 서버로부터 메시지 수신
        char buffer[BUFFER_SIZE];
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            cerr << "데이터 수신에 실패했습니다." << endl;
            break;
        }

        buffer[bytesRead] = '\0';
        cout << "서버: " << buffer << endl;

        // 사용자가 'bye'를 입력하면 루프 종료
        if (message == "bye") {
            break;
        }
    }

    // 소켓 종료
    close(clientSocket);

    return 0;
}

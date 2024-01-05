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
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "소켓 생성에 실패했습니다." << endl;
        return 1;
    }

    // 서버 주소 설정
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT); // 서버의 포트 번호
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // 소켓 바인딩
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        cerr << "바인딩에 실패했습니다." << endl;
        close(serverSocket);
        return 1;
    }

    // 소켓 리스닝
    if (listen(serverSocket, 5) == -1) {
        cerr << "리스닝에 실패했습니다." << endl;
        close(serverSocket);
        return 1;
    }

    cout << "연결 대기중..." << endl;

    while (true) {
        // 클라이언트 연결 수락
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);

        if (clientSocket == -1) {
            cerr << "연결 수락에 실패했습니다." << endl;
            close(serverSocket);
            return 1;
        }

        cout << "연결되었습니다." << endl;

        // 통신 루프
        while (true) {
            // 클라이언트로부터 메시지 수신
            char buffer[BUFFER_SIZE];
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

            if (bytesRead <= 0) {
                cerr << "데이터 수신에 실패했습니다." << endl;
                break;
            }

            buffer[bytesRead] = '\0';

            cout << "클라이언트: " << buffer << endl;

            // 클라이언트가 "bye"를 입력했을 때 종료
            if (strcmp(buffer, "bye") == 0) {
                // 서버에서 클라이언트로 "bye" 전송
                send(clientSocket, "bye", 3, 0);
                close(clientSocket);
                break;  // 루프를 종료하고 다음 클라이언트 연결을 대기
            }

            // 클라이언트에 메시지 전송
            send(clientSocket, buffer, bytesRead, 0);
        }

        cout << "연결 대기중입니다..." << endl;
    }

    // 소켓 종료 (실행되지 않음)
    close(serverSocket);

    return 0;
}

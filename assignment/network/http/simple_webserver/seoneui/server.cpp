#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <map>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

#include <sys/event.h>
#include <sys/types.h>

#include <cstdlib>
#include <ctime>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;
const int MAX_EVENT = 1000;

#define HEADER_FMT "HTTP/1.1 %d %s\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n"

using namespace std;

// http 해더 형식 지정 함수
void fill_header(char *header, int status, long len, char *type)
{
    char status_text[40]; // http응답 상태 코드에 해당하는 텍스트 저장
    switch (status)
    {
    case 200:
        strcpy(status_text, "OK"); // status_text에 적절한 텍스트 복사
        break;
    case 404:
        strcpy(status_text, "NOT FOUND");
        break;
    case 500:
    default:
        strcpy(status_text, "Internet Server Error");
        break;
    }
    sprintf(header, HEADER_FMT, status, status_text, len, type); // header문자열에 http응답 헤더를 형식화해서 삽입
}

// 404 에러 처리
void handle_404(int acceptSocket, char *type)
{
    char header[BUFFER_SIZE];
    char body[BUFFER_SIZE];

    sprintf(body, "<html><body><h1>404 Not Found</h1></body></html>");
    fill_header(header, 404, strlen(body), type);
    write(acceptSocket, header, strlen(header));
    write(acceptSocket, body, strlen(body));
}

// 500 에러 처리
void handle_500(int acceptSocket, char *type)
{
    char header[BUFFER_SIZE];
    char body[BUFFER_SIZE];

    sprintf(body, "<html><body><h1>500 Internal Server Error</h1></body></html>");
    fill_header(header, 500, strlen(body), type);
    write(acceptSocket, header, strlen(header));
    write(acceptSocket, body, strlen(body));
}

// uri에서 MIME 타입을 찾는 함수
void find_mime(char *ct_type, char *uri)
{
    char *ext = strrchr(uri, '.');
    if (!strcmp(ext, ".html"))
    {
        strcpy(ct_type, "text/html");
    }
}

// 클라이언트로부터 받은 http 요청을 처리하고 응답을 보냄
void httpHandler(int acceptSocket)
{
    int waitTime = rand() % 3000000 + 1000000; // 1,000,000에서 3,000,000 마이크로초(1~3초) 사이의 랜덤 값
    usleep(waitTime);                          // 랜덤 시간만큼 대기

    char buf[BUFFER_SIZE];
    char response[BUFFER_SIZE * 2]; // 충분한 크기를 가지도록 조정
    char safe_uri[BUFFER_SIZE];
    struct stat st;

    // 요청 읽기
    read(acceptSocket, buf, BUFFER_SIZE - 1);

    cout << buf << endl;

    // HTTP 메서드와 URI 파싱
    char *method = strtok(buf, " ");
    char *uri = strtok(NULL, " ");

    // 기본 URI 설정
    strcpy(safe_uri, uri);
    if (!strcmp(safe_uri, "/"))
    {
        strcpy(safe_uri, "/index.html");
    }

    char *local_uri = safe_uri + 1; // 경로 앞의 '/' 제거

    // MIME 타입 찾기 및 헤더 채우기
    char ct_type[40] = "text/plain"; // 기본 MIME 타입
    find_mime(ct_type, local_uri);   // MIME 타입 찾기

    // GET 메서드만 처리
    if (strcmp(method, "GET") != 0)
    {
        handle_500(acceptSocket, ct_type);
        return;
    }

    if (stat(local_uri, &st) < 0)
    {
        handle_404(acceptSocket, ct_type);
        return;
    }

    // 파일 열기
    int fd = open(local_uri, O_RDONLY);
    if (fd < 0)
    {
        handle_500(acceptSocket, ct_type);
        return;
    }

    // 응답 헤더 생성
    char header[BUFFER_SIZE];
    fill_header(header, 200, st.st_size, ct_type);

    // 헤더와 파일 내용을 response 버퍼에 복사
    strcpy(response, header);
    int header_len = strlen(header);
    int cnt = read(fd, response + header_len, BUFFER_SIZE - header_len - 1);
    if (cnt < 0)
    {
        // 파일 읽기 실패
        handle_500(acceptSocket, ct_type);
        close(fd);
        return;
    }

    // 클라이언트에 응답 전송
    write(acceptSocket, response, header_len + cnt);

    close(fd);
}

int main()
{
    srand(time(NULL));

    struct sockaddr_in remote_sin;
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // 소켓 생성
    int kq;

    // 소켓에 에러가 발생한 경우 처리
    if (serverSocket < 0)
    {
        cout << "소켓 생성 실패" << endl;
        exit(1);
    }

    // 서버 주소 설정
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // 소켓에 ip주소와 포트 번호 바인딩
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    { // 만약 -1일 경우(실패)
        cout << "바인딩 실패" << endl;
        close(serverSocket);
        exit(1);
    }

    cout << "바인드 성공" << endl;

    // 소켓을 listen 상태로 전환함
    if (listen(serverSocket, 100) < 0)
    { //-1일 경우(실패)
        cout << "소켓 리스닝 실패" << endl;
        exit(1);
    }

    cout << "소켓 리스닝 성공" << endl;

    kq = kqueue();
    struct kevent changeEvent;
    struct kevent eventList[MAX_EVENT]; // 이벤트 목록

    // 서버 소켓에 대한 이벤트 등록
    EV_SET(&changeEvent, serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kq, &changeEvent, 1, NULL, 0, NULL) == -1) {
        cout << "kevent 등록 실패" << endl;
        exit(1);
    }

    while (true) {
        int newEvents = kevent(kq, NULL, 0, eventList, MAX_EVENT, NULL); // 발생한 이벤트를 대기
        for (int i = 0; i < newEvents; i++) {
            if (eventList[i].ident == (uintptr_t)serverSocket) {
                // 새로운 연결 수락
                sockaddr_in clientAddress;
                socklen_t clientAddressLength = sizeof(clientAddress);
                int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);
                if (clientSocket < 0) 
                    continue; // 연결 수락 실패

                // 클라이언트 소켓에 대한 읽기 이벤트 등록
                EV_SET(&changeEvent, clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                if (kevent(kq, &changeEvent, 1, NULL, 0, NULL) == -1) {
                    cout << "클라이언트 소켓 kevent 등록 실패" << endl;
                    close(clientSocket);
                }
            } else if (eventList[i].filter == EVFILT_READ) {
                // 클라이언트로부터 데이터 읽기
                httpHandler(eventList[i].ident);
                
            }
        }
    }
}

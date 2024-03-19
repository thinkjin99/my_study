#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

//int PORT = 8080;
const int BUFFER_SIZE = 1024;

#define HEADER_FMT "HTTP/1.1 %d %s\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n"
#define NOT_FOUND_CONTENT "404 Not Found"
#define SERVER_ERROR_CONTENT "500 Internal Server Error"

using namespace std;

void httpHandler(int acceptSocket);
void fill_header(char *header, int status, long len, char *type);
void find_mime(char *ct_type, char *uri);
void handle_404(int acceptSocket);
void handle_500(int acceptSocket);

int main(int argc, char **argv){
    struct sockaddr_in remote_sin;
    int PORT, pid;

    if(argc < 2){
        cout<<"Usage"<<PORT<<argv[0]<<endl;
        exit(0);
    }

    PORT = atoi(argv[1]);


    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); //소켓 생성

    //소켓에 에러가 발생한 경우 처리
    if(serverSocket < 0){
        cout << "소켓 생성 실패"<<endl;
        exit(1);
    }

    //서버 주소 설정
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    //소켓에 ip주소와 포트 번호 바인딩
    if(bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1){ //만약 -1일 경우(실패)
        cout << "바인딩 실패"<<endl; 
        close(serverSocket);
        exit(1);
    }

    cout<<"바인드 성공"<<endl;

    //소켓을 listen 상태로 전환함 
    if(listen(serverSocket, 100) < 0){ //-1일 경우(실패)
        cout << "소켓 리스닝 실패"<<endl;
        exit(1);
    }

    cout << "소켓 리스닝 성공" <<endl;

    while(1){
        //클라이언트 주소 설정
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        //acceptSocket : 클라이언트와 통신할 때 사용되는 소켓, (struct sockaddr *)&clientAddress : 클라이언트 주소 정보를 저장할 변수 포인터, &clientAddressLength : 클라이언트 주소 정보 크기를 나타내는 변수 포인터
        int acceptSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength); 
        //클라이언트가 연결 요청을 하면 그 연결 요청을 수락하고 acceptSocket을 생성

        if(acceptSocket == -1){
            cout << "연결 수락 실패"<<endl;
            continue;
        }

        httpHandler(acceptSocket); //클라이언트 요청 처리
        close(acceptSocket); //연결 종료
    

    }
}

//클라이언트로부터 받은 http 요청을 처리하고 응답을 보냄
void httpHandler(int acceptSocket){
    char header[BUFFER_SIZE];
    char buf[BUFFER_SIZE]; 
    char safe_uri[BUFFER_SIZE];
    char *local_uri;
    struct stat st;

    //소켓으로부터 http요청 읽음, 읽은 요청을 buf에 저장
    if(read(acceptSocket, buf, BUFFER_SIZE)<0){ //만약 읽기가 실패하면(-1을 반환하면) handle_500 함수 호출하여 오류 응답 전달
        cout<<"요청 읽기 실패";
        handle_500(acceptSocket);
        return;
    }

    cout <<buf<<endl; //버퍼에 읽어드린 내용 출력

    //읽은 http 요청 
    char *method= strtok(buf," ");
    char *uri = strtok(NULL, " ");

    //http 요청을 파싱하여 요청된 uri 추출
    strcpy(safe_uri, uri);
    if(!strcmp(safe_uri, "/")){ //요청된 uridl "/"인 경우
        strcpy(safe_uri, "/index.html"); //기본페이지이므로 "/index.html"로 변경
    }

    local_uri = safe_uri +1;
    if(stat(local_uri, &st) < 0){ //요청 리소스를 찾기 위해 stat 함수 사용
        handle_404(acceptSocket);//요청된 리소스가 없다면 404 응답 전송
        return;
    }

    int fd = open(local_uri, O_RDONLY);//요청된 리소스가 존재하면 해당 파일을 열어 디스크립터를 얻음
    if(fd <0){
        handle_500(acceptSocket);//파일을 열지 못하면 500 내부 서버 응답 전송
        return;
    }

    int ct_len = st.st_size; //파일의 크기를 얻어 콘텐츠 길이로 사용
    char ct_type[40];
    find_mime(ct_type, local_uri); //요청된 파일의 MIME 타입을 찾기 위한 함수 호출
    fill_header(header, 200, ct_len, ct_type);//응답 해더를 생성하기 위한 함수 호출, 이 함수는 200 OK와 함께 콘텐츠 길이 및 MIME타입을 포함한 http 응답 헤더 생성
    write(acceptSocket, header, strlen(header));//생성된 http웅던 헤더를 클라이언트에게 전송

    int cnt;
    while((cnt = read(fd, buf, BUFFER_SIZE)) > 0){ //파일에서 데이터를 읽어와서 클라이언트에게 전송하는데, 파일의 내용을 버퍼 사이즈만큼 읽고 읽은 데이터를 클라이언트에게 전송
        write(acceptSocket, buf, cnt);
    }
    
}

//http 해더 형식 지정 함수
void fill_header(char *header, int status, long len, char *type){
    char status_text[40]; //http응답 상태 코드에 해당하는 텍스트 저장
    switch(status){
        case 200:
            strcpy(status_text,"OK"); //status_text에 적절한 텍스트 복사
            break;
        case 404:
            strcpy(status_text,"NOT FOUND");
            break;
        case 500:
        default :
            strcpy(status_text,"Internet Server Error");
            break;
    }
    sprintf(header, HEADER_FMT, status, status_text, len, type); //header문자열에 http응답 헤더를 형식화해서 삽입
}

//uri에서 MIME 타입을 찾는 함수
void find_mime(char *ct_type, char *uri){
    char *ext = strrchr(uri, '.');
    if(!strcmp(ext, ".html")){
        strcpy(ct_type, "text/html");
    }
}

//404 에러 처리
void handle_404(int acceptSocket){
    char header[BUFFER_SIZE];
    fill_header(header, 404, sizeof(NOT_FOUND_CONTENT), "text/html");
    write(acceptSocket, header, strlen(header));
}

//500 에러 처리
void handle_500(int acceptSocket){
    char header[BUFFER_SIZE];
    fill_header(header, 500, sizeof(SERVER_ERROR_CONTENT), "text/html");
    write(acceptSocket, header, strlen(header));
}
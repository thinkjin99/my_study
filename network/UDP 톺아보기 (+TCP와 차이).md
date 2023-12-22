### 출처
* https://www.quora.com/What-is-the-reason-behind-Youtube-using-TCP-and-not-UDP  (왜 유튜브는 tcp를 쓸까?)
* https://stackoverflow.com/questions/21266008/can-i-use-broadcast-or-multicast-for-tcp (브로드캐스트는 왜 udp만?)
* https://www.cloudflare.com/ko-kr/learning/ddos/glossary/user-datagram-protocol-udp/ (udp의 기초)
* https://www.cloudflare.com/ko-kr/learning/ddos/udp-flood-ddos-attack/ (udp 폭주)
* https://evan-moon.github.io/2019/10/08/what-is-http3/ (http3.0)
____
### 개요
* [[#UDP란]]
* [[#유튜브는 UDP를 쓸까?]]
* [[#UDP와 DDOS]]
* [[#UDP 브로드캐스트]]
* [[#간단한 UDP 에코 서버]]
* [[#TCP, UDP 코드로 비교 해보기]]
* [[#UDP 소켓 통신 방식]]
* [[#UDP 세그먼트]]
* [[#UDP의 미래]]
___
### UDP란

UDP는 사용자 데이터그램 프로토콜의 줄임말로써 전송 계층에서 사용하는 프로토콜이다. UDP는 TCP와 대척점에 놓인 프로토콜로 TCP와 달리 패킷의 전달을 보장하지 않는다. 이로 인해 ==**UDP는 빠른 전송을 실시할 수 있지만, 패킷의 손실이 발생할 수 있다.**==

따라서 <b><u>UDP는 패킷 손실이 발생하더라도 속도가 더 중요한 서비스 혹은 손실된 패킷을 곧장 복구 할 수 있는 서비스에서 활용한다. </u></b> 전자의 예시로는 실시간 스트리밍 서비스나, 온라인 게임 등이 있는데 이 경우 과거의 패킷의 정보를 현재의 패킷이 어차피 덮어 사용하기 때문에 과거의 패킷에 손실이 발생하더라도 집착할 필요는 없다. 후자의 예시로는 DNS가 있다.

DNS는 도메인에 따른 ip 주소만을 넘겨주면 모든 작업을 마친다. 따라서  만약 손실이 발생하더라도 ip만 다시 전송하면 되기 때문에 손실에 대한 오버헤드가 무척이나 작은 편이다. 이에 따라 굳이 핸드 쉐이크를 통한 오버헤드를 감내하지 않고 UDP로 통신하고 만약 유실될 경우 재전송을 요청한다.

<b><u>UDP가 이러한 성질을 갖는 이유는 연결 지향적이지 않기 떄문이다. 연결이 없으니 신뢰성을 보장할 수 없고 그만큼 오버헤드가 적어 빠르다.</u></b>

> [!info]
> UDP는 빠르지만 신뢰성을 갖지는 않는다. TCP가 캐치볼이면 UDP는 기관총 난사다.

![[스크린샷 2023-12-01 오후 10.04.17.png]]

___
### 유튜브는 UDP를 쓸까?
**유튜브는 TCP를 쓴다. 위에서 스트리밍 서비스에 UDP를 사용한다고 했지만, 대표적인 스트리밍 서비스인 유튜브는 TCP를 사용한다.** 이는 크게 보안 이슈와 녹화 파일이라는 특성 떄문이다.

보안 이슈부터 살펴보면 UDP는 DDOS등에 취약한 방법인데 이는 전송 여부를 서로 확인하는 시간 없이 무지성 송,수신이 가능하기 때문이다. 이러한 문제로 인해 UDP의 사용을 자제하는 경향이 있다.

또한 유튜브의 메인 서비스는 실시간 스트리밍이 아닌 녹화 파일 스트리밍이다. 유튜브는 녹화 파일이기 때문에 파일이기 때문에 파일의 패킷이 손실된다면 패킷이 깨지는 등의 문제가 발생할 수 있다.  

* **그럼 유튜브는 느린가?**
유튜브는  UDP를 활용해 전송 속도를 늘리기보다. TCP를 통해 네트워크 상태에 대한 피드백을 받고 네트워크 상황에 따른 전송 데이터의 양을 조절하는  DASH 방식을 채택했다.
___
### UDP와 DDOS
앞서 보안 이슈로 인해  UDP를 사용하지 않는 것일 수도 있다 했는데 실제로 UDP를 활용하면 DDOS 공격을 진행할 수 있다. 이는 **응답을 확인하지 않는다는 UDP의 맹점을 활용한 방식인데 UDP 패킷을 기관총 처럼 난사해 서버를 다운 시키는 방법**이다. 이 경우 서버의 포트가 전부 차거나 리소스 과부하로 서버가 다운될 수 있다. 실제로 DNS를 이런 식으로 공격해 다운 시키는 경우가 종종 있었다.
![[Pasted image 20231201223133.png]]
___
### UDP 브로드캐스트
브로드캐스팅을 하기 위해선 UDP를 사용해야만 한다.  TCP로는 브로드 캐스팅을 할 수 없다. 추가적으로 복수 개의 대상에 동시에 전송하는 멀티 캐스트 역시 UDP로만 가능하다.

* **잠깐 브로드 캐스팅, 멀티 캐스팅이 뭔가요?**
**브로드캐스팅은 소속된 LAN 전체에 패킷을 전송하는 행위**를 말한다. 이때 패킷의 응답을 기대하지 않는다. 브로드캐스팅은 주로 ip, MAC등의 주소 파악을 위해 활용한다. [[ARP]]

**멀티 캐스팅은 브로드 캐스팅과 달리 전체가 아닌 특정 집단을 대상으로만 전송하는 행위**를 말한다. 멀티 캐스트와 브로드캐스트는 모두 1번에 여러 대상에게 전송을 하기 때문에 송신자와 네트워크의 부담을 덜어준다는 간점이 있다.

수신자가 복수 개 존재하는 경우 TCP를 사용할 수 없는 이유는 단순하다.  TCP의 경우 핸드 쉐이크를 통해 연결을 수립하고 해당 연결을 통해서만 데이터를 전송하는 형태이기 때문이다. 따라서  <u><b>각 커넥션 별로 패킷을 전송해야하지 한번에 복수 대상에 패킷을 전송하는 것은 불가능</b></u>하다.
___
### 간단한 UDP 에코 서버

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock;
    char message[BUF_SIZE];
    int str_len;
    socklen_t clnt_adr_sz;

    struct sockaddr_in serv_adr, clnt_adr;

    if (argc != 2)
    {
        printf("Usage:%s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (serv_sock == -1)
        error_handling("UDP socket creation error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    while (1)
    {
        memset(message, 0, BUF_SIZE);
        clnt_adr_sz = sizeof(clnt_adr);
        str_len = recvfrom(serv_sock, message, BUF_SIZE, 0, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        printf("server received: %s\n", message);
        sendto(serv_sock, message, str_len, 0, (struct sockaddr *)&clnt_adr, clnt_adr_sz); //읭? 어디다 전송을 하는거야?
    }
    close(serv_sock);
    return 0;
}
```
___
### TCP, UDP 코드로 비교 해보기

#### TCP
TCP는 **리슨 소켓을 통해 연결 요청을 감지**하다가 **요청이 들어오면 accpet를 실행**한다. **accept의 반환 값으로 연결된 소켓 객체를 획득**하고 해당 **소켓을 통해 원하는 데이터를 송신**한다. accept가 완료되기 전까지는 함수의 실행흐름이 블락 되며 다른 사용자들은 서버에 접속하지 못하는 상황이 발생한다.

```c
 listen(server_socket, 5); //요청 리슨
 while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len); // 클라이언트 커넥션 할당
        struct listen_socket sc;
        sc.fd = server_socket;
        for (int i = 0; i < client_cnt; i++)
        {
            memset(buffer, 0, MAX_MESSAGE_LEN);
            read(client_socket, buffer, MAX_MESSAGE_LEN); //read socket
            printf("Client:%d says %s\n", clients[i], buffer);
            write(client_socket, buffer, MAX_MESSAGE_LEN); //write socket
        }
    }
```

#### UDP
**UDP는 요청을 listen할 필요도 accept할 필요도 없다.** UDP는 연결 지향적이지 않고 상대방의 상태를 고려하지 않는 stateless한 성질을 가지기 때문에 데이터를 보냈다는 사실에만 집중한다. **따라서 TCP와 달리 커넥션이 성사된 소켓이 존재하지 않고 이를 통해 통신을 진행하지 않는다.**  

<b><u>UDP는 recvfrom과 sendto를 활용해 통신을 진행하는데 커넥션 소켓에서 데이터를 읽고 쓰는 TCP와 달리 UDP는 전송 받은 메시지에서 목적지, 도착지의 주소를 직접 읽고 이를 헤더에 작성해야 하기 때문이다.</u></b>

```c
 while (1)
    {
        memset(message, 0, BUF_SIZE);
        clnt_adr_sz = sizeof(clnt_adr);
        str_len = recvfrom(serv_sock, message, BUF_SIZE, 0, (struct sockaddr *)&clnt_adr, &clnt_adr_sz); //read message and get client address
        printf("server received: %s\n", message);
        sendto(serv_sock, message, str_len, 0, (struct sockaddr *)&clnt_adr, clnt_adr_sz); //write destination to header and send to socket
    }
```

#### 정리
TCP는 신뢰성을 보장해야하기 때문에 통신 이전에 많은 사전작업을 진행한다. 이후 연결이 성사돼면 해당 소켓을 통해 통신을 진행하면 된다. 이 연결은 1:1로 성립되며 서버와 클라이언트는 서로의 상태를 지속적으로 인지하는 것이 가능하다.

UDP는 사전 작업 없이 곧장 통신을 진행한다. 연결이 없기 때문에 클라이언트와 서버는 서로의 상태를 인지할 수 없으며, 세그먼트 헤더 정보를 통해 메시지의 발신지 정도만 파악할 뿐이다. UDP는 연결이 없기 때문에 세그먼트 헤더에서 데이터를 읽고 헤더에 직접 통신 대상의 정보를 작성해 통신한다. 

>[!info]
> **TCP가 전화라면 UDP는 편지의 방식이다.**

___
### UDP 소켓 통신 방식

![[Pasted image 20231222181115.png]]

UDP 소켓은 위의 이미지와 같은 순서로 동작한다. 상남자 방식이다. 악수하고 그런 과정 없다. 눈 마주치면 바로 통신 개시다. 당연히 신뢰성을 보장할 수 없다.
___
### UDP 세그먼트

![[Pasted image 20231222173337.png]]
세그먼트 역시 상남자다. 이런저런 요소가 덕지덕지 붙은 TCP와 비교가 안될정도로 가볍다. UDP는   TCP에 비해 단순하다 보니 필요 데이터가 적고 이것이 작은 세그먼트라는 결과를 만들었다. 

![[Pasted image 20231222182937.png]]

> [!info]
> **발신 포트와 출발 포트만 알면 어디든 갈 수 있어..**

___
### UDP의 미래

네트워크 수업을 수강하면 TCP를 설명하는데 90% 정도의 시간을 할애하고 10%의 시간동안 UDP를 설명한다는 것을 경험한 사람이 많을 것이다. 묘하게 찬밥 취급을 받는데, 요즘은 오히려 UDP가 대세가 되고 있다. 아래는  UDP가 사용되는 최신 기술들이다.

*  **HTTP 3.0**
	HTTP1.0의 업그레이드 버전으로 더 빠른 속도를 제공한다.

* **QUIC**
	구글 서비스간 통신에 사용하는 프로토콜로 UDP 기반으로 동작한다.

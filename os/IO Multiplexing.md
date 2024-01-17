### 출처
* https://rammuking.tistory.com/entry/Epoll%EC%9D%98-%EA%B8%B0%EC%B4%88-%EA%B0%9C%EB%85%90-%EB%B0%8F-%EC%82%AC%EC%9A%A9-%EB%B0%A9%EB%B2%95 (epoll 기초)
* https://www.marccostello.com/async-await-pitfall-1-blocking-async-calls/ (async-await in python)
* https://reakwon.tistory.com/117 (select)
* https://stackoverflow.com/questions/17355593/why-is-epoll-faster-than-select (epoll vs select
* https://niklasjang.github.io/backend/select-poll-epoll/ (epoll & select)
___
### 개요
[[#Intro]]
[[#Why Async-Blocking is bad]]
[[#await]]
[[#select]]
[[#epoll]]
[[#epoll 더 알아보기]]
[[#epoll 결론]]
___
### Intro

지난시간 [[동기와 비동기 (Blocking, None-Blocking)]]를 학습하며 동기, 비동기의 차이 Blocking, None-Blocking의 차이를 학습했다. 동기는 호출 결과를 신경쓰는 방식, 블락킹은 함수의 실행 흐름이 정체되는 현상이라는 것을 기억할 것이다. 또한 비동기와 논-블락킹은 이들 각각과 상반된 키워드들이라는 것까지 떠올릴 수 있어야 한다. 이제 이러한 키워드들이 어디서 어떻게 사용 되는지를 파악할 것이다. 

**==비동기와 논블락킹을 활용하는 사례의 99%는 IO와 관련된 작업들이다. IO가 아닌 작업일 경우 백 그라운드에서 실행할 수 없기 때문에 논 블락킹이 불가능하기 때문이다.==** 예를 들어 큰 행렬 연산을 비동기로 실행한다 하면 단일 스레드라고 할때 해당 연산이 실행되는 동안 실행 흐름은 블락 되므로 일반적인 동기로 처리하는 것과 큰 차이가 없다. 하지만 IO의 경우 흐름을 블락하지 않고 백 그라운드에서 처리가 가능하므로 비동기, 논 블락킹과 조합하기 좋은 작업이다.  

> [!info]
> **CPU burst 작업이면 백 그라운드 실행이 불가능하므로 함수의 실행시간 동안 흐름이 멈추게 되므로 비동기의 효과가 없다.**

___
### Why Async-Blocking is bad

비동기-블락킹은 아예 블락킹한 로직을 비동기로 구현하는 경우 뿐만 아니라 **블락킹 하는 영역이 조금이라도 포함되면 모두 비동기-블락킹이라 칭한다.** 예를 들어 특정 리퀘스트를 비동기로 호출하고 응답을 동기-블락킹 방식으로 처리할 경우도 비동기-블락킹에 해당한다. 이는 큰 성능 저하를 야기하는데,아래의 코드를 살펴보자.

```python
COUNT = 10
URLS = [
    "http://www.foxnews.com/",
    "http://www.cnn.com/",
    "http://www.bbc.co.uk/",
] * COUNT

def load_url(url, timeout):
    with urllib.request.urlopen(url, timeout=timeout) as conn:
        print(f"Run {url}")
        return conn  # read is blocking
        
async def async_multy(url):
    loop = asyncio.get_event_loop()
    # partial을 통해 매개변수를 미리 입력해준다.
    requset = partial(urllib.request.urlopen, url, timeout=5)
    print(f"Run {url}")
    response = await loop.run_in_executor(
        None, requset
    )  # requset 대기 부분을 await로 처리해 비동기로 작동하게 한다.
    print(f"Done {url} page {response.read()}")
    return url, response


async def main():
    start = time.time() #시작시간
    futures = [asyncio.create_task(async_multy(url)) for url in URLS] #비동기 루틴 등록
    await asyncio.gather(*futures) #비동기 종료대기
    end = time.time() #종료시간
    print(f"Asyncio Runtime: {end - start}")
```

아래의 코드는 다음과 같은 순서로 동작한다.

1. 특정 리퀘스트를 복수개 비동기로 전송
2. 리퀘스트는 전부 백 그라운드에서 동시 실행
3. 리퀘스트 완료 파악
4. 응답을 바이트 단위로 읽음 (블락킹 발생)

이 경우 ==**4번이 블락킹 방식으로 실행 되면서 병목현상이 발생하고 처리 속도가 크게 저하된다. 따라서 비동기-논블락킹으로 로직을 작성했다면, 되도록 전체 로직을 비동기-논블락킹으로 구성하는 것이 효율적이다.==**

>[!info]
>**그렇지만 순수 비동기-논블락킹으로 구성하기는 어려운 일이다. 순서(완료의 보장 등)를 배제해야하기 때문이다.**
___

___
### await
```python
import asyncio
import random


async def do_something(i: int):
    sec = random.randint(1, 10)
    print(f"{i} Wait for {sec}...")  # random wait...
    await asyncio.sleep(sec)  # sleep for secs
    print(f"{i} waiting done")


async def main():
    tasks = [asyncio.create_task(do_something(i)) for i in range(3)]
    await asyncio.gather(*tasks) #wait for all task end...


if __name__ == "__main__":
    asyncio.run(main())

```

비동기 함수의 어려운 점은 실행흐름의 관리가 어렵다는 것이다. 위의 예제를 보더라도 3번 실행하는 do_something중 어떤 경우가 먼저 종료 될지를 보장할 수 없다. 또한 함수의 종료를 명시적으로 대기해주지 않으면 함수가 전부 완료되지 않은 채로 프로그램이 종료될 수도 있다.

이에 따라 **비동기 함수가 명시적으로 종료될때 까지 실행흐름을 블락하는 기능이 필요해졌는데 이때 사용하는 것이 await**이다. await를 사용하면 해당 함수의 실행이 모두 완료될 때까지 실행 흐름이 블락되며 이에 따라 await이후로는 함수의 실행 완료를 보장할 수 있다.

여기서 생각해볼 점은 함수의 완료를 어떻게 파악하느냐이다. 위의 gather 함수는 어떻게 모든 함수의 종료를 파악하는 걸까? 가장 기본적인 방법으로 busy-waiting을 떠올릴 수 있다.
```python
done_count = 0 #task done count
while done_count < task_count:
	if check_function_done(): #check function is finished (busy-wait)
			task_count += 1
```

하지만 앞서 말했듯이 busy-waiting은 cpu 낭비를 많이 발생 시키는 비효율적 방법이다. gather의 경우 이벤트 루프의 도움을 받아 이를 해결하는데 이는 차차 알아보고 이하에서는 근본적으로 어떻게 다중 IO에서 busy-waiting을 피하면서 완료를 감지 했는지 그 발전 역사를 살펴 보도록 하자.
___
### select

select()는 POSIX에 따라 구현돼야 하는 시스템 콜 중 하나로, **==여러 개의 파일 디스크립터를 모니터링하는 기능을 제공하는 함수이다.==** select()를 활용하면 우리는 특정 파일 디스크립터에서 발생한 이벤트를 감지할 수 있다.

> <b><u>select() allows a program to monitor multiple file descriptors, waiting until one or more of the file descriptors become "ready" for some class of I/O operation (e.g., input possible).</u></b> A file descriptor is considered ready if it is possible to perform a corresponding I/O operation (e.g.,read(2), or a sufficiently small write(2)) without blocking. 

```python
import socket
import select


HOST = "www.naver.com"
HTTP_MSG = f"GET / HTTP/1.1\r\nHost: {HOST}\r\n\r\n"


def sync_select():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # 블로킹 모드를 비블로킹 모드로 변경
    http_request = HTTP_MSG.encode("utf-8")
    client_socket.connect((HOST, 80))

    client_socket.sendall(http_request)
    res = b""
    client_socket.setblocking(False)

    while True:
        try:
            msg = client_socket.recv(32)
            res += msg
            if not msg:
                break

        except Exception:
            print("busy wait...")
            select.select([client_socket], [], []) #wait for event

    return res


def sync_busy_wait():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # 블로킹 모드를 비블로킹 모드로 변경
    http_request = HTTP_MSG.encode("utf-8")
    client_socket.connect((HOST, 80))

    client_socket.sendall(http_request)
    res = b""
    client_socket.setblocking(False)

    while True:
        try:
            msg = client_socket.recv(32)
            res += msg
            if not msg:
                break

        except Exception:
            print("busy wait...")
            continue

    return res
```

위의 예시는 select를 활용해 대기를 했을 때와 busy-wait을 했을 때의 동작 차이를 한눈에 비교할 수 있는 예시이다. 계속해서 while문을 탐색하는 busy-wait 방식과 달리 select 방식을 사용하면 한번만 대기를 수행하는 것을 확인할 수 있다. 이는 **==select가 등록한 파일 디스크립터에 입출력 가능 이벤트가 발생할 때까지 블락하는 방식으로 동작하기 때문이다.==**

**select() 는 FD_SET(등록한 파일 디스크립터 집합) 바이트 배열의 특정 인덱스를 검사하는 방법으로 해당 파일의 상태를 파악**하는데, 이때 검사하는 index는 검사하고자 하는 파일의 fd값과 동일하다.

![[Pasted image 20231121183520.png]]

fd값이 3인 소켓의 이벤트를 확인하고 싶은 경우 다음과 같이 동작한다. **select()는 계속해서 3까지의 모든 fd에 대한 이벤트를 조사하다 3에 이벤트가 발생할 경우 FD_SET의 해당 fd 인덱스의 값을 1로 활성화 한다.** 이는 타임아웃이 발생할 때까지 반복되는데 <b><u>관심있는 FD는 3번 하나인데 이를 위해 4개를 감시해야하니 무척 비효율적이다.</u></b> 이후 이벤트가 발생한 fd에서 데이터를 읽은 후 FD_SET을 다시 초기화 해준다.

![[Pasted image 20231121183535.png]]

따라서 select의 동작순서를 정리하면 다음과 같다.

1. 내가 관심있는 fd + 1 크기의 FD_SET 큐를 생성한다.
2. 해당 큐를 루프하며 이벤트를 탐지한다. 
3. 이벤트를 탐지한 후에는 다시 FD_SET을 비활성화로 초기화 한다.

C언어로 작성된 select 사용 예시를 살펴보면 확인할 수 있다.

```c
while (1)
	{
		FD_ZERO(&readFds); //파일 디스크립터 집합 초기화
		FD_SET(socket->socket, &readFds); //집합에 파일 디스크립터 등록
		select(socket->socket + 1, &readFds, NULL, NULL, &timeout); //타임아웃까지 집합 검사

		if (FD_ISSET(socket->socket, &readFds)) //fd에 이벤트가 발생했는지 커널에 질문
		{
			rx_len = read(socket->socket, socket->buffer, MAX_MESSAGE_LEN); //read
			if (rx_len > 0)
			{
				printf("\n%s\n", socket->buffer);
				memset(socket->buffer, 0, MAX_MESSAGE_LEN);
			}
			else
			{
				printf("Fail to read data\n");
				exit(0);
			}
		}
	}
    return NULL;
```

<span class="red red-bg"><b>이러한 방식에는 문제가 있는데, 가뜩이나 비효율적으로 큰 FD_SET을 매번 검사하고 초기화하는 작업을 반복해야 한다는 것이다.</b></span>  select를 활용하면 단일 스레드에서 다중 IO를 구현할 수 있다. 하지만 **fd가 비효율적으로 관리되는 문제,  잦은 모드 스위칭 발생 문제** 등의 이슈가 존재하고 이로 인해 잘 사용하지 않는다.

#### FD_SET과 모드 스위칭 문제

<b><u>FD_SET는 감시할 파일 디스크립터 값을 저장하는 자료구조로 유저 영역에 위치한다.</b></u> FD_SET은 유저 영역에 위치하기 때문에 fd를 쉽게 등록하거나 삭제하는 것이 가능해졌다. 또한 유저 영역을 활용함으로써 select를 설계할 당시 중요했던 커널 메모리 공간 효율화도 이뤄낼 수 있었다.

하지만 <b><u>실질적인 IO 발생, 완료 여부는 커널과 통신 해야만 파악이 가능했기 때문에 select를 호출 할 때 마다 커널 영역으로 FD_SET을 복사해줘야 하는 오버헤드가 발생했다.</u></b> 이는 select를 호출할 때마다 발생하기에 처리하는 IO가 많은 경우 큰 이슈였고 select가 과거의 유산이 되는데 큰 기여를 했다.

[내용 추가 (윤영민)](https://github.com/thinkjin99/my_study/issues/6)
___
### epoll

epoll은 select의 단점을 보완해 만든 IO 감지 모델을 말한다. select의 문제점은 아래와 같았다.

1. 관심있는 fd만 탐지하는 것이 불가능 함.
2. fd를 연속으로 탐지해야 하기에 FD_SET의 크기가 큼
3. 커널 영역에 접근할 일이 잦음 (FD_ISSET)


**epoll은 내가 이벤트를 감지하고 싶은 파일을 특정해 커널에 전달할 수 있다.** 따라서 관심있는 fd의 이벤트만을 감지하는 것이 가능하다. 또한 커널에서 FD_SET(Event Set)을 관리하기 때문에 유저-커널 통신으로 인한 오버헤드가 적은 편이다. 

또한 **epoll을 활용할 경우 select에 비해 스위칭도 적게 할 수 있다**. select의 경우 관심있는 fd들을 등록한 FD_SET이 유저영역에 위치해 매번 커널로 복사해줘야만 했다. 하지만 **epoll의 경우 이벤트 구독 리스트(FD_SET과 흡사)는 커널 영역에 위치하기 때문에 유저가 매번 관심있는 fd 리스트를 넘겨줄 필요가 없고 이로 인해 스위칭 발생이 적다**. 유저는 커널 영역에 위치한 준비된 이벤트 리스트만 접근해 어떤 이벤트가 발생 했는지만 확인하면 된다.

> [!info]
> **==epoll은 관심있는 fd만 등록해 이벤트를 감지하고 커널에서 처리하는 방식이다.==**

___
### epoll 더 알아보기

<b><u>epoll API는 epoll 인스턴스를 통해 커널과 통신하는 방법으로 동작한다.</u></b> epoll 인스턴스는 두가지 리스트를 포함하는데 하나가 **이벤트를 구독중인 fd를 등록하는 리스트**이고 하나는 **IO가 준비된 준비된 fd를 관리하는 리스트**이다. 

epoll 인스턴스는 소켓과 흡사하게 커널과 유저 영역 사이의 인터페이스로 동작한다. 따라서 직접 **FD_SET을 커널에 넘겨주며 IO 완료 여부를 물어봐야 했던 select와 달리 유저 영역에 있는 인스턴스의 상태만 체크하면 되므로 epoll이 더 효율적으로 동작한다. (모드 스위칭이 적다)**

epoll의 동작 흐름을 살펴보면 우선 **epoll 인스턴스를 생성**하고 해당 **인스턴스에 이벤트를 구독하는 fd를 등록**한다. 이후 **정해진 시간 동안 이벤트 발생 여부를 확인**하고 **이벤트가 발생했을 경우에는 적절한 콜백 처리**를 진행 해준다. 아래에서 자세히 살펴보자.
#### epoll_event
**[epoll event](https://man7.org/linux/man-pages/man3/epoll_event.3type.html)는 이벤트가 발생했을 경우 커널이 인스턴스를 통해 전달해줘야 할 데이터를 정의**한다. 이벤트는 아래와 같은 구조로 작성돼 있으며, **이벤트가 발생한 파일 디스크립터 정보 등을 저장**한다. 

```c
struct epoll_event {
	uint32_t      events;  /* Epoll events */
    epoll_data_t  data;    /* User data variable */
};

union epoll_data {
    void     *ptr;
    int       fd;
    uint32_t  u32;
    uint64_t  u64;
};
```

#### epoll_create
[epoll_create](https://man7.org/linux/man-pages/man2/epoll_create.2.html)은 <u><b>새 epoll 인스턴스를 생성하는 함수이다. </b></u> epoll 인스턴스의 fd 값을 반환하며 해당 fd를 통해 인스턴스에 접근할 수 있다. 더 이상 해당 인스턴스를 참조하는 객체가 없을 경우 커널에 의해 삭제된다. 근래 리눅스에서는 형태가 조금 변경된 epoll_create1을 활용한다.

#### epoll_ctl
[epoll_ctl](https://man7.org/linux/man-pages/man2/epoll_ctl.2.html)은 <b><u>epoll 인스턴스에 실질적으로 접근하는 함수로 인스턴스에서 관리하는 이벤트 구독 리스트에 fd를 추가, 삭제, 수정하는 작업들을 수행한다.</u></b> 상수를 통해 작업의 종류를 결정하며 사용하는 상수는 **EPOLL_CTL_ADD**, **EPOLL_CTL_MOD**, **EPOLL_CTL_DEL** 등이 있다. 

#### epoll_wait
[epoll_wait](https://man7.org/linux/man-pages/man2/epoll_wait.2.html)는 <span class= "red red-bg"><b>블락킹 함수로 구독한 fd에서 이벤트가 발생하는지 정해진 시간 동안 대기하는 함수이다.</b></span> 발생한 이벤트가 존재할 경우 **인스턴스의 ready 리스트에서 커널로 부터 전달 받은 evnet 객체를 유저 영역의 events배열에 넘겨준다.** 따라서 유저는 커널 영역의 완료된 이벤트 리스트를 한번만 확인하면 발생한 모든 이벤트에 접근 가능하다. 반환 값은 발생한 이벤트의 수이다.
___
### epoll 써먹기

아래는 epoll을 활용한 실제 TCP 소켓 서버이다. 코드를 살펴보자.
```c
typedef struct {
    int sockfd;
    char buffer[BUF_SIZE];
} Client;

// Initialize client
void initClient(Client *client, int sockfd) {
    client->sockfd = sockfd;
    memset(client->buffer, 0, sizeof(client->buffer));
}
```
클라이언트 연결을 구조체를 활용해 표현한다. 클라이언트 구조체는 소켓과 해당 소켓의 버퍼를 포함한다.

```c
// Receive data from a client
void receiveData(Client *client) {
    ssize_t bytesRead = recv(client->sockfd, client->buffer, sizeof(client->buffer), 0);

    if (bytesRead > 0) {
        client->buffer[bytesRead] = '\0';
        printf("Received data from client %d: %s\n", client->sockfd, client->buffer);
    } else if (bytesRead == 0) {
        printf("Connection closed by client %d\n", client->sockfd);
        close(client->sockfd);
    } else {
        perror("Error in recv");
        close(client->sockfd);
    }
}
```
클라이언트로 부터 전달된 데이터를 읽는다. 이후 소켓 버퍼를 비워주는 작업을 진행한다. 

```c
listen(serverSockfd, SOMAXCONN);

// Create epoll instance
epollfd = epoll_create1(0);
struct epoll_event event;
event.events = EPOLLIN; //이벤트 타입으로 read 이벤트를 의미한다.
event.data.fd = serverSockfd; //리스닝 소켓 등록
epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSockfd, &event); //인스턴스의 구독 리스트에 추가
```

**epoll 인스턴스를 생성하고 이벤트를 정의해 서버의 리스닝 소켓의 read 이벤트를 구독 리스트에 등록한다**. 이후 리스닝 소켓에서 READ가 가능해지면 해당 이벤트는 ready 리스트로 이동하고 epoll 인스턴스는 이를 접근해 이벤트가 발생했다는 것을 사용자가 알수 있게한다.

```c
while (1) { // 이벤트 루프의 시작
	int numEvents = epoll_wait(epollfd, events, MAX_EVENTS, -1); //이벤트가 발생할 때 까지 블락
	for (int i = 0; i < numEvents; ++i) {
		if (events[i].data.fd == serverSockfd) { //발생한 이벤트가 리스닝 소켓이면
			// Accept a new connection
			clientSockfd = accept(serverSockfd, (struct sockaddr *)&clientAddr, &clientAddrLen); //연결 수립
			printf("Accepted a new connection from %s:%d\n",
				   inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

			// Initialize and add the new client to epoll
			initClient(&clients[clientSockfd], clientSockfd); //클라이언트 정보 설정
			event.events = EPOLLIN; //읽기 이벤트 
			event.data.fd = clientSockfd; //수립한 클라이언트 커넥션
			epoll_ctl(epollfd, EPOLL_CTL_ADD, clientSockfd, &event); //이벤트 구독
		} else {
			// Receive data from an existing client
			receiveData(&clients[events[i].data.fd]); //클라이언트에서 이벤트 발생시 
		}
	}
}
```

==**반복문을 순회 하면서 이벤트가 발생할 때까지 잠깐 대기를 진행**==한다. 이벤트가 발생했을 경우 이벤트가 어떤 fd에서 발생했는지를 확인해 리스닝 소켓일 경우 연결을 수립하고 아닐 경우 메시지를 읽는 작업을 수행한다. 

<span class="red red-bg"><b>연결 수립시 새로 수립한 연결에 대한 read 이벤트를 생성하고 이를 리스트에 등록하는 작업 또한 진행한다. 이를 통해 신규로 성립된 클라이언트 커넥션에 대한 이벤트도 인스턴스를 통해 파악 가능해진다.</b></span>

이러한 구조는 <b><u>이벤트 루프에 대한 대표적 예시이며 이를 통해 이벤트에 대한 적절한 리스닝 및 핸들링이 가능해진다.</u></b>

>[!info]
>**epoll은 이벤트 루프의 핵심이며 이를 통해 싱글 쓰레드에서의 멀티 IO를 효율적으로 구현할 수 있다.**

```c
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


```

mac에서는 epoll이 아닌 kqueue를 사용하기 때문에 위의 코드를 실행하기 위해선 도커를 사용해야 한다.

```dockerfile
# 베이스 이미지 설정
FROM ubuntu:20.04

# 환경 변수 설정
ENV DEBIAN_FRONTEND=noninteractive

# 패키지 업데이트 및 필요한 패키지 설치
RUN apt-get update -y

RUN apt-get install -y \
    gcc \
    git \
    vim \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 작업 디렉토리 설정 (원하는 디렉토리로 변경 가능)
WORKDIR /usr/src/app

RUN git clone https://github.com/libuv/libuv.git

COPY ./server.c .

# 도커 컨테이너가 시작될 때 실행할 명령 (예: 셸 실행)
CMD ["/bin/bash"]

```

위의 도커 파일을 설정한 후 아래의 커맨드를 입력한다. 이후 접속한 터미널에서 server.c를 컴파일 후 실행하면 된다.
```bash
docker build -t epoll .
docker run --rm -it -p 8080:12345 epoll:latest

#컨테이너 터미널 접속 이후
gcc server.c && ./a.out
```

테스트를 위한 클라이언트 코드는 아래와 같다.
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int main()
{

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("couldn't create socket\n");
        return 1;
    }
    printf("socket created\n");

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("couldn't connect\n");
        return 1;
    }
    printf("connected to the server\n");

    char msg[100], server_msg[100];

    while (1)
    {
        printf("write message : ");
        scanf("%[^\n]%*c", msg);
        send(sockfd, msg, sizeof(msg), 0);

        memset(server_msg, 0, sizeof(server_msg));
        recv(sockfd, server_msg, sizeof(server_msg), 0);
        printf("Server reply : %s\n", server_msg);

        if (strcmp(server_msg, "bye") == 0)
        {
            printf("exiting...\n");
            break;
        }
    }

    close(sockfd);

    return 0;
}
```
_____
### epoll 결론

epoll은 select의 발전 형으로 커널 메모리의 크기가 증가돼 커널 영역에 이벤트 리스트를 배치하는게 가능해지며 등장했다. epoll을 사용함으로써 기존의 과정이 다음과 같이 변경됐다.

* **select**
	유저 -> fd 등록 -> FD_SET 커널 전달 -> select 실행 -> 커널에서 유저로 FD_SET 재 전달 -> 유저에서 FD_SET 순회하며 이벤트 검출

* **epll**
	유저 -> fd 등록 -> 커널의 등록 리스트에 반영 -> epoll_wait 실행 -> 커널에서 유저로 발생한 이벤트 전달 -> 발생한 이벤트 처리

<b><u>이벤트 발생유무와 관계 없이 매번 시스템 콜을 호출하며 FD_SET을 복사해야하는 select와 달리 epoll은 이벤트가 존재하는 경우에만 커널 데이터를 복사하면 된다.</u></b> 또한 SET 전체를 복사해야하는 select와 달리 발생한 이벤트 혹은 등록할 이벤트 정도만 복사하면 되기에 시스템 콜로 인한 오버헤드가 적다.

추가적으로 <b><u>select의 경우 어떤 fd에서 이벤트가 발생했을지 모르기 때문에 전체 FD_SET을 항시 순회해야 하지만, epoll의 경우 발생한 이벤트에 한해서만 순회하며 처리해주면 되기 때문에 훨씬 효율적인 처리가 가능하다.</b></u>

 > [!info]
 >  **epoll은 커널 영역으로 사용함으로써  빠른 이벤트 처리가 가능 해지고 시스템 콜 오버헤드를 줄였다.**

[내용 추가 (윤영민)](https://github.com/thinkjin99/my_study/issues/6)
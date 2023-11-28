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
> \- POSIX
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

<span class="red red-bg"><b>이러한 방식에는 문제가 있는데, 가뜩이나 비효율적으로 큰 FD_SET을 매번 검사하고 초기화하는 작업을 반복해야 한다는 것이다.</b></span>  select를 활용하면 단일 스레드에서 다중 IO를 구현할 수 있지만, FD_SET을 비효율적으로 관리하는 문제, fd의 수가 제한적이라는 문제 등으로 인해 잘 활용하진 않는다.
___
### epoll
epoll은 select의 단점을 보완해 만든 IO 감지 모델을 말한다. select의 문제점은 아래와 같았다.

1. 관심있는 fd만 탐지하는 것이 불가능 함.
2. fd를 연속으로 탐지해야 하기에 FD_SET의 크기가 큼
3. 유저와 커널이 소통할 일이 잦음 (FD_ISSET)

**epoll은 내가 이벤트를 감지하고 싶은 FD_SET을 특정해 커널에 전달할 수 있다.** 따라서 관심있는 fd의 이벤트만을 감지하는 것이 가능하다. 또한 커널에서 FD_SET을 관리하기 때문에 유저-커널 통신으로 인한 오버헤드가 적은 편이다. epoll은 다음과 같이 동작한다.

1. epoll 객체를 생성
2. epoll_ctl을 통해 등록할 fd와 이벤트 등을 설정한다.
3. epoll_wait를 통해 이벤트가 발생한 fd와 발생한 이벤트를 전달 받는다.

epoll역시 FD_SET을 루프하고 있지만, FD_SET이 커널 영역에 위치해 유저 영역에서 루프를 돌던 select에 비해 리소스가 적게 든다. 또한 <span class="red-bg red"><b>epoll은 select와 달리 관심있는 fd만을 루프하기 때문에 소요시간이 훨씬 적고 이를 관리하는 작업의 규모도 작다.</b></span>

> [!info]
> epoll은 관심있는 fd만 등록해 이벤트를 감지하고 커널에서 처리하는 방식이다.

___



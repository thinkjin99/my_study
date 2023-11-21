### 출처

___
### 개요

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

fd값이 3인 소켓의 이벤트를 확인하고 싶은 경우 다음과 같이 동작한다. **select()는 계속해서 3까지의 모든 fd에 대한 이벤트를 조사하다 3에 이벤트가 발생할 경우 FD_SET의 해당 fd 인덱스의 값을 1로 활성화 한다. 이는 타임아웃이 발생할 때까지 계속된다.**

![[Pasted image 20231121183535.png]]

이후 FD_ISSET은 매크로 함수로 fd_set의 특정 인덱스가 활성화 됐는지를 검사한다. 활성화 됐다면 이벤트가 발생했다는 의미이므로 데이터를 수신할 수 있다.

select를 활용하면 다중 IO를 배열에 등록해두고 FD_SET을 계속해서 순회하는 방식으로 이벤트를 감지할 수 있다. 하지만 이 역시 <b><u>FD_SET을 계속해서 순회해야 한다는 점과 등록할 수 있는 디스크립터의 수가 제한적이라는 한계로 인해 현재는 잘 사용되지 않는다.</u></b>
___
### epoll
epoll은 select의 단점을 보완해 만든 IO 감지 모델을 말한다. 파일 디스크립터를 유저가 아닌 커널이 관리를 하며, 이로 인해 CPU가 직접 FD_SET을 루프로 순회하며 이벤트를 감지할 필요가 없다. epoll은 커널에서 이벤트를 감지하며 이벤트가 발생할 경우 해당 파일 디스크립터를 파악해 커널에서 유저 영역으로 복사해주는 방식으로 동작한다. 



___
### Event Loop


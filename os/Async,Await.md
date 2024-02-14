
### 출처
* https://bentist.tistory.com/89?category=1010744 (비동기 개요)
* https://it-eldorado.tistory.com/159 (테스크의 스케줄링 방법)
* https://tech.buzzvil.com/blog/asyncio-no-1-coroutine-and-eventloop/ (실습 예제)
* https://soooprmx.com/asyncio/ (테스크, 코루틴, 퓨처
* https://stackoverflow.com/questions/49005651/how-does-asyncio-actually-work (파이썬의 asyncio)
___
### 개요
* [[#Async / Await]]
* [[#Async/Await 의 동작방식]]
* [[#Corutine]]
* [[#파이썬의 이벤트 루프]]
* [[#Task]]
* [[#Futures]]
* [[#Corutine, Future, Task 한눈에 보기]]
* [[#파이썬 async의 실행 흐름 자세하게]]
* [[#await 주의점]]
___
### Async / Await

**async/await는 비동기 객체를 쉽게 다루기 위해 고안된 방법으로 미래의 특정 시점에 완료되는 작업을 처리할 때 빛을 발한다.** 해당 방법을 활용하면 콜백 지옥에서 탈출해 depth가 훨씬 적은 우아한 코드 작성이 가능해진다. 

async와 await는 각 언어 별로 사용법이 조금씩 상이하며 해당 글에서는 파이썬을 활용해 설명한다. 아래의 예제를 살펴보자.

```python
def login(user_id:str, password:str):
	db_hash:str = get_db_hash(user_id) #get_db_hash는 비동기, 논 블락킹으로 동작한다.
	#db_hash에 함수의 완료 값이 들어감을 보장할 수 없다.
	if db_hash == do_hash(password):
		return 200
	else: return 401
```

get_db_hash 함수가 비동기-논블락킹으로 동작한다고 하면 db_hash 변수에 함수의 완료 값이 전달 됐는지를 보장하기 위해선 비지 웨이팅 하던가, 시그널 변수, 콜백 등을 별도로 활용해야 한다. 

이 경우 일반적으로 콜백을 활용해 처리했지만 로직이 중첩될 경우 [[이벤트 루프#프로미스 체인|콜백지옥]]에 빠질 수 있다는 것을 앞서 학습했다. 지옥 탈출을 위한 번거로운 작업 없이 async/await를 활용하면 간단히 지옥에서 빠져나올 수 있다.

**async를 사용해 함수를 코루틴으로 만들고 대기가 발생하는 함수 호출부에 await를 선언 해주면 콜백지옥에서 간단히 빠져나올 수 있다**

```python
async def login(user_id:str, password:str):
	db_hash:str = await get_db_hash(user_id) # 이제 해당 함수는 db_hash가 완료될때 까지 블락된다.
	if db_hash == do_hash(password):
		return 200
	else: return 401
```

<span class="red red-bg"><b>await 키워드를 함수 호출 부 앞에 선언하면 호출한 함수가 완료될 때까지 해당 함수를 블락한다는 의미가 된다.</b></span> await 이하에 존재하는 코드들은 호출한 함수의 완료를 보장받을 수 있고 번잡한 콜백 체인을 활용하지 않아도 된다.

>[!info]
>await가 하는 일중 하나는 실행 흐름을 블락하는 것이다.

___
### Async/Await 의 동작방식

 **await가 함수의 실행 흐름을 블락킹 한다면**  이는 논 블락킹의 성질을 해치고 오버헤드를 증가 시킬 것이라는 의심을 할 수 있다. 만약 그렇다면 동기-블락킹을 활용하는 것이 더욱 직관적인 방식일 것이다. 
 
 아래 예제들을 살펴보며 async/await를 활용하는 경우와 동기-블락킹의 차이를 파악하고 동작 방식을 파악해보자.

```python
def saync_random_timer(i):
    pending_time = random.randint(1, 3)
    print(f"Wait {i} for {pending_time}")
    time.sleep(pending_time)
    print(f"{i} is finished...")
    return

def sync_main():
    for i in range(5):
        saync_random_timer(i)
```

해당 함수는 동기-블락킹 방식으로 설계 됐으므로 1번 실행, 종료-> 2번 실행, 종료 -> 3, 4, 5... 로 순차적으로 실행될 것이다. 따라서 5번째 타이머는 앞선 4번의 대기 작업을 마치고 나서야 대기를 시작할 것이다. 응답성이 떨어지는 방식이다.

```python
async def random_timer(i):
    pending_time = random.randint(1, 3)
    print(f"Wait {i} for {pending_time}")
    await asyncio.sleep(pending_time) #여기서 함수가 블락된다
    print(f"{i} is finished...")
    return

async def main():
    tasks = [asyncio.create_task(random_timer(i)) for i in range(5)]
    #print(asyncio.all_tasks()) 테스크 현황 출력
    await asyncio.gather(*tasks)


asyncio.run(main())
```

await를 활용할 경우 이와 다른 방식으로 동작한다. <span class="red red-bg"><b>await를 만나는 순간 쓰레드는 해당 함수를 블락하고 상태를 저장 해둔 다음실행 가능한 다른 함수를 실행한다.</b></span> 따라서 다음의 순서로 동작한다.

1번 실행 -> 2번 실행 -> 3,4,5번 실행  -> 타이머가 먼저 종료되는 함수부터 이어서 실행...

<b><u>await를 활용할 경우 컨텍스트 스위칭 처럼 실행되는 함수의 교체가 진행되고 함수는 상태가 그대로 pending돼 await하고 있는 함수가 완료될 때까지 대기하다 다시 실행된다.</u></b> 따라서 일반적인 동기-블락킹보단 응답성이 개선된 방식으로 동작한다.

> [!info]
> **await를 활용하면 해당 예약어를 선언한 시점에서 실행하는 함수를 교체한다.** 

**이를 이미지로 표현하면 아래와 같다. 여러 코루틴이 await를 기반으로 교체되며 병행적으로 실행된다.**

![[스크린샷 2024-02-13 오후 2.47.33.png]]
#### create_task는 뭐야?
create_task는 뭘까 아래와 같이 await를 즉각적으로 붙여서 코드를 작성할 수는 없을까?
```python
async def main():
    for i in range(5):
        await random_timer(i)
```

이렇게 코드를 작성하면 timer 함수가 끝날 때까지 main으로 실행 권한이 돌아오지 않기 때문에 이러한 코드를 작성하면 안된다. 
await는 크게 두가지의 기능을 수행하는데 첫번째는 예약어 뒤에 존재하는 **코루틴을 실행하는 작업을 진행**하고 두 번째로는 **해당 코루틴이 완료될 때까지 실행권한을 이벤트 루프에 반환**한다.

이에 따라 `await random_timer`를 실행하면 `random_timer`를 실행하고 이후 `await asyncio.sleep(pending_time)` 줄을 만나면 실행 권한을 다시 반환한다. 이때 main이 실행 권한을 얻어 코드가 이어 실행될듯 하지만 main은 `await random_timer`가 종료될 때까지 블락 상태이기 때문에 실행 가능한 코루틴이 존재하지 않게 된다. 따라서 **위와 같이 코드를 작성할 경우 타이머 함수를 병행적으로 실행하지 않고 한개씩 처리하는 동기-블락킹 방식으로 동작한다.**

이를 해결하기 위해선 <span class = "red red-bg">코루틴 여러 개를 이벤트 루프에 등록해주면 된다. 여러 개의 코루틴을 루프에 등록해줄 경우 하나의 코루틴이 권한을 반납하면 다른 코루틴을 실행해주면 된다. </span> 이를 `create_task`가 가능케 해주며 각 코루틴을 task 객체로 감싸 이벤트 루프에 등록해 루프에게 처리할 작업이 복수개 존재한다는 것을 인지시킨다. 

![[스크린샷 2024-02-13 오후 2.41.28.png]]

> [!info]
> **테스크 객체로 코루틴을 감싸줘야 루프가 작업을 스케줄링할 때 다룰 수 있다.**

___
### Corutine

앞서 말했듯이 async/await는 상태를 저장하고 함수를 교체하는 작업을 수행한다. 이에 따라 <span class ="red red-bg"><b>실행 흐름을 기억 했다 다시 실행하는 방식의 동작이 가능한 특별한 형태의 함수를 사용해야 하는데 이것이 코루틴이다.</b></span>

**코루틴이 (await, yield)등을 통해 인위적으로 다른 곳으로 권한을 양도하면 자신의 실행과 관련된 상태를 어딘가에 저장하고 실행을 중지**한다. 이후 다시금 코루틴을 실행하면 중지했던 부분에서 다시 상태를 복원해 작업을 진행한다. 코루틴은 제네레이터와 흡사하지만 **제네레이터와 달리 초기화 이후 값을 전달받는 것이 가능하며 매개변수를 받을 수 있는 제네레이터로서 동작한다.**

코루틴 객체를 만들기 위해서는 async 키워드를 서브 루틴 앞에 추가하면 된다. 이때 주의할 점은 <b><u>코루틴은 함수와 달리 호출을 해도 함수 내부의 코드가 동작하지 않고 코루틴 객체를 반환 한다는 것이다. </u></b> 함수 내부의 코드를 동작시켜 결과 값을 얻기 위해서는 await 구문을 활용해야 한다.

또한 **await 키워드는 코루틴 안에서만 호출할 수 있으므로 코루틴을 실행하기 위해서는 또 다른 코루틴이 요구 된다.** 이 문제는 꼬리에 꼬리를 물며 계속 연결되므로 이를 해결하기 위해서는 코루틴의 시작을 일반 함수나 함수 외부에서 실행 할 수 있게 해줘야 한다.

`asyncio.run()`을 통해 이 문제를 해결할 수 있다. `asyncio.run()`은 이벤트 루프를 생성하는 `asyncio.get_loop()` 와 이벤트 루프를 실행하는 `loop.run_until()` 을 합친 메서드이다. `asyncio.run()` 이 실행되는 시점이 비동기 프로그램의 시작점(엔트리 포인트)이 되며 이를 통해 일반 함수에서 코루틴을 호출할 수 있다.

![][https://file.notion.so/f/f/0250db12-343d-4616-9db4-e97f91b5feba/131a674d-4fad-4957-9938-d8255560e826/Untitled.png?id=39a870a8-f6cc-4338-a914-aa16dbbf06e8&table=block&spaceId=0250db12-343d-4616-9db4-e97f91b5feba&expirationTimestamp=1707897600000&signature=oeMDvBCkw03ERkMnRqCNS1-eJRoUFPzzZJGMhjlYM2A&downloadName=Untitled.png]

___

### 파이썬의 이벤트 루프

우리는 앞서 [[이벤트 루프]] 에서 이벤트 루프란 이벤트를 감지하고 이에 따른 적절한 처리를 수행하는 하나의 디자인 패턴이라는 것을 학습했다. 파이썬에서의 이벤트 루프도 크게 다르지 않다. 준비된 코루틴을 재개하고 대기가 필요한 코루틴들을 스케줄링하는 작업을 이벤트 루프가 담당한다.

**즉, 이벤트 루프는 무한 반복하며 코루틴을 하나 씩 실행 시키는 비동기 방식의 실행 주체**이다.

<span class="red red-bg"><b>실행 중인 코루틴에서 await하면 해당 코루틴은 이벤트 루프로 실행 권한을 양도한다.</b></span> 권한을 양도 받으면 이벤트 루프는 대기 상태인 태스크들을 순회하며 태스크의 상태를 구분하는 작업을 수행한다. 
이벤트 루프는 태스크의 상태를 준비, 대기로 구분하는데 준비 상태는 실행 준비가 완료된 태스크를 의미하고 대기 상태는 IO 작업이 계속 진행중인 태스크를 의미한다.

대기 상태의 테스크는 대기 작업이 완료될 때 까지 상태가 보존 됐다가 대기가 끝나면 다시금 실행된다. 이를 위해선 **테스크의 대기가 끝났다는 것을 확인하는 작업이 필요한데 이벤트 루프가 테스크를 반복적으로 호출하는 방식으로 이를 수행**한다. 따라서 **대기 작업이 끝났는지를 파악하고 스케줄링 하기 위해선 이벤트 루프에 작업을 등록해야만 한다.**

테스크의 상태 구분이 완료되면 이벤트 루프는 준비 상태인 테스크 중 하나를 선택해 실행한다. 이후 위의 과정이 반복된다.

<span class="red red-bg"><b>테스크는 인위적으로 명시한 부분에서만 권한을 이벤트 루프에 넘겨준다. 이 부분은 자동적으로 권한을 뺐는 멀티 스레딩과 차이가 존재한다.</b></span>

아래는 제네레이터를 활용해 구현한 이벤트 루프의 골조이다. 제네레이터에 매개변수를 전달하며 각 서브루틴을 교체로 실행하는 방식으로 동작하는 것을 확인할 수 있다. 이 아이디어를 발전 시킨 것이 async/await가 이다.

```python
def coroutine1():
    print('C1: Start')
    yield #권한 반환
    print('C1: hello')
    yield
    print('C1: end')

def coroutine2():
    print('C2: Start')
    yield #권한 반환
    print('C2: hello')
    yield
    print('C2: end')

# 이벤트 루프 실행
def run(cors):
    while cors:
        for co in [c1,c2]:
            co.send(None) #제네레이터를 실행

# 코루틴 객체 생성 => asyncio def
c1 = coroutine1()
c2 = coroutine2()

task = [c1, c2]
```

![](https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Fblog.kakaocdn.net%2Fdn%2FooHgS%2Fbtq8BDw8qQJ%2Ff2aNgf3naawkZGgcPC81QK%2Fimg.png)

___
### Task

**Task 객체는 코루틴을 감싸는 객체로 코루틴이 비동기적으로 동작하기 위한 핸들러이다.**

==**코루틴들을 await로 실행하면 비동기적으로 작동하지 않고 동기적으로 작동한다.**== 이는 await 구문이 단순히 코루틴을 실행하는 기능만을 가지고 있기 때문이다. 따라서 **비동기적 사용을 위해선 스케줄링이 필요하고 이를 Task가 수행한다**. ==**Task는 코루틴들을 비동기적으로 실행하기 위해 스케줄을 관리해주는 하나의 핸들러**==라고 보면 된다.

```python
import asyncio
import time
async def timer(n):
    print(f"{n} wait")
    await asyncio.sleep(n)
    print(f"{n} wait finish")

async def main():
    print("start:", time.time())
    await timer(1)
    await timer(2)
    print("end:", time.time())

asyncio.run(main())
```

위 코드는 이렇게 작동할 것 처럼 보여진다.
- **timer(1)→ wait → timer(2) → wait→ timer(1) end → timer(2) end**

하지만 실상 이렇게 동작한다.
- **timer(1)→ wait → timer(1) end → timer(2) → wait → timer(2) end**

이러한 현상이 발생하는 이유는 앞서 말했듯이 **이벤트 루프가 실행할 작업이 스케줄링 돼있지 않기 때문**이다. 위 코드에서 이벤트 루프는 main()만을 스케줄링 한다. 따라서 main에서 await가 발생한다 하더라도 추후 실행할 작업이 존재하지 않기 때문에 무작정 대기하는 일이 발생한다.

따라서 **여러 코루틴을 비동기적으로 동작시키기 위해선 이벤트 루프가 코루틴들을 스케줄링 할 수 있어야 하고 이를 가능하게 코루틴을 감싸는 객체가 바로 Task**이다.

**Task는 생성과 동시에 코루틴을 이벤트 루프에 등록(스케줄링) 시킨다.** Task를 통해 특정 코루틴의 결과를 기다릴 수도 있고 코루틴을 중단하거나 실행 할 수도 있다. **Task는 이벤트 루프에 등록된 코루틴을 컨트롤하는 핸들이다.**

> ==**It's an asyncio construct that tracks execution of a coroutine in a concrete event loop. When you call create_task, you submit a coroutine for execution and receive back a handle.**== You can await this handle when you actually need the result, or you can never await it, if you don't care about the result. This handle is the task, and it inherits from Future, which makes it awaitable and also provides the lower-level callback-based interface, such as add_done_callback.

따라서 위의 코드를 이상적인 비동기-논블락킹으로 동작 시키고 싶다면 task 객체로 생성해 이벤트 루프에 등록해야 한다. 또한 이러한 과정은 `create_task()`를 활용해 한번에 진행할 수 있다.

```python
async def main():
    print("start:", time.time())
    futures = [asyncio.create_task(timer(i)) for i in range(2)] #테스크를 반환
    for f in futures:
        await f #테스크의 완료까지 대기
    print("end:", time.time())
```

___
### Futures

퓨처는 어떠한 작업의 실행 상태 및 결과를 저장하는 객체이다. 여기서 말하는 실행 상태는 **작업이 대기 중인지 취소 됐는지 혹은 완료** 됐는지를 의미한다. 작업의 대기는 PENDING 완료는 FINISHED와 CANCELED 2개로 구분된다. 퓨처에 저장되는 실행결과는 작업의 반환 값 혹은 예외이며 예외가 발생하더라도 상태는 FINISHED가 된다.

퓨처는 JS의 promise와 흡사한 개념처럼 보이나 동일하진 않다 **퓨처는 실행 상태와 결과를 저장만 할 뿐 실질적으로 실행을 개시하지는 않기** 때문이다.

퓨처의 주요 메서드로는 add_done_callback() 함수가 존재하며 이는 퓨처의 작업이 완료되면 바로 호출하는 콜백 함수를 등록하는 메서드이다. 등록된 콜백은 테스크가 완료되지마자 곧 바로 실행된다.

파이썬에는 두개의 future가 존재하는데 `concurrent.futures`와 `asyncio.futures`이다. 두 futures는 호환되지 않으며 다른 목적을 갖는다. 전자의 futures가 우선 탄생됐으며 후자는 전자를 흉내 내고자 만들어졌다.

#### [concurrent.futures](https://docs.python.org/ko/3/library/concurrent.futures.html#concurrent.futures.Future)
**futures 모듈은 멀티 쓰레드 환경에서 각 쓰레드의 값들을 전달하고 넘겨받는 과정을 손쉽게 만들기 위해 탄생했다.** 이전의 멀티 쓰레드 모듈은 전부 C를 기반으로 작성 됐기 때문에 전통적인 C와 동일하게 큐나 공유 변수 등을 통해 쓰레드간 통신을 진행 해야만 했다.

퓨처 객체는 언제가 어떠한 쓰레드에서 처리 됨을 보장받으며 사용자는 이에 콜백을 추가하거나 취소할 수 있다. 자바스크립트의 Promise와 흡사한 객체라 생각할 수 있다.

퓨처의 실행과 스케줄링은 사용자가 신경쓸 필요 없이 모두 Executor(실행자) 클래스에서 도맡아 진행한다. 따라서 사용자는 적절한 실행자를 선택해 해당 실행자에 진행하고 싶은 작업을 등록만 하면 된다.

등록은 `submit()`을 통해 진행할 수 있으며 결과 값으로 퓨처 객체가 반환 된다. 퓨처 객체는 이때 완료를 보장 하지는 않으므로 별도의 `wait()`나 `as_complted()` 또는 `result()` 같은 함수로 퓨처의 작업이 완료될 때까지 대기해야 한다.

```python
from concurrent.futures import ThreadPoolExecutor as PE
from urllib.request import urlopen
import concurrent.futures
URLS = [
    'http://www.foxnews.com',
    'http://www.cnn.com',
    'http://europe.wsj.com',
    'http://www.bbc.co.uk']
def load_url(url):
    conn = urlopen(url, timeout=1)
    return conn.read()
def main():
    with PE() as exe: #멀티 쓰레드 실행자
        fs = {exe.submit(load_url, url): url for url in URLS} #퓨처 딕셔너리
        for f in concurrent.futures.as_completed(fs):
            try:
                url = fs[f]
                data = f.result()
                print(f'[{url}] has {len(data)} characters.')
            except Exception:
                print(f'Fail to open : {url}')
if __name__ == '__main__':
    main()
```

위의 코드를 보면 멀티 쓰레드를 활용해 작업을 진행하고 각 쓰레드에서 실행된 작업의 결과를 하나의 쓰레드에서 손쉽게 획득하는 것을 확인할 수 있다. 

> [!info]
> **해당 퓨처스는 멀티 쓰레드 환경에서 비동기 처리를 위해 고안된 방식이다. 따라서 async/await와 연관이 없고 None-awaitable이다.**

#### 단일 쓰레드 환경에서의 futures의 탄생
> `Future` 개념의 도입은 스레드를 관리하고, 다른 스레드에서 돌아가는 작업에 대해서 리턴을 동기화하는 등의 작업들이 매우 골치아팠던 것을 그 자체를 객체로 래핑하면서 매우 우아하게 처리할 수 있었다. 이는 결국 비선형적인 제어 흐름과 관계된 코드를 작성하는 것이 더 이상 너저분한 작업이 아닐 수 있다는 가능성을 보였다.
> 
> **다중 스레드 및 다중 프로세스에 대해서 `Future`를 적용하는 것이 성공적이었다면, 이는 단일 스레드에 대해서도 비동기 non-blocking 코드를 작성하는데에 동일한 `Future` 개념을 도입할 수 있지 않을까하는 것으로 아이디어가 옮겨갔다.**
> 
___
### Corutine, Future, Task 한눈에 보기

==**asyncio 모듈에 한해서 위 3개의 객체는 전부 awaitable하므로 이벤트 루프에 권한을 양도하고 다른 작업을 처리하는 것이 가능하다.**==

![](https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Fblog.kakaocdn.net%2Fdn%2FdITHrN%2Fbtq8BmWfbIr%2FKKjtyW2gLmtEx5dE7pJkBk%2Fimg.png)

중간에 나왔다가 들어갈 수 있는 서브 루틴의 구조를 코루틴이 구현한다. 실행중인 코루틴의 결과는 퓨처를 활용해 표현한다. 코루틴은 그 자체가 상태를 가질 수 없기 때문에 작업의 중단과 재개를 위해서 테스크를 활용한다.

![](https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Fblog.kakaocdn.net%2Fdn%2Fed6uEg%2Fbtrt1JocVB7%2FWbjhG6smnQ4PCu3zH2hou1%2Fimg.png)

>[!info]
>**테스크가 실행하는 작업이 코루틴이고 코루틴의 실행 상태는 퓨처로 나타내진다.** 

___
### 파이썬 async의 실행 흐름 자세하게

이벤트 루프를 실행하면 전달 받은 코루틴을 통해 테스크 객체가 만들어지고, 이 과정에서 테스크의 실행이 이벤트 루프에 스케줄링 된다. 이때 **실행이란 `__step()`을 통해 테스크에 있는 코루틴을 실행하는 것**을 말한다.

`__step()`이 실행되고 나면 해당 메서드는 테스크 객체의 `__coro` 필드에 저장된 `send()`를 실행해 코루틴을 실행하는 역할을 수행한다. 이제 이 코루틴을 시작점으로 await 키워드를 마주칠 때마다 새로운 코루틴을 실행하게 되면서 코루틴 체인이 구축된다.

만약 **코루틴의 반환 값이 퓨처일 경우 테스크는 퓨처 객체를 생성해 반환 값과 바인딩**한다. 이후 바인딩한 퓨처 객체를 await하는데 `__await__()`는 자기 자신을 반환하는 함수이므로 반환 값이 코루틴 체인을 타고 `__step() ` 까지 반환될 것이다.

==테스크 객체는 `__step()`으로 yield된 퓨처 객체를 전달 받으면 이를 자신의 `__fut_waiter` 필드에 바인딩한다.== 이후 퓨처 객체가 완료 될 때 테스크를 재실행하기 위해 `__step()` 함수를 이벤트 루프에 등록한다. 이 작업을 수행하고 나면 ==테스크는 자신의 작업은 중단하고 권한을 이벤트 루프로 양도한다.==

이후 해당 테스크는 `__fut_waiter` 필드에 값이 존재해 이벤트 루프의 실행 대상에서 제외되고 이벤트 루프는 다른 테스크를 실행한다.

이후 이벤트 루프가 등록해뒀던 퓨처 작업의 완료를 확인하면  콜백으로 인해 `__step()` 함수가 재실행된다.  함수는 테스크의 `__fut_waiter`의 값을 비우고 `send`를 활용해 코루틴을 다시 호출한다. 코루틴은 이전의 중단 부분 이었던 `__await__()` 이후의 코드 부터 순차적으로 실행된다. 

아래 코드를 실행하면 실제로 변화하는 `__fut_waiter`값을 확인할 수 있다.
```python
async def main():
    print("start:", time.time())
    tasks = [asyncio.create_task(timer(i)) for i in range(2)]
    await asyncio.sleep(1) #등록한 코루틴을 실행하기 위해 권한 반납.
    for t in tasks:
        print(t._fut_waiter)
        # await t

    await asyncio.gather(*tasks)
    print("end:", time.time())
```

#### 코루틴의 종료
이렇게 실행을 진행하다보면 언젠가 return을 통해 종료되는 코루틴을 만나게 된다. 코루틴이 종료되면 테스크의 **`__step()`은 StopIteration을 만나게 되고 이로 인해 종료**하게 된다. StopIteration이 반환되면 코루틴의 반환 값을 테스크의 result 필드에 저장한다. 이후 테스크는 더 이상 스케줄링 되지 않고 완전히 종료된다.

___
### await 주의점

#### await가 항상 권한을 양도하진 않는다
<b><u>await를 실행할 경우 권한을 항상 이벤트 루프에 양도하진 않는다. 루프에 권한을 양도하는 경우는 코루틴의 실행 결과로 퓨처 객체가 반환되는 경우 만이다.</u></b> 아래의 예제를 보자.

```python
import asyncio


async def aprint(s):
    print(s)


async def forever(s):
    while True:
        await aprint(s)
        # await asyncio.sleep(0)


async def main():
    await asyncio.gather(forever("a"), forever("b"))


asyncio.run(main())
```

`gather()`에 코루틴을 등록할 경우 자동으로 테스크 객체로 래핑이 되기 때문에 이벤트 루프에는 2개의 테스크가 존재하게 된다. 이에 따라 `forever(a)`가 실행중 await를 만난다면 `forever(b)`로 스위칭이 발생해야 하지만, 코드를 실행해보면 스위칭 없이 a만 계속해서 출력되는 것을 확인할 수 있다.

이와 같은 현상이 발생하는 이유는 **코루틴의 실행 결과가 퓨처 객체가 아니기 때문에 `forever(a)` 가 실행 권한을 루프로 반환하지 않고 계속해서 사용하기 때문**이다. 따라서 강제로 스위칭을 발생시키고 싶다면 해당 코루틴의 결과가 퓨처 객체로 반환될 수 있게 인위적으로 `sleep`을 추가해주면 된다.

#### await coro != await task
코루틴을 await하는 것과 task를 await하는 것은 다르다. 코루틴을 await하면 곧장 코루틴을 실행하는 반면 task를 await하면 이벤트 루프에 해당 테스크를 등록하고 이벤트 루프로 실행권한을 넘긴다. 따라서 코루틴을 곧장 await할 경우 바로 실행되지만, 태스크의 경우 곧장 실행되지 않을 수 있다.

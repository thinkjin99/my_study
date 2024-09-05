### 출처
* https://bentist.tistory.com/89?category=1010744 (비동기 개요)
* https://it-eldorado.tistory.com/159 (테스크의 스케줄링 방법)
* https://tech.buzzvil.com/blog/asyncio-no-1-coroutine-and-eventloop/ (실습 예제)
* https://soooprmx.com/asyncio/ (테스크, 코루틴, 퓨처
* https://stackoverflow.com/questions/49005651/how-does-asyncio-actually-work (파이썬의 asyncio)
____
### 개요
* [[#asyncio]]
* [[#코루틴]]
* [[#이벤트 루프]]
* [[#asyncio VS 멀티 쓰레드]]
* [[#Task]]
* [[#Futures]]
* [[#Corutine, Future, Task 한눈에 보기]]
* [[#파이썬 async의 실행 흐름 자세하게]]
* [[#await 주의점]]
___
### asyncio

asyncio는 파이썬 3.4 부터 등장한 모듈로 파이썬에서의 비동기 작업을 손쉽게 가능하게 해준다. asyncio는 이벤트 루프와 코루틴을 활용해 IO 작업의 속도를 개선하고 동시성을 향상 시키는 것을 목적으로 한다.

___
### 코루틴

**코루틴(Coroutine)이란 특정 시점에 자신의 실행과 관련된 상태를 어딘가에 저장한 뒤 실행을 중단하고, 나중에 그 상태를 복원하여 실행을 재개할 수 있는 서브 루틴**을 의미한다.

코루틴이 await, yield 구문 등을 통해 인위적으로 다른 곳으로 ==**권한을 양도하면 자신의 실행과 관련된 상태를 어딘가에 저장하고 실행을 중지**==한다. 이후 **다시금 코루틴을 실행하면 멈췄던 부분에서 다시금 상태를 복원해 작업을 진행**한다. 

코루틴은 제네레이터와 혼동되는 경우가 잦은데 이는 코루틴의 기반이 제네레이터에 존재하고 있기 때문이다. 제네레이터의 주 목적은 값을 생성해 호출 부로 전달하는 것이다. 내부 -> 외부 일방으로 동작하는 흐름을 주로 갖는데 코루틴의 경우 외부에서 인자를 전달 받고 내부의 인자를 외부로 전달하는 양방으로 동작하는 경우가 많다.

물론 제네레이터 역시 `send()` 등의 메서드를 활용하면 외부로 부터 값을 전달 받는 것이 가능하다. 하지만 일반 제네레이터의 경우 이벤트 루프에 의한 스케줄링이 불가능하고 비동기 작업을 위해 필요한 속성들이 정의돼 있지 않다. 

<b><u>반면 코루틴은 테스크로 래핑할 경우 이벤트 루프에서 스케줄링이 가능하고 비동기 동작에 최적화 돼 있다. 즉 , 코루틴은 비동기 처리에 특화된 제네레이터라고 볼 수 있다.</u></b>

==**코루틴 객체를 만들기 위해서는 async 키워드를 서브 루틴 앞에 추가하면 된다. 이때 주의할 점은 코루틴은 함수와 달리 호출을 해도 함수 내부의 코드가 동작하지 않고 코루틴 객체를 반환 한다는 것이다.**== 함수 내부의 코드를 동작시켜 결과 값을 얻기 위해서는 await 구문을 활용해야 한다.

짚고 넘어가야 할 점은 **await 구문을 코루틴 함수 내부에서만 사용할 수 있다**는 것이다. 함수 외부나 일반 함수에서 await를 통해 코루틴을 실행하는 것은 불가하다. 따라서 **코루틴을 만들고 이를 실행하기 위해서는 실행을 위한 또 다른 코루틴이 필요** 해진다. 이 문제는 꼬리에 꼬리를 물며 계속 연결되므로 이를 해결하기 위해서는 코루틴의 시작을 일반 함수나 함수 외부에서 실행 할 수 있게 해줘야 한다.

`asyncio.run()`을 통해 이 문제를 해결할 수 있다. `asyncio.run()`은 이벤트 루프를 생성하는 `asyncio.get_loop()` 와 이벤트 루프를 실행하는`loop.run_until()` 을 합친 메서드이다. `asyncio.run` **이 실행되는 시점이 비동기 프로그램의 시작점(엔트리 포인트)이 되며 이를 통해 일반 함수에서 코루틴을 호출할 수 있다.**

![500](https://obs3dian.s3.ap-northeast-2.amazonaws.com/asyncio%20/%20Pasted%20image%2020240904153031.png)

___
### 이벤트 루프

[[이벤트 루프]]에 대한 개념은 링크를 참조하자. 아래에서는 `asyncio`에서의 이벤트 루프의 실질적 동작을 설명한다. 이벤트 루프안에는 코루틴이 스케줄링이 가능한 `Task` 객체로 래핑돼 존재한다. **이벤트 루프는 각 `Task`들의 상태를 확인하고 실행 가능한 테스크들을 실행하는 작업을 수행한다.** 

테스크는 이벤트 루프로 부터 실행 권한을 부여 받아 코드의 실행을 지속하다 `await` 키워드를 마주치면 실행 제어 흐름을 이벤트 루프에게 돌려준다. `await`는 특정한 퓨쳐 객체를 대기하는 행위로 소켓 IO를 예시로 들면 소켓에 데이터가 전달되는 이벤트가 발생할 때까지 코루틴의 실행을 블락하라는 의미가 된다. 

`await`를 통한 퓨처 객체를 전달 받으면 테스크는 이를 `_futwaiter`라는 테스크 객체 필드에 저장한다. 이후 해당 퓨쳐 객체의 `add_done_callback()` 메서드를 통해 **IO 작업 완료에 대한 콜백으로 자신을 이벤트 루프에 추가하는 작업을 생성한다.** 이렇게 하면 자동으로 퓨쳐 객체의 IO작업이 완료된 후 루프를 통한 코루틴의 재 실행을 보장 받을 수 있다.

이후 테스크 객체는 실행 제어 흐름을 이벤트 루프로 양도한다. 흐름을 돌려주는 까닭은 단순한데 **이벤트 루프가 각 테스크들을 모니터링하고 스케쥴링하는 주체이기 때문이다. 또한 테스크들의 상태를 업데이트하고 스케줄링을 진행하기 위해서는 다른 테스크들을 멈추고 실행 제어 흐름을 획득해야 하기 때문이다.**

흐름을 받은 이벤트 루프는 먼저 실행 가능한 테스크들을 실행하는 작업을 수행한다. **만약 실행할 테스크가 존재하지 않다면, I/O Multiplexing 시스템 콜을 활용해 등록된 퓨처 객체들 중 I/O 이벤트가 완료된 객체를 찾고, 위에서 언급한 콜백이 실행되게 한다.** 

이제 이벤트 루프가 실행이 예약된 태스크를 실제로 실행시키는 과정을 살펴보자. **태스크의 실행이란 곧 해당 태스크 객체의** **__step()** **메소드를 호출하는 것**을 의미한다. 이 메소드는 먼저 자기 자신(태스크 객체)과 **퓨처 객체의 바인딩을 해제**하여 더 이상 기다리는 퓨처 객체가 없음을 나타내고, 다시 **자신의 코루틴 객체에 대해** **send()** **메소드를 호출하여 해당 코루틴의 실행을 재개**한다. 그러면 다시 해당 퓨처 객체의 __await__() 메소드에서 실행이 중단되었던 부분(자기 자신을 yield하는 부분)까지 가게 된다.

`__await()__` 메소드로까지 돌아왔을 때, 만약 I/O 관련 코루틴 때문에 기다리고 있었던 거라면 이제는 해당 소켓에 대해 데이터를 읽거나 쓸 준비가 되었다는 것이므로 해당 소켓(자기 자신에 바인딩되어 있음)에 대해 데이터를 읽거나 쓴 다음 그 값을 `return` 할 것이다.

![](https://obs3dian.s3.ap-northeast-2.amazonaws.com/asyncio%20/%20Pasted%20image%2020240904163311.png)

___
### asyncio VS 멀티 쓰레드

파이썬의 asyncio를 사용하는 방식과 멀티 쓰레드 방식의 차이점을 파악해보자. 

asyncio(이하부터 비동기로 칭한다)를 활용하면 싱글 쓰레드 환경에서 이벤트 루프에게 스케줄링을 맡기는 형태로 동작한다. **IO를 제외하고 하나의 코루틴 만이 실행되며 이로 인해 경쟁 조건과 같은 문제는 발생하지 않는다.** 추가적으로 **asyncio는 await를 기준으로 실행 흐름을 이벤트 루프에 명시적으로 반환하기 때문에 작업의 스위칭 시점을 유저가 명시할 수 있다.** 아래의 코드를 확인해보자.

```python
import asyncio
import time
  

async def foo():
	print("sleep")
	time.sleep(5)
	print("sleep end")
	#await asyncio.sleep(3) await를 줄 경우엔 foo, foo2가 교차로 진행된다.


async def foo2():
    for i in range(3):
        print(f"{i} is finished")


async def main():
    task1 = asyncio.create_task(foo())
    task2 = asyncio.create_task(foo2())
    await asyncio.gather(task1, task2)


asyncio.run(main())


sleep
sleep end
0 is finished
1 is finished
2 is finished
```

asyncio를 활용해 위와 같이 코드를 작성할 경우 await를 통해 실행 흐름이 이벤트 루프로 넘어오지 않기 때문에 `foo()`가 전부 실행돼야 `foo2()`의 실행이 진행된다. 이는 멀티 쓰레딩과의 차이점인데 **멀티 쓰레딩을 활용할 경우 운영체제가 스케쥴링을 진행하기 때문에 하나의 쓰레드가 몇초씩 제어 흐름을 확보하고 있는게 불가능하다.** 아래 코드를 보자.

```python
import threading
import time


def foo():
    print("sleep start")
    time.sleep(5)
    print("sleep end")


def foo2():
    for i in range(5):
        print(f"{i} is finished")


count_thread = threading.Thread(target=foo)
count_thread.start()

foo2()

count_thread.join(0)

sleep start
0 is finished
1 is finished
2 is finished
3 is finished
4 is finished
sleep end

```

멀티 쓰레딩을 활용할 경우 신규 쓰레드에서 일정 시간 대기를 진행하다. OS 스케쥴러에 의해 강제로 스케쥴링이 발생해 `foo2`가 실행되는 것을 확인할 수 있다. **이에 따라 멀티 쓰레딩을 활용할 경우 특정 쓰레드가 영원히 실행되지 않는 현상은 거의 발생하지 않는다.** `await`와 같은 키워드를 명시하지 않아도 **자동으로 쓰레드 간의 스위칭이 발생하며 실행되는 것은 적절해보이나 의도치 않은 컨텍스트 스위칭을 야기해 잘못된 결과를 도출할 수도 있다.** 

이외에도 멀티 쓰레딩 방식은 각각의 쓰레드가 별도의 스택 영역을 활용할 수 있다는 점이 존재한다. 

정리를 해보자. 비동기 방식은 단일 쓰레드로 많은 양의 IO를 처리하고자 고안된 방법이다. 단일 쓰레드이기에 컨텍스트 스위칭이 없고 락이나 경쟁조건 등을 고려할 필요가 존재하지 않는다. 또한 쓰레드를 생성할 필요가 없기에 메모리 소모도 적고 더욱 대용량의 작업을 처리하는데 유리하다. 유저가 직접 스케쥴링을 진행할 수 있으며 이를 통해 효율적인 스위칭 또한 가능하다.

멀티 쓰레딩의 경우 IO 작업은 동시에 진행할 수 있지만 파이썬의 경우 GIL로 인해 CPU 바운드 작업에선 쓸모가 없다. 심지어 멀티 IO를 위해 멀티 쓰레드를 활용한다고 하더라도 쓰레드는 생성 비용이 비싸고 수도 제한적이고 공유 자원으로 인한 락과 같은 문제도 해결해야 한다. 이 경우 도대체 멀티 쓰레드가 비동기에 비해 갖는 장점이 뭘까?

#### 파이썬에서 쓰레드가 비동기보다 더 나은게 뭐야
동시성과 자원으로 비교하면 쓰레드가 이길 요소가 잘 없다. 쓰레드를 사용할 때 얻을 수 있는 몇가지 장점은 다음과 같다.

* **C확장과 사용해 GIL을 제거하고 사용하면 병렬 작업이 가능하다**
* **동기로 동작해 실행흐름 추적이 쉽다.**
* **기존의 블락킹 코드 위에서 동작해야 할 때 유용하다**
* **스택 영역 분리를 통한 독립성 확보가 가능하다**
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

퓨처는 논 블럭 함수의 반환 값으로 ==**언제가 될지는 모르지만 언젠가 완료되는 작업의 결과와 상태를 담고 있는 객체이다.**== 퓨처 객체는 특정한 작업이 마무리 될 때까지 값을 요청한 측의 흐름을 블락하고 다른 작업들을 우선 처리하는 것을 가능하게 해준다. 이에 따라 만약 퓨처 객체의 결과 값을 얻기 위해 `result()`나 `await`를 사용할 경우 퓨쳐 객체의 작업이 완료될 때까지 실행 흐름이 블락된다.

퓨쳐의 상태에는 작업의 진행상태가 저장되며, 대기는 PENDING 완료는 FINISHED와 CANCELED 2개로 구분된다. 퓨처에 저장되는 실행결과는 작업의 반환 값 혹은 예외이며 예외가 발생하더라도 상태는 FINISHED가 된다.

퓨처는 JS의 promise와 흡사한 개념처럼 보이나 동일하진 않다. **퓨처는 실행 상태와 결과를 저장만 할 뿐 실질적으로 실행을 개시하지는 않기** 때문이다.

퓨처의 주요 메서드로는 add_done_callback() 함수가 존재하며 이는 퓨처의 작업이 완료되면 바로 호출하는 콜백 함수를 등록하는 메서드이다. 등록된 콜백은 테스크가 완료 되지마자 곧장 실행된다.

파이썬에는 두개의 future가 존재하는데 `concurrent.futures`와 `asyncio.futures`이다. 두 futures는 호환되지 않으며 다른 목적을 갖는다. 전자의 futures가 우선 탄생됐으며 후자는 전자를 흉내 내고자 만들어졌다.
#### [concurrent.futures](https://docs.python.org/ko/3/library/concurrent.futures.html#concurrent.futures.Future)
**futures 모듈은 멀티 쓰레드 환경에서 각 쓰레드의 값들을 전달하고 넘겨받는 과정을 손쉽게 만들기 위해 탄생했다.** 이전의 멀티 쓰레드 모듈은 전부 C를 기반으로 작성 됐기 때문에 전통적인 C와 동일하게 큐나 공유 변수 등을 통해 쓰레드간 통신을 진행 해야만 했다.

퓨처 객체는 언젠가 어떠한 쓰레드에서 처리 됨을 보장받으며 사용자는 이에 콜백을 추가하거나 취소할 수 있다. 퓨처의 실행과 스케줄링은 사용자가 신경쓸 필요 없이 모두 Executor(실행자) 클래스에서 도맡아 진행한다. 따라서 사용자는 적절한 실행자를 선택해 해당 실행자에 진행하고 싶은 작업을 등록만 하면 된다.

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
> `asyncio.Future`  쓰레드 동기화 등의 골치 아팠던 작업들을 단일 쓰레드 환경에서 우아하게 처리하고자 하는 욕구에서 시작 됐다.
> 
> **다중 스레드 및 다중 프로세스에 대해서 `Future`를 적용하는 것이 성공적이었기에, 단일 스레드에서도 비동기 non-blocking 코드 작성을 위한 `Future` 개념을 도입할 수 있지 않을까라는 발상이 생겨났다**
___
### Corutine, Future, Task 한눈에 보기

==**asyncio 모듈에 한해서 위 3개의 객체는 전부 awaitable하므로 이벤트 루프에 권한을 양도하고 다른 작업을 처리하는 것이 가능하다.**==

![](https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Fblog.kakaocdn.net%2Fdn%2FdITHrN%2Fbtq8BmWfbIr%2FKKjtyW2gLmtEx5dE7pJkBk%2Fimg.png)

중간에 나왔다가 들어갈 수 있는 서브 루틴의 구조를 코루틴이 구현한다. 실행중인 코루틴의 결과는 퓨처를 활용해 표현한다. 코루틴은 그 자체가 상태를 가질 수 없기 때문에 작업의 중단과 재개를 위해서 테스크를 활용한다.

![](https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Fblog.kakaocdn.net%2Fdn%2Fed6uEg%2Fbtrt1JocVB7%2FWbjhG6smnQ4PCu3zH2hou1%2Fimg.png)

>[!info]
>**테스크가 실행하는 작업이 코루틴이고 코루틴에서 발생하는 대기가 필요한 작업들은 퓨처로 표현된다.** 

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

이와 같은 현상이 발생하는 이유는 **코루틴의 실행 결과가 퓨처 객체가 아니기 때문에 `forever(a)` 가 실행 권한을 루프로 반환하지 않고 계속해서 사용하기 때문**이다. 따라서 강제로 스위칭을 발생시키고 싶다면 해당 코루틴의 결과가 퓨처 객체로 반환될 수 있게 인위적으로 `sleep`을 추가해야 한다.

#### await coro != await task
코루틴을 await하는 것과 task를 await하는 것은 다르다. 코루틴을 await하면 곧장 코루틴을 실행하는 반면 task를 await하면 이벤트 루프에 해당 테스크를 등록하고 이벤트 루프로 실행권한을 넘긴다. 따라서 코루틴을 곧장 await할 경우 바로 실행되지만, 태스크의 경우 곧장 실행되지 않을 수 있다.

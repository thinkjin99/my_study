### 출처
* https://velog.io/@jhlee508/%EC%9A%B4%EC%98%81%EC%B2%B4%EC%A0%9C-Message-Queue-IPC (message queue 실습 예제)
___
### 개요
[[#Independent & Cooperative]]
[[#Inter Process Communication]]
[[#Shared Memory VS Message Passing]]
[[#Message Queue Example]]
___
### Independent & Cooperative

* **Independent**
	**독립 프로세스는 다른 프로세스와 독립적으로 실행되며 서로 영향을 미치지 않는다.** 프로세스의 상태는 다른 프로세스에 의해 바뀌지 않으며 독립 프로세스는 단일로 실행 가능하기 때문에 재시작과 종료가 단순하다.

* **Cooperating**
	**공유 프로세스는 프로세스 상호간 통신이 가능한 프로세스로 공유 가능한 구조로 설계해야 한다.** 공유 프로세스는 데이터 공유가 가능하며 서로의 상태에 영향을 끼칠 수 있다. 공유 프로세스로 채택하면 연산을 나눠 빠른 더욱 빠른 연산을 진행하거나 기능 단위를 분할해 개발 가능하다는 이점이 존재한다. **공유 프로세스는 공유 자원을 사용하기 때문에 경쟁 조건이 발생하거나 방해가 발생할 수 있다. 따라서 동기화 작업이 필수적이다.**
___
### Inter Process Communication
IPC는 프로세스간 데이터 또는 메시지를 교환하고 상호 작용하는 방법을 의미한다. 크게 공유 메모리와 메시지 패싱 방식으로 구분된다. **이는 크게 두가지 패러다임을 말한 것이며 구현체로는 소켓,  파이프, 메시지 큐 등이 있다.**
#### Shared Memory
공유 메모리는 프로세스가 공통적으로 접근하는 메모리 영역을 사용하는 방법이다. <u><b>공유 메모리는 유저 영역에 할당 되기에 OS의 도움 없이 유저 영역에서 커뮤니케이션이 가능하다</b></u>. 공유 메모리 방식의 **핵심 이슈는 동기화로 공유된 메모리에 프로세스들이 동시 접근할 때 상호 배제를 적절히 구현해야 한다**.

* ***생산자 소비자 문제**
	**데이터를 생산하는 프로세스와 이를 소비하는 프로세스가 연속 동작할 때 발생**하는 문제이다. 생산자는 자신이 생산한 데이터가 곧바로 사용되지 못하는 상황을 고려해 버퍼에 생성한 데이터를 저장하고 소비자는 버퍼에서 데이터를 하나씩 꺼내 읽는다. **별개의 프로세스에서 공통의 버퍼에 접근 할 필요가 있는데 이를 공유 메모리 기법을 활용함으로써 해결할 수 있다.**

#### Message Passing
<u><b>각 프로세스가 메시지를 send,(), recv() 하는 방법을 활용해 두 프로세스간의 통신을 진행하는 방법을 의미한다 메시지는 커널을 통해 중계된다.</b></u>  
메시지 패싱 방식은 직접 전달과 간접 전달으로 구분된다. 직접 전달은 해당 프로세스에 직접적으로 메시지를 전달하는 방식이다. 간접 전달은 프로세스로 직접 메시지를 전달하지 않고 메시지를 공유하는 메시지 함(메시지 큐) 같은 프로세스를 생성해 해당 프로세스를 통해 메시지를 전달한다.

![[스크린샷 2023-10-18 오전 11.59.29.png]]
___
### Shared Memory VS Message Passing
* **Shared Memory**
	공유 메모리는 메모리를 직접 공유하는 방식이기 때문에 데이터 전달과 복사에 오버헤드가 존재하지 않는다. 하지만 **메모리를 공유하기 때문에 잘못 덮어 씌워질 문제가 존재하고, 공유 자원이기에 경쟁 조건이 발생할 수 있다**.

* **Message Passing**
	메시지 패싱은 **메시지 큐를 사용하기 때문에 동일한 소비자가 존재하는 상황이 아닐 경우 경쟁 조건이 발생할 일이 없다**. 또한 **메모리를 공유하는 방식이 아니기 때문에 안정성이 더 높다**. 하지만 메시지를 송, 수신하는 오버헤드가 발생하고 커널 영역을 거쳐야만 한다.
___
### Message Queue Example
메시지 큐를 간단하게 실험해 프로세스 간의 커뮤니케이션을 구현해 볼 수 있다.
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//meesage_queue_recv.c

// 메시지 구조체 정의
struct msg_buffer
{
    long msg_type;
    char message[100];
};

int main()
{
    // 메시지 큐 키 생성
    key_t key = ftok("progfile", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT); // create msg queue

    struct msg_buffer message;

    // 메시지 큐에서 메시지 수신
    if (msgrcv(msgid, &message, sizeof(message), 1, 0) != -1) //recv msg
        printf("메시지 수신: %s\n", message.message);
    return 0;
}

```
메시지를 수신할 큐를 생성한다. 큐를 생성한 후 큐에 전달되는 메시지를 소비해 출력 해준다. 
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// 메시지 구조체 정의
struct msg_buffer
{
    long msg_type;
    char message[100];
};

int main()
{
    // 메시지 큐 키 생성
    key_t key = ftok("progfile", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT); //open msg queue

    struct msg_buffer message;
    message.msg_type = 1;
    strcpy(message.message, "안녕, 메시지 큐!");

    // 메시지 큐에 메시지 전송

    msgsnd(msgid, &message, sizeof(message), 0); //send msg
    printf("메시지 전송: %s\n", message.message);

    return 0;
}
```
결과는 아래와 같이 서로 다른 프로세스에 메시지가 전달되는 것을 확인할 수 있다.
![[스크린샷 2023-10-18 오후 12.48.41.png]]
___
### Pipes
파이프는 IPC를 구현하는 방법 중 하나로 주로 메시지 패싱 방법을 활용한다. <u><b>파이프는 2개의 프로세스 사이 정보가 통하는 길을 생성해 만들어진 길을 통해서 정보를 주고 받을 수 있게 해준다.</b></u> 파이프는 메시지 패싱 방식을 따르기 때문에 메시지가 파이프를 타고 커널 영역을 거쳐 특정 프로세스에 전달되는 방식으로 동작한다.
* **Ordinary Pipe**
	일반적인 파이프는 생산자-소비자 패턴을 따르기 때문에 일방향으로 설계된다. 또한 프로세스는 무조건 부모-자식의 관계여야 한다. 부모-자식 관계가 아니면 해당 파이프를 활용할 수 없다.

* **Named Pipe**
<u><b>양방향 통신이 가능한 파이프로 관계에 관계 없이 다양한 프로세스가 해당 파이프에 접근 할 수 있다.</b></u>

![[스크린샷 2023-10-19 오전 11.43.16.png]]
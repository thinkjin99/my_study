### 출처

___
### 개요
* [[#Sync]]
* [[#Blocking and None-Blocking]]
* [[#Blocking IO and None-Blocking IO]]
* [[#Event Loop]]
* 
___
### Sync
<span class="red-bg"><b>동기와 비동기는 함수의 응답을 누가 신경을 쓰느냐에 따라 구분된다. 동기는 함수의 응답을 호출부에서 신경을 계속해서 쓰는 방식을 말하고 비동기는 신경을 쓰지 않는 방식을 말한다.</b></span>

동기의 경우 호출 부가 필수적으로 함수의 결과를 확인해야 한다. 이는 함수의 블락킹, 논블락킹 여부와 관계 없으며 함수의 완료 값을 받든 함수가 아직 실행 중이라는 상태 값을 얻던 동기는 함수의 결과를 호출 부에 반드시 전달해야 한다.

**Sync-Blocking**
동기-블락킹은 방식은 가장 전통적인 방식으로 다음의 구조로 동작한다. 파일을 읽는 함수를 실행한다 했을 때 우리가 가장 기본적인 형태로 읽기를 진행하면 다음의 이미지와 같이 동작한다. 
![][https://developer.ibm.com/developer/default/articles/l-async/images/figure2.gif]
<b><u>유저 어플리케이션은 커널 모드로 스위칭 되고 실행 흐름이 정지된다. 이후 커널 영역에서 read 시스템 콜을 호출하고 커널 쓰레드는 IO를 진행한다.</u></b> 다시 유저 영역으로 읽은 데이터를 전송해 준다. 전송이 끝난 후에 유저 영역은 다시 실행이 가능하다. 이는 가장 직관적인 방식이지만, IO가 발생하는 동안 CPU가 대기 상태로 유저 영역에서 블락 된다는 단점이 존재한다.

**Sync-Nonblocking**
동기-논블락킹은 파일을 읽는 동안 유저 어플리케이션의 흐름이 막히지 않는다. 유저 어플리케이션에서 read를 시도하면 커널 모드로 스위칭한 후 함수의 결과를 대기 없이 즉각 반환한다. <b><u>이때 읽기 작업은 완료 됐을 수도 되지 않았을 수도 있다.</u></b>
![][https://developer.ibm.com/developer/default/articles/l-async/images/figure3.gif]
 <span class="red-bg"><b>중요 부분은 블락킹이 발생하지 않는다는 점인데, 이전과 달리 read 시스템 콜이 즉각적으로 반환되기 때문에 IO 대기 없이 곧장 다른 작업을 수행할 수 있다. </b></span>하지만 값을 곧장 반환하기에 <b><u>데이터가 적절히 전달되지 않을 수 있으므로 IO 작업이 완료 됐는지 주기적으로 확인할 필요가 있다. 이를 위해 read를 반복 호출해 함수의 완료 여부를 확인하는 busy-waiting 이 발생해 오버헤드가 커지게 된다.</u></b>

**동기의 성질**
<span class="red-bg">동기의 핵심적인 성질은 결과를 호출부에 반환한다는 것이다. 함수를 호출한 호출부는 함수의 실행 결과를 전달 받는다. 즉 함수의 응답을 호출 부에서 신경 쓴다고 말할 수 있다.</span> 동기-블락킹의 경우 호출부에 함수의 실행 결과가 전달되고 동기-논블락킹의 경우 함수의 실행 상태 혹은 결과가 전달된다. 중요한 것은 함수를 호출하고 이에 대한 응답을 호출 부가 받는 방식으로 동작한다는 것이다.
____
### Sync Example
``` c
ssize_t read_file(int fd, char *buffer)
{
    ssize_t bytesRead = read(fd, buffer, 32);
    if (bytesRead == -1)
    {
        perror("Failed to read file");
    }
    printf("Read %d is finished!\n", fd);
    return bytesRead;
}
```
가장 기본적인 동기-블락킹 방식의 함수이다. <span class="red-bg">동기-블락킹 함수의 특징은 호출 순서대로 완료 된다는 점으로  완료 순서를 보장하는 작업에 주로 사용한다. </span> 또한 해당 방식을 사용할 경우 실행 흐름이 직관적으로 보이기 때문에 디버깅 작업 또한 쉽게 진행할 수 있다.
종종 실행 순서가 완료 순서를 보장하는 in-order의 성질이 동기로 인해 보장되는 줄 착각하는 경우가 있는데, <b><u>동기 만으로는 순서를 보장하지 못한다. 순서를 보장하는 것은 동기-블락킹이 같이 사용 됐을 경우 만이다.</u></b>

```c
ssize_t none_block_read(int fd, char *buffer)
{
    // Set the file descriptor to non-blocking mode
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    ssize_t bytesRead = read_file(fd, buffer);
    if (bytesRead == -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            // Non-blocking read returned with no data available
            printf("Still reading...\n");
        }
    }
    return bytesRead;
}

int sync_none_block_test(int fd, int fd2, char *buffer, char *buffer2)
{
    ssize_t bytesRead, bytesRead2;
    while (1)
    {
        bytesRead = none_block_read(fd, buffer);
        printf("Check 1 is end\n");
        bytesRead2 = none_block_read(fd2, buffer2);
        printf("Check 2 is end\n");

        if (bytesRead == 0)
        {
            printf("1 fished!\n");
            break;
        }
        else if (bytesRead2 == 0)
        {
            printf("2 fished!\n");
            break;
        }
        printf("Read %zd bytes: %.*s\n", bytesRead, (int)bytesRead, buffer);
        printf("Read %zd bytes: %.*s\n", bytesRead2, (int)bytesRead2, buffer2);
    }
    return 0;
}
```
위는 동기-논블락킹 방식의 예시이다. 함수의 호출부가 응답 결과를 관리하며 논 블락킹이기 떄문에 IO로 인해 유저 실행 흐름이 막히지 않고 실행된다. 흥미로운 부분은 늦게 실행된 2번째 none-block-read 함수가 더 빨리 종료 될 수 있다는 점이다. 로컬에서 파일을 통해 진행한다 했을때 만약 2번째로 실행하는 파일의 크기가 더욱 작을 경우 더 빨리 종료될 수 있다.

> [!info]
> 동기만으로는 완료순서를 보장하지 못한다.

___
### Async
<span class="red-bg"><b>비동기는 동기와 달리 함수의 응답을 호출부에서 신경쓰지 않는다. 비동기 호출부는 함수가 다 완료 됐는지 함수의 결과 값이 무엇인지 궁금해하지 않고 함수를 실행 했다는 사실에만 집중한다. </b></span>응답 값은 나중에 시간이 흐른 후 필요할 때 꺼내서 사용하면 된다.
비동기는 함수의 완료를 동기와 달리 응답 여부로 판별하지 못한다. 비동기 함수의 호출부는 실행에만 집중하기 때문에 함수 호출에 대한 응답 값은 실행할 수 있다와 실행할 수 없다로만 국한된다. 따라서 동기와 달리 함수의 완료 여부를 호출부에서 확인할 수 없다. 
![[스크린샷 2023-11-09 오전 11.32.06.png]]

비동기는 함수의 책임자를 변경하는 듯한 효과를 가진다. 리눅스의 비동기 io인 aio 라이브러리를 살펴보면 io요청을 리퀘스트 큐에 enqueue하고 이를 백 그라운드에서 실행한다. 이후 완료되면 시그널을 발생시켜 함수의 상태를 업데이트 하고 결과 값을 전달 받은 버퍼에 채워넣는다. 이러한 동작 방식을 보면 함수의 호출 부는 함수의 결과에도 실패 유무에도 관계 없이 동작하는 것을 확인 할 수 있다.

**Async Example**




### Blocking and None-Blocking

### Blocking IO and None-Blocking IO

### Event Loop



### 출처
* https://developer.ibm.com/articles/l-async/ (전체 내용 출처)
___
### 개요
* [[#Sync]]
* [[#Blocking and None-Blocking]]
* [[#Blocking IO and None-Blocking IO]]
* [[#Event Loop]]
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
<span class="red-bg"><b>동기의 핵심적인 성질은 결과를 호출부에 반환한다는 것이다. 함수를 호출한 호출부는 함수의 실행 결과를 전달 받는다. 즉 함수의 응답을 호출 부에서 신경 쓴다고 말할 수 있다.</b></span> 동기-블락킹의 경우 호출부에 함수의 실행 결과가 전달되고 동기-논블락킹의 경우 함수의 실행 상태 혹은 결과가 전달된다. 
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
        printf("Check Large is done...\n");
        bytesRead2 = none_block_read(fd2, buffer2);
        printf("Check Small is done...\n");

        if (bytesRead == 0)
        {
            printf("Large fished!\n");
            break;
        }
        else if (bytesRead2 == 0)
        {
            printf("Small fished!\n");
            break;
        }
        printf("Read %zd bytes: %.*s\n", bytesRead, (int)bytesRead, buffer);
        printf("Read %zd bytes: %.*s\n", bytesRead2, (int)bytesRead2, buffer2);
    }
    return 0;
}
```
위는 동기-논블락킹 방식의 예시이다. 함수의 호출부가 응답 결과를 관리하며 **논 블락킹이기 때문에 IO로 인해  실행 흐름이 막히지 않는다.** 또한 **파일을 전부 읽었다는 것을 확인하기 위해 while문 내부에서 busy-waiting이 발생하는 것을 볼 수 있다.** 흥미로운 부분은 늦게 실행된 2번째 none-block-read 함수가 더 빨리 종료 될 수 있다는 점이다. 만약 2번째로 읽는 파일의 크기가 더욱 작을 경우 더 빨리 종료될 수 있다.

> [!info]
> **동기만으로는 완료순서를 보장하지 못한다.**

___
### Async
<span class="red-bg"><b>비동기는 동기와 달리 함수의 응답을 호출부에서 신경쓰지 않는다. 비동기 호출부는 함수가 다 완료 됐는지 함수의 결과 값이 무엇인지 궁금해하지 않고 함수를 실행 했다는 사실에만 집중한다. </b></span>응답 값은 나중에 시간이 흐른 후 필요할 때 꺼내서 사용하면 된다.
비동기는 함수의 완료를 동기와 달리 응답 여부로 판별하지 못한다. 비동기 함수의 호출부는 실행에만 집중하기 때문에 함수 호출에 대한 응답 값은 실행 가능 여부 로만 국한된다. 따라서 동기와 달리 함수의 완료 여부를 호출부에서 확인할 수 없다. 
![[스크린샷 2023-11-09 오전 11.32.06.png]]

**왜 비동기?**
비동기는 응답 결과를 호출부에서 신경쓰지 않는 방식으로 동작하는 것이라는 사실은 파악했다. 그렇다면 이러한 방식을 왜 만들었을까? 비동기의 탄생 배경을 확인하기 위해선 우선적으로 동기의 단점을 이해해야한다. 

동기-블락킹 방식의 단점을 우선적으로 살펴보자. 해당 방식의 문제점은 함수를 호출한 이후 해당 함수가 완료될 때까지 실행 흐름이 블락 된다는 점에 있다. 만약 해당 작업이 CPU bound 작업이라면 이는 비효율적이지 않지만, IO 작업으로 인해 블락될 경우 CPU의 대기시간이 증가해 프로세스의 처리 속도가 감소할 수 있다. <a href="https://m.blog.naver.com/gngh0101/220678113179">(왜  IO는 CPU를 안써도 될까?)</a>

동기-논블락킹은 위의 단점을 해결한 방식이다. IO 작업을 진행 하더라도 실행 흐름이 완전히 막히지 않아 별도의 작업을 처리할 수 있다. 하지만 시스템 콜을 반복적으로 호출하며 busy-waiting을 유발한다. 또한 시스템 콜을 반복 호출하기에 모드 스위칭으로 인한 오버헤드도 존재한다.

<span class="red-bg"><b>이에 따라 결과를 busy-waiting하지 않으면서 실행 흐름을 막지 않는 방식이 필요해졌고 이것이 바로 우리가 자주 듣는 Asychrnous-None Blocking이다.</b></span> None-Blocking은 함수를 곧장 반환하는 것으로 구현이 가능하지만, 응답을 신경쓰지 않는 것은 어떻게 구현할지 막연하다. 

<span class="red-bg"><b>리눅스의 비동기 지원 aio 라이브러리를 살펴보면 io요청을 리퀘스트 큐에 enqueue하고 이를 백 그라운드에서 실행한다. 이후 완료되면 시그널 혹은 스레드를 사용해 함수의 상태를 업데이트 하고 결과 값을 전달 받은 버퍼에 채워넣는다. </b></span> 

**Async Example** 
```c
    int fd = open("example.txt", O_RDONLY);

    if (fd == -1)
    {
        perror("Failed to open file");
        return 1;
    }

    // Create an aiocb structure to hold information about the asynchronous I/O request
    struct aiocb my_aiocb;
    memset(&my_aiocb, 0, sizeof(struct aiocb));

    // Set up the aiocb structure
    my_aiocb.aio_fildes = fd;
    my_aiocb.aio_buf = malloc(1024);
    my_aiocb.aio_nbytes = 1024;
    my_aiocb.aio_offset = 0;

    // Perform the asynchronous read operation
    if (aio_read(&my_aiocb) == -1)
    {
        perror("aio_read");
        close(fd);
        return 1;
    }
```
위의 코드는 비동기 입출력을 진행할 aiocb 객체를 생성하고 aio_read를 활용해 비동기로 데이터를 읽는 작업을 수행한다. ***aio_read를 실행하면 io_request가 생성돼 큐에 저장되며 큐를 통해 해당 작업을 관리한다.*** 

aio_read의 반환 값은 0과 -1로 0은 큐에 작업이 적절히 저장된 것을 의미하고 -1은 작업이 큐에 들어가지 않는 상황을 의미한다. 에러는 다양한 상황에서 발생할 수 있으며, 큐에 들어가 있는 작업이 너무 많은 경우에도 발생할 수 있다. IO는 백 그라운드에서 처리되기 때문에 현재의 실행 흐름을 막지 않고 호출부가 함수의 응답에 관심이 없으므로 이는 명확하게 비동기이다.

<b><u>결과는 어떻게 얻어요?</u></b>
함수의 반환 값이 0과 -1밖에 없다면 반환 값을 어떻게 얻을지가 궁금해진다. 일반적으로 함수를 호출하면 항상 함수의 실행 결과가 곧장 반환 됐기 때문에 별개의 방법으로 응답 값을 받아야 하는 상황자체가 어색하게 다가온다. 비동기의 반환 값은 큐에 있는 aiocb 객체를 통해 가져올 수 있다. 아래의 코드를 보자.
```c
  // Wait for the asynchronous I/O operation to complete
    while (aio_error(&my_aiocb) == EINPROGRESS)
    {
        // Do other work or sleep here
    }

    // Check for errors in the completed operation
    if (aio_error(&my_aiocb) != 0)
    {
        perror("aio_error");
        close(fd);
        return 1;
    }

    // Process the read data
    ssize_t bytesRead = aio_return(&my_aiocb);
    printf("Read %zd bytes: %.*s\n", bytesRead, (int)bytesRead, (char *)my_aiocb.aio_buf);

    // Clean up and close the file
    free(my_aiocb.aio_buf);
    close(fd);
```
<u><b>while문을 돌면서 aiocb가 담당하는 IO 작업이 완료될 때까지 대기한 후 해당 aiocb가 갖고 있던 버퍼의 값을 읽어오면 된다. 잠깐 그러면 결국 busy-waiting을 해야 하는거 아닌가?</b></u>
___
### Busy-wait in Async?
지금까지의 흐름을 정리해보면 우리는 Blocking을 피하고자 None-Blocking을 적용했고, busy-waiting을 피하고자 Async한 방식을 적용했다. 두가지 방식을 적용함으로써 우리는 CPU의 가용성을 높이고 busy-wait으로 인한 오버헤드를 줄였다고 생각했다.

하지만 위의 aio 라이브러리 사용 예시를 살펴보면, 비지 웨이팅이 발생하는 구간이 존재한다. 이는 <b><u>비동기 방식에 순서를 부여해 사용하기 때문에 발생하는 일이다. 비지 웨이팅 구문은 aio의 완료를 보장하기 위해 사용한다. 따라서 해당 while문에서 인위적인 blocking을 발생 시키고 이하에서 발생하는 작업은 전부 aio가 완료된 이후에 실행된다는 것을 보장한다. </b></u>
그렇다고 반드시 비지 웨이팅으로 블락킹을 만들어야 하는 것은 아니다. epoll, mutex 등을 활용해 작업하면 비지 웨이팅 없이 블락킹을 만드는 것이 가능하다. ([[IO Multiplexing]] 참조)
>[!info]
>**비동기에서 실행 순서를 보장하려면 블락킹이 필연적이다.**
___

* **그럼에도 비동기를 써야하는 이유**
**첫번째는 크기가 큰 파일을 IO하는 경우**이다. 비동기 방식의 경우 백 그라운드에서 IO를 진행하고 다른 업무를 처리할 수 있겠지만, 논 블락킹 - 동기의 경우 작업을 하는 와중에 계속해서 IO를 호출 해줘야 하기 때문에 오버헤드가 커질 수 있다.

**두 번째는 콜백으로 처리하는게 훨씬 간단한 경우**이다. 이는 보통 **==실행 완료를 보장할 필요가 없는 경우**==를 말하는데 DB 1의 값을 조회한 후 DB 2로 옮기는 작업을 진행한다고 해보자. 비동기로 작성한다면, 조회하는 함수를 작성하고 콜백으로 2에 옮기는 함수를 등록해주면 된다. 하지만 논 블락킹-동기로 작성할 경우 별도의 로직 구성이 필요하고 성능도 더 안좋을 가능성이 높다.
___
### Signal and Call Back
비동기는 자신의 종료 여부를 알리는 방식으로 시그널과 콜백을 사용한다. 시그널의 경우 [[Inter Process Communication]]의 대표적인 종류중 하나로 프로세스간 통신을 할때 사용하는 신호를 의미하고 콜백의 경우 호출에 대한 결과로써 호출되는 함수를 의미한다. 콜백의 경우 특정 함수의 완료시 동작하는 함수를 의미한다. 

비동기의 경우 기존에 실행했던 함수가 완료되면 시그널이 발생하면서 시그널 핸들러에 의해 특정 함수를 실행하거나 콜백을 실행할 수 있다. 이는 **자동적으로 진행되는 과정이며 인위적으로 함수의 완료를 확인할 필요는 없다.**

* **시그널**
시그널의 경우 시그널 핸들러와 감지할 시그널을 우선적으로 정의한다. 이후 생성한 시그널 핸들러를 비동기 객체와 연결한다. 비동기 객체에서 특정한 이벤트가 발생했을 때 시그널을 발생 시키고 발생한 시그널을 OS가 감지해 시그널 핸들러를 실행한다. 수도 코드는 아래와 같다.
```c
void setup_io( ... )
{
  int fd;
  struct sigaction sig_act;
  struct aiocb my_aiocb;

  ...

  /∗ Set up the signal handler ∗/
  sigemptyset(&sig_act.sa_mask);
  sig_act.sa_flags = SA_SIGINFO;
  sig_act.sa_sigaction = aio_completion_handler;


  /∗ Set up the AIO request ∗/
  bzero( (char ∗)&my_aiocb, sizeof(struct aiocb) );
  my_aiocb.aio_fildes = fd;
  my_aiocb.aio_buf = malloc(BUF_SIZE+1);
  my_aiocb.aio_nbytes = BUF_SIZE;
  my_aiocb.aio_offset = next_offset;

  /∗ Link the AIO request with the Signal Handler ∗/
  my_aiocb.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
  my_aiocb.aio_sigevent.sigev_signo = SIGIO;
  my_aiocb.aio_sigevent.sigev_value.sival_ptr = &my_aiocb;

  /∗ Map the Signal to the Signal Handler ∗/
  ret = sigaction( SIGIO, &sig_act, NULL );

  ...

  ret = aio_read( &my_aiocb );

}


void aio_completion_handler( int signo, siginfo_t ∗info, void ∗context )
{
  struct aiocb ∗req;


  /∗ Ensure it's our signal ∗/
  if (info‑>si_signo == SIGIO) {

    req = (struct aiocb ∗)info‑>si_value.sival_ptr;

    /∗ Did the request complete? ∗/
    if (aio_error( req ) == 0) {

      /∗ Request completed successfully, get the return status ∗/
      ret = aio_return( req );

    }

  }

  return;
}
```

* **콜백**
콜백도 위와 흡사하게 동작한다. 콜백 함수를 정의한 후 콜백 함수와 비동기 객체를 연결하는 작업을 진행한다. 연결된 콜백함수는 비동기 작업이 마무리 되면 자동적으로 실행된다. 작업이 마무리가 됐다는 것과 콜백을 실행하는 작업은 모두 OS가 처리해준다. 수도 코드는 아래와 같다.
``` c
void setup_io( ... )
{
  int fd;
  struct aiocb my_aiocb;

  ...

  /∗ Set up the AIO request ∗/
  bzero( (char ∗)&my_aiocb, sizeof(struct aiocb) );
  my_aiocb.aio_fildes = fd;
  my_aiocb.aio_buf = malloc(BUF_SIZE+1);
  my_aiocb.aio_nbytes = BUF_SIZE;
  my_aiocb.aio_offset = next_offset;

  /∗ Link the AIO request with a thread callback ∗/
  my_aiocb.aio_sigevent.sigev_notify = SIGEV_THREAD;
  my_aiocb.aio_sigevent.notify_function = aio_completion_handler;
  my_aiocb.aio_sigevent.notify_attributes = NULL;
  my_aiocb.aio_sigevent.sigev_value.sival_ptr = &my_aiocb;

  ...

  ret = aio_read( &my_aiocb );

}


void aio_completion_handler( sigval_t sigval )
{
  struct aiocb ∗req;

  req = (struct aiocb ∗)sigval.sival_ptr;

  /∗ Did the request complete? ∗/
  if (aio_error( req ) == 0) {

    /∗ Request completed successfully, get the return status ∗/
    ret = aio_return( req );

  }

  return;
}
```

<span class="red-bg"><b>이러한 시그널과 콜백을 활용하면 계속해서 결과를 신경 쓸 필요 없이 결과의 완료 소식만을 확인해주면 된다.  </b></span> 이는 비동기의 핵심적인 속성 중의 하나로 이를 활용해야 비동기를 비동기 답게 활용할 수 있다.
___
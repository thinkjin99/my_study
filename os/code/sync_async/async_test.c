#include <stdio.h>
#include <stdlib.h>
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>

// void read_callback(union sigval sig)
// {
//     int ret;
//     struct aiocb *aiocbp = (struct aiocb *)sig.sival_ptr;
//     /* read() 시스템 호출의 반환 값을 확인합니다. */

//     if (ret < 0)
//     {
//         perror("read()");
//         exit(EXIT_FAILURE);
//     }

//     /* 읽은 데이터를 출력합니다. */
//     printf("read %s bytes: %zd\n", aiocbp->aio_buf, aio_return(aiocbp));
// }

void read_callback()
{
    printf("Hello\n");
}

int main(void)
{
    int fd;
    struct aiocb aiocbp;
    char buf[1024];

    /* 파일을 열고 aiocb 구조체를 초기화합니다. */
    fd = open("file.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("open()");
        exit(EXIT_FAILURE);
    }
    aiocbp.aio_fildes = fd;
    aiocbp.aio_buf = buf;
    aiocbp.aio_nbytes = sizeof(buf);
    aiocbp.aio_offset = 0;
    aiocbp.aio_sigevent.sigev_notify = SIGEV_THREAD;
    aiocbp.aio_sigevent.sigev_notify_function = read_callback;
    aiocbp.aio_sigevent.sigev_notify_attributes = NULL;

    /* aio_read() 시스템 호출을 사용하여 파일에서 데이터를 읽습니다. */
    aio_read(&aiocbp);

    /* 파일을 닫습니다. */
    // while (1)
    // {
    /* code */
    printf("wait..\n");
    sleep(1);
    // }
    close(fd);

    return 0;
}
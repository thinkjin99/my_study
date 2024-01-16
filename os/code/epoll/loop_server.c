#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <uv.h>

uv_loop_t *loop;
struct sockaddr_in addr;

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status)
{
    if (status)
    {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread < 0)
    {
        if (nread != UV_EOF)
        {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            uv_close((uv_handle_t *)client, NULL); // 연결을 종료한다.
        }
    }
    else if (nread > 0)
    {
        uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t)); // 쓰기용 리퀘스트 객체 생성
        uv_buf_t wrbuf = uv_buf_init(buf->base, nread);             // 쓰기용 버퍼 생성
        printf("buffer: %s\n", buf->base);
        uv_write(req, client, &wrbuf, 1, echo_write); // 버퍼에 내용 작성
    }

    if (buf->base)
    {
        free(buf->base);
    }
}

void on_new_connection(uv_stream_t *server, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);                         // 클라이언트 용 tcp 소켓 초기화
    if (uv_accept(server, (uv_stream_t *)client) == 0) // 서버 소켓의 커넥션 클라이언트에 할당
    {
        uv_read_start((uv_stream_t *)client, alloc_buffer, echo_read); // 연결된 커넥션에서 데이터를 읽는 이벤트를 등록한다. 이벤트가 발생할 때 마다 콜백을 실행한다.
    }
    else
    {
        uv_close((uv_handle_t *)client, NULL);
    }
}

int main()
{
    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server); // tcp 소켓 초기화

    uv_ip4_addr("127.0.0.1", 7000, &addr);

    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);           // 바인딩
    int r = uv_listen((uv_stream_t *)&server, 128, on_new_connection); // 리스닝 작업 등록, 리스닝 이벤트 발생시 on_new_connection 콜백 실행
    if (r)
    {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }
    return uv_run(loop, UV_RUN_DEFAULT);
}
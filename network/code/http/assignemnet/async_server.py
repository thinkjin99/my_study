import pathlib
import random
import socket
import select

import time
import logging

from dataclasses import dataclass
from typing import Generator, Dict

logging.basicConfig(level=logging.INFO, format="%(message)s")


@dataclass
class Connection:
    event: select.kevent
    sock: socket.socket
    requests: list[Generator]


def read_data(sock: socket.socket):
    msg = ""
    while msg[-4:] != "\r\n\r\n":  # body 부분이 나오기 직전까지 읽기
        char = sock.recv(1).decode("utf-8")
        msg += char
        if (
            not char
        ):  # 메시지가 없다면 연결종료 (이러면 안된다 Conncetion:Close) 를 읽어야만 종료해야 함
            return None
    return msg


def get_html(path: str) -> str:
    cwd = pathlib.Path(__file__).resolve().parent
    if path == "/":
        path = "index.html"
    path = path.replace("/", "")
    with open(cwd / path, "r") as f:
        html = f.read()  # html 파일 로딩
    return html


def create_response(method: str, path: str) -> str:
    html = None
    try:
        status_code = 200
        message = "ok"
        assert method == "GET", "Not Implemented method"
        html = get_html(path)

    except FileNotFoundError:
        status_code = 404
        message = "Page Not Found"

    except AssertionError:
        status_code = 405
        message = " Method not Allowed"

    except Exception:
        status_code = 500
        message = "Server Error"

    finally:
        if not html:
            html = get_html(f"{status_code}.html")  # http 요청에 오류가 존재하는 경우

        first_line = f"""HTTP/1.1 {status_code} {message}\r\n"""
        header = f"Content-Type: text/html\r\nContent-Length:{len(html)}\r\n\r\n"
        response_msg = f"{first_line}{header}{html}"
        return response_msg


def server(Connection: Connection):
    start_line = None
    client_sock = Connection.sock

    try:
        msg = read_data(Connection.sock)

        if not msg:
            return Connection

        logging.info(msg + "\n")

        start_line, *_ = msg.split("\r")  # 메시지 파싱

    except (ConnectionResetError, OSError):
        logging.info(f"Connection reset connection...")
        return Connection

    if start_line:
        method, path, version = start_line.split()
        response_msg = create_response(method, path)

        start = time.time()
        hold_time = 3

        # 인위적으로 생성한 시간이 소요되는 프로세스
        while True:
            now = time.time()
            # print("Server waits...")
            if now - start >= hold_time:
                break
            yield 0  # 0은 커넥션이 살아있음

        client_sock.sendall(response_msg.encode("utf-8"))

    return 0


def run_requests(connections: dict[int, Connection]) -> list[int]:
    remove_fds = []
    for key, Connection in connections.items():
        finished_requests = set()  # 종료된 요청
        for req in Connection.requests:
            try:
                if req:
                    next(req)  # 대기 중인 작업 재실행

            except StopIteration as e:
                if e.value:
                    remove_fds.append(key)  # 종료할 커넥션
                finished_requests.add(req)

        Connection.requests = [
            req for req in Connection.requests if req not in finished_requests
        ]  # 완료된 작업 삭제

    return remove_fds


def remove_connection(
    kq: select.kqueue, remove_fds: list[int], connections: dict[int, Connection]
):
    for fd in remove_fds:
        closed_connection = connections[fd]
        kq.control(
            [
                select.kevent(
                    closed_connection.event.ident,
                    select.KQ_FILTER_READ,
                    select.KQ_EV_DELETE,
                )
            ],
            0,
        )  # 커넥션을 큐에서 제거한다.
        closed_connection.sock.close()  # 커넥션 닫기
        logging.info(f"\n{closed_connection.sock} is closed\n")
        del connections[fd]  # 커넥션 풀에서 제거한다.


def async_main():
    listen_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listen_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    listen_sock.bind(("127.0.0.1", 8080))
    listen_sock.listen(1000)

    kq = select.kqueue()
    server_event = select.kevent(
        listen_sock.fileno(), select.KQ_FILTER_READ, select.KQ_EV_ADD
    )
    kq.control([server_event], 0)

    connections: Dict[int, Connection] = {}  # 커넥션 풀

    while True:
        events = kq.control(None, 1000, 0.1)  # 최대 100개의 이벤트 등록

        for event in events:
            if listen_sock.fileno() == event.ident:
                client_sock, _ = listen_sock.accept()
                peer = client_sock.getpeername()
                logging.info(f"Connection Socket: {peer[1]} is connected")
                logging.info(f"Connection count: {len(connections.keys())}")

                client_event = select.kevent(
                    client_sock.fileno(), select.KQ_FILTER_READ, select.KQ_EV_ADD
                )  # read를 등록하고 큐에 추가하는 이벤트 생성

                kq.control([client_event], 0)  # 큐에 커넥션을 등록
                connections[client_sock.fileno()] = Connection(
                    sock=client_sock, event=client_event, requests=[]
                )  # 커넥션 풀에 커넥션 등록

            elif event.filter == select.KQ_FILTER_READ:
                request = server(connections[event.ident])  # 요청 제네레이터
                connections[event.ident].requests.append(request)

        remove_fds = run_requests(connections)
        remove_connection(kq, remove_fds, connections)  # 종료된 연결 삭제


if __name__ == "__main__":
    async_main()

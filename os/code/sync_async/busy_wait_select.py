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
            select.select([client_socket], [], [])

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


if __name__ == "__main__":
    # print(sync_busy_wait())
    print(sync_select())

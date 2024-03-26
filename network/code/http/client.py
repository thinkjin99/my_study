import socket
import time
import concurrent.futures


def send_recv(sock: socket.socket | None = None):
    request_msg = """GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\r\n"""
    start = time.time()

    if not sock:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.connect(("127.0.0.1", 8080))

    sock.send(request_msg.encode("utf-8"))
    resp = sock.recv(512)

    print("resp: ", resp[:10])
    end = time.time()

    return end - start


if __name__ == "__main__":
    with concurrent.futures.ThreadPoolExecutor(max_workers=8) as executor:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.connect(("127.0.0.1", 8080))
        # futures = [executor.submit(send_recv) for _ in range(100)]  # no reuse
        futures = [executor.submit(send_recv, sock) for _ in range(100)]  # yes reuse

        total = 0
        start = time.time()
        for i, f in enumerate(concurrent.futures.as_completed(futures), start=1):
            print(f"{i} is completed Sec: {f.result()}")
            total += f.result()

    print("close socket")
    if sock:
        sock.close()

    end = time.time()
    print(f"Total:{end-start}, avg sec: {total / 100}")

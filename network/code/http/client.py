import socket
import time
import concurrent.futures


def send_recv():
    request_msg = (
        """GET / HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nConnection: keep-alive\r\n"""
    )

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    start = time.time()
    sock.connect(("127.0.0.1", 8080))
    sock.send(request_msg.encode("utf-8"))
    resp = sock.recv(512)
    print("resp: ", resp[:10])
    end = time.time()

    return end - start


with concurrent.futures.ThreadPoolExecutor(max_workers=8) as executor:
    futures = [executor.submit(send_recv) for _ in range(100)]
    average = 0
    for i, f in enumerate(concurrent.futures.as_completed(futures), start=1):
        print(f"{i} is completed Sec: {f.result()}")
        average += f.result()

print(f"Average sec: {average / 100}")

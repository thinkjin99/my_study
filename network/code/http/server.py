import socket
from concurrent.futures import ThreadPoolExecutor
import pathlib
import random
import select
import time


import logging
import queue


logging.basicConfig(level=logging.INFO, format="%(message)s")
log_queue = queue.Queue()


def log_worker():
    while True:
        message = log_queue.get()
        logging.info(message)
        log_queue.task_done()


def get_html(path: str):
    cwd = pathlib.Path(__file__).resolve().parent
    if path == "/":
        path = "index.html"
    path = path.replace("/", "")
    with open(cwd / path, "r") as f:
        html = f.read()
    return html


def create_response(method: str, path: str):
    first_line = """HTTP/1.1 200 OK\r\n"""
    try:
        assert method == "GET", "Not Implemented method"
        html = get_html(path)

    except FileNotFoundError:
        first_line = """HTTP/1.1 404 Not Found\r\n"""
        html = get_html("404.html")

    except AssertionError:
        first_line = """HTTP/1.1 405 Method not Allowed\r\n"""
        html = get_html("503.html")

    except Exception:
        first_line = """HTTP/1.1 500 Server Error\r\n"""
        html = get_html("500.html")

    finally:
        bytes_length = len(html)
        header = f"Content-Type: text/html\r\nContent-Length:{bytes_length}\r\n\r\n"
        response_msg = f"{first_line}{header}{html}"
        return response_msg


def server(client_sock: socket.socket):
    peer = None
    start_line = None
    while True:
        try:
            if not peer:
                peer = client_sock.getpeername()

            log_queue.put(f"Client Socket: {peer[1]} ")

            msg = client_sock.recv(1024).decode("utf-8")
            if not msg:
                break

            log_queue.put(msg + "\n")

            start_line, *_ = msg.split("\r")

        except ConnectionResetError:
            log_queue.put(f"{peer} Client reset connection...")
            break

        if start_line:
            method, path, version = start_line.split()
            response_msg = create_response(method, path)
            time.sleep(random.randint(1, 3))
            client_sock.send(response_msg.encode("utf-8"))

    log_queue.put(f"Socket is closed")
    client_sock.close()


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    sock.bind(("127.0.0.1", 8080))
    sock.listen(100)
    print("Server is running...")
    with ThreadPoolExecutor(max_workers=16) as executor:
        executor.submit(log_worker)
        while True:
            client_sock, _ = sock.accept()
            log_queue.put(
                f"Connected {client_sock.getsockname()} >> {client_sock.getpeername()}\n"
            )
            executor.submit(server, client_sock)






if __name__ == "__main__":
    main()

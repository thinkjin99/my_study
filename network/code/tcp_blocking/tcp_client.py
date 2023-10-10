import socket
import time


def client(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print("Client assgined socket: ", sock.getsockname())
    sock.connect((host, port))  # SEND SYN
    print("Socket is connected")
    while True:
        msg = input("Client says: ")
        sock.sendall(msg.encode("utf-8"))
        if msg == "exit":
            break
        reply = sock.recv(128).decode("utf-8")
        print("Clinet got: ", reply)

    _ = input("Press any to close socket..")
    sock.close()


client("127.0.0.1", 8080)

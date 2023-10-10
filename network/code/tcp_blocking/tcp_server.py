import socket
import time


def server(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((host, port))  # CLOSED SOCKET
    print("Create Server Socket: ", sock)
    sock.listen(1)  # LISTEN
    print("Server is listening...")

    socks = []

    sock.setblocking(False)

    while True:
        try:
            client_sock, _ = sock.accept()  # ESTABLISHED
            client_sock.setblocking(False)
            print("Sever socket: ", sock.getsockname())  # 통신을 진행하는 소켓의 주소
            print("Client socket: ", client_sock.getpeername())  # 연결된 호스트의 주소
            socks.append(client_sock)

        except socket.error as e:
            time.sleep(1)
            print("Server is waiting...")

        finally:
            if len(socks):
                for i, sc in enumerate(socks):
                    print(i, "//", len(socks))
                    try:
                        msg = sc.recv(16).decode("utf-8")
                        if msg == "exit":
                            break

                        sc.sendall(msg.encode("utf-8"))
                        print(i, "send msg complete")

                    except socket.error as e:
                        print("Server wait for msg...")
                        continue

    # client_sock.close()  # close를 하지 않으면 어떻게 될까?
    # sock.close()


server("127.0.0.1", 8080)

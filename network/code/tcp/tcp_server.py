import socket
import time

# 간단한 에코서버이다. 여러 시스템 콜을 주석 처리해가며 소켓의 상태 변화를 확인해보자.
# 소켓의 상태는 netstat이나 lsof를 활용해 체크 가능하다.
# 해당 코드는 다중 접속을 허용하는 에코 서버이다.


def server(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((host, port))  # CLOSED SOCKET
    print("Create Server Socket: ", sock)
    sock.listen(1)  # LISTEN
    print("Server is listening...")

    socks = []  # 연결된 클라이언트 소켓

    sock.setblocking(False)  # 서버 소켓을 논 블락킹으로

    while True:  # 비지 웨이팅?
        try:
            client_sock, _ = sock.accept()  # new socket
            client_sock.setblocking(False)  # 소켓을 논 블락킹으로
            socks.append(client_sock)

        except OSError:
            pass

        for i, sc in enumerate(socks):
            print(i, "//", len(socks))
            try:
                msg = sc.recv(16).decode("utf-8")  # 클라이언트의 메시지 수신
                if msg == "exit":
                    client_sock.close()
                    socks.remove(sc)
                    break
                sc.sendall(msg.encode("utf-8"))  # 메시지 전송
                print(i, "send msg complete")

            except socket.error as e:
                print("Server wait for msg...")
                continue

    # client_sock.close()  # close를 하지 않으면 어떻게 될까?
    sock.close()


server("127.0.0.1", 8080)

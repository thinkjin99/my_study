import socket
import time


def server(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind((host, port))  # CLOSED SOCKET
    print("Create Server Socket: ", sock)
    # _ = input("Press any to listen")
    sock.listen(5)  # LISTEN
    print("Server is listening...")
    client_sock, _ = sock.accept()  # ESTABLISHED
    print("Sever socket: ", sock.getsockname())  # 통신을 진행하는 소켓의 주소
    print("Client socket: ", client_sock.getpeername())  # 연결된 호스트의 주소

    # 개선된 주먹구구
    sock.setblocking(False)  # 리스닝 소켓도 논 블락킹으로 생성한다.
    socks = []

    while True:
        try:
            client_sock, _ = sock.accept()  # 신규 커넥션 소켓 생성
            client_sock.setblocking(False)  # 클라이언트 소켓도 논 블락킹
            print("Sever socket: ", sock.getsockname())  # 통신을 진행하는 소켓의 주소
            print("Client socket: ", client_sock.getpeername())  # 연결된 호스트의 주소
            socks.append(client_sock)

        except socket.error as e:
            time.sleep(1)  # 연결이 안들어올 경우 1초 대기
            print("Server is waiting...")

        finally:
            # 신규 연결 유뮤와 상관 없이 기존 연결들은 전부 통신 가능해야 한다.
            if len(socks):
                for i, sc in enumerate(socks):  # 기존 연결들 전부 순회
                    print(i, "//", len(socks))
                    try:
                        msg = sc.recv(16).decode("utf-8")
                        if msg == "exit":
                            socks.remove(sc)
                            break

                        sc.sendall(msg.encode("utf-8"))
                        print(i, "send msg complete")

                    except socket.error as e:
                        print("Server wait for msg...")  # 메시지가 아직 오지 않은 경우
                        continue


server("127.0.0.1", 8080)

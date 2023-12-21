import socket


sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.bind(("127.0.0.1", 5005))
while True:
    # sock.sendto(bytes("hello", "utf-8"), ip_co)
    data, addr = sock.recvfrom(1024)
    print(data)

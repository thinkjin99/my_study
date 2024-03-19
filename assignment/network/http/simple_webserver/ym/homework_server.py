import asyncio
import socket
import time

# 서버 설정
host = "127.0.0.1"  # 서버의 IP 주소 또는 도메인 이름
port = 8000  # 포트 번호

# 기본 HTML 파일 경로
base_html_path = "./src"

# 서버 소켓 생성
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((host, port))
server_socket.listen(5)
server_socket.setblocking(False)

print(f"서버가 {host}:{port}에서 대기 중입니다...")

# 요청 처리 시간 저장을 위한 리스트
request_times = []

async def task(client_socket):
    start_time = time.time()  # 요청 처리 시작 시간

    try:
        # 클라이언트 요청 수신
        event_request = client_socket.recv(1024).decode("utf-8")
        # 요청된 첫 번째 줄 파싱 (예: "GET /article1.html HTTP/1.1")
        first_line = event_request.split("\n")[0]
        # URL 추출
        url = first_line.split(" ")[1]

        # 루트("/") 요청을 "index.html"로 매핑
        if url == "/":
            url = "/index.html"

        # 요청된 파일 경로 생성
        file_path = base_html_path + url

        # 파일 내용 가져오기
        content = get_file_content(file_path)
        if content:
            response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + content
        else:
            # 파일이 없을 때 404 응답
            response = "HTTP/1.1 404 Not Found\n\n<html><body><h1>404 Not Found</h1></body></html>"

        client_socket.send(response.encode("utf-8"))

    except Exception as e:
        print(f"오류 발생: {e}")
        error_response = "HTTP/1.1 500 Internal Server Error\n\nInternal Server Error"
        client_socket.send(error_response.encode("utf-8"))
    finally:
        end_time = time.time()  # 요청 처리 종료 시간
        request_times.append(end_time - start_time)  # 처리 시간 저장
        average_time = sum(request_times) / len(request_times)  # 평균 요청 처리 시간 계산
        print(f"평균 요청 처리 시간: {average_time:.4f}초")
        client_socket.close()

async def handle_client(client_socket, client_address):
    print(f"클라이언트 {client_address}가 연결되었습니다.")
    await task(client_socket)


def get_file_content(filename):
    # 파일반환
    try:
        with open(filename, "r", encoding="utf-8") as file:
            return file.read()
    except FileNotFoundError:
        return None

async def main():
    print("시작")
    while True:
        loop = asyncio.get_running_loop()

        while True:
            client_socket, client_address = await loop.sock_accept(server_socket)
            asyncio.create_task(handle_client(client_socket, client_address))

asyncio.run(main())

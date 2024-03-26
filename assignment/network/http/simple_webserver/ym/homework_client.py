import socket
import webbrowser
import tempfile
import time
import threading


def make_http_request_and_open_response(host, port, request_path):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((host, port))
        request = f"GET {request_path} HTTP/1.1\r\nHost: {host}\r\n\r\n"

        start_time = time.time()
        s.send(request.encode('utf-8'))

        response = s.recv(4096).decode('utf-8')
        end_time = time.time()

        response_time = end_time - start_time
        print(f"응답 시간: {response_time:.4f}초")

        header_end_index = response.find('\r\n\r\n') + 4
        html_content = response[header_end_index:]

        # HTML 콘텐츠를 임시 파일에 저장하고 웹브라우저에서 열기
        with tempfile.NamedTemporaryFile(delete=False, suffix='.html') as temp_file:
            temp_file.write(html_content.encode('utf-8'))
            temp_file_path = temp_file.name

        webbrowser.open(f'file://{temp_file_path}')


def make_requests_concurrently(host, port, request_path, num_requests):
    threads = []
    for _ in range(num_requests):
        thread = threading.Thread(target=make_http_request_and_open_response, args=(host, port, request_path))
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()  # 모든 스레드가 완료될 때까지 기다림


if __name__ == "__main__":
    host = "127.0.0.1"
    port = 8000
    request_path = "/"
    num_requests = 100  # 예시로 요청 수를 5개로 줄임

    make_requests_concurrently(host, port, request_path, num_requests)

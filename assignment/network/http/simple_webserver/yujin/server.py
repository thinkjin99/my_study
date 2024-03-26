import asyncio
import random
import time

async def work(reader, writer):
    request_line = await reader.readline()
    request_line = request_line.decode('utf-8').strip()
    print(f"Received request: {request_line}")

    try:
        method, path, _ = request_line.split()
        print(f"{method, path}")

        #GET 아닐 때 예외처리
        if method != 'GET': 
            raise ValueError("Only GET requests are supported")
        
        # 요청된 파일 경로 설정
        filepath = f'{path}'  # 루트 폴더를 현재 디렉터리로 가정
        if path[0] == '/':
            # 얘가 상대경로를 못찾네..
            filepath = 'assignment/network/http/simple_webserver/yujin/index.html'
        
        # 파일 읽기
        with open(filepath, 'rb') as f:
            content = f.read()
        
        # 랜덤 대기
        await asyncio.sleep(random.randint(1, 3))

        # 응답 전송
        writer.write(b"HTTP/1.1 200 OK\r\n")
        writer.write(b"Content-Type: text/html\r\n")
        writer.write(b"\r\n")
        writer.write(content)

    #예외처리 다 500
    except Exception as e:
        writer.write(b"HTTP/1.1 500 Internal Server Error\r\n")
        writer.write(b"Content-Type: text/html\r\n")
        writer.write(b"\r\n")
        writer.write(b"Server Error")
        print(f"Error: {e}")

    finally:
        await writer.drain()
        writer.close()

async def main():
    server = await asyncio.start_server(work, '127.0.0.1', 8080)
    print(f"Serving on {server.sockets[0].getsockname()}")
    await server.serve_forever()

asyncio.run(main())

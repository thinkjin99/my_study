import requests
import time
# 동시성 실행을 위한 라이브러리로, 여러 스레드에서 함수를 병렬로 실행할 수 있게함
from concurrent.futures import ThreadPoolExecutor

# HTTP GET 요청을 보내고, 응답의 상태 코드를 반환하는 함수
def getStatusCode(url):
    with requests.get(url) as response:  
        return response.status_code 

def main():
    url = "http://127.0.0.1:8080/index.html" 
    with ThreadPoolExecutor(max_workers=100) as executor:  

        # 얘를 안쓰고 싶음...
        # 최대 100개의 스레드를 가진 ThreadPoolExecutor를 생성
        start_time = time.time()  # 요청 시작 시간 기록
        
        # executor.submit: 100개의 요청을 병렬로 보냄, getStatusCode 함수를 100번 호출
        # futures 리스트에 저장
        futures = [executor.submit(getStatusCode, url) for _ in range(100)]
        # futures 리스트에 저장된 모든 future 객체의 결과를 기다리고, 각각의 결과(Status code)를 results 리스트에 저장
        results = [future.result() for future in futures]
        end_time = time.time()  

    # 총 시간, 평균 요청 시간, 모든 요청의 상태 코드 출력
    print(f"100 requests completed in {end_time - start_time} seconds")
    print(f"Average request time: {(end_time - start_time) / 100} seconds")
    print(f"Status codes: {results}")

if __name__ == "__main__":
    main()

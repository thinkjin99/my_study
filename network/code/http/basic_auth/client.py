import http.client
import base64

# 서버 주소
host = "localhost"
port = 8080
url = "/"

# 유효한 사용자 이름과 비밀번호
username = "user"
password = "pass"

# Basic Authentication 헤더 생성
credentials = f"{username}:{password}"
encoded_credentials = base64.b64encode(credentials.encode("utf-8")).decode("utf-8")
print(encoded_credentials)
auth_header = f"Basic {encoded_credentials}"

# HTTP 클라이언트 생성
conn = http.client.HTTPConnection(host, port)

# GET 요청 전송
headers = {"Authorization": auth_header}
conn.request("GET", url, headers=headers)

response = conn.getresponse()
data = response.read().decode("utf-8")

# 응답 출력
if response.status == 200:
    print("Authentication successful:", data)
else:
    print("Authentication failed:", response.status, data)

conn.close()

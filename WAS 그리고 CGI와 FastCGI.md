### 출처
* [CGi,.WSGI, ASGI](https://kangbk0120.github.io/articles/2022-02/cgi-wcgi-asgi)
* [WAS, CGI, WSGI](https://brownbears.tistory.com/350)
### 개요
* CGI란?
* CGI의 동작방식
* 파이썬의 CGI
* CGI의 단점
* FastCGI란
* Nginx에 FastCGI 적용하기
* FastCGI와 WSGI
* WSGI의 동작방식
* WAS란?
* 장고는 어떻게 동작하나요?
___
### CGI란?

일전의 [[웹서버와 nginx#정적 웹서버의 한계]]에서 말했듯 정적 웹서버는 동적인 요청을 처리할 수는 없었다. 이에 따라 동적인 요청을 처리하기 위한 <b><u>스크립트 등을 동작 시켜 클라이언트에게 반환하고 싶다는 욕망이 발생하게 됐는데 이를 위해 탄생한 것이 CGI이다.</u></b>

CGI는 Common Gateway Interface의 약자로 ==**웹 서버(정적)와 응용 프로그램(파이썬, PHP 등)이 통신하는 규약을 정의한다.**== CGI는 통신 규약이기 때문에 각 언어에 맞춰 다양한 종류가 존재한다.

CGI는 웹서버가 실제 프로그램 코드를 실행하는 방식으로 동작한다. 이때 **요청을 수신할 때마다 프로세스를 생성해 프로그램을 동작 시키므로 요청이 많을 경우 부하가 심하다.**
___
### CGI의 동작 방식

동작방식의 기준은 파이썬 CGI이다. 과거 파이썬 CGI는 아래와 같이 동작한다.
* 웹 서버가 클라이언트로부터 요청을 전달 받는다.
* **웹 서버는 요청에 담긴 정보를 환경 변수나 표준 입력에 전달한 후 스크립트를 실행**한다.
* 스크립트는 표준 입력을 통해 받은 데이터를 활용해 로직을 수행하고 **결과물을 표준 출력으로 웹서버에 전달한다.**
* 웹 서버는 이를 클라이언트에 전달한다.

이제 실제 파이썬에 기본 탑재 돼있는 CGI의 내부 동작을 한번 샅샅이 파악해보자. 파이썬 내장 클래스 `http.server`를 활용해 간단한 웹 서버를 작성해보자.

```python
import http.server
import socketserver

def run(server_class=http.server.HTTPServer, handler_class=http.server.SimpleHTTPRequestHandler):
    PORT = 8000
    server_address = ('localhost', PORT)
    httpd = server_class(server_address, handler_class)
    httpd.serve_forever()

run()
```

[serve_forever](https://github.com/python/cpython/blob/main/Lib/socketserver.py#L218) 함수는 이벤트 루프를 계속해서 실행하는 것과 비슷한 효과를 발생시킨다. 서버는 `select` 시스템 콜을 활용해 자신이 관심있는 소켓에서 이벤트가 발생했는지 체크한 후 처리를 진행한다.


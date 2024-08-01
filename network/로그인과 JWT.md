#jwt #로그인 #BasicAuth
### 출처
* [토스 기본인증](https://docs.tosspayments.com/resources/glossary/basic-auth)
* [HMAC VS RS256](https://erjuer.tistory.com/83)
___
### 개요
* [[#로그인]]
* [[#Basic Auth]]
* [[#기본인증 파이썬 서버]]
* [[#Stateful한 로그인과 Stateless한 로그인]]
* [[#JWT]]
* [[#왜 JWT?]]
___
### 로그인

로그인은 중요한 데이터와 리소스를 보호하기 위한 행위로 컴퓨터 시스템에서 사용하는 일반적인 보안 수단이다. 서비스는 유저의 이메일과 비밀번호를 요청하고 이를 통한 인증 이후 사용자에게 서비스 이용 권한을 부여한다. **로그인은 사용 권한을 얻기 위한 인증 과정을 말한다.**
___
### Basic Auth

로그인은 인증과정이고 인증 수단으로 유저이름과 비밀번호를 사용한다는 이야기까지 진행했다. 
그렇다면 지금부터 HTTP를 기반의 간단한 로그인을 만들어보자. 인증 만이 목표라면 아래와 같은 방식으로 만들 수 있다.

![500](https://obs3dian.s3.ap-northeast-2.amazonaws.com/%EB%A1%9C%EA%B7%B8%EC%9D%B8%EA%B3%BC%20JWT%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-07-30%20%EC%98%A4%ED%9B%84%203.32.36.png)

위의 이미지는 **HTTP 요청 메시지의 헤더에 인증에 필요한 데이터를 담아 전송**하고 있다. 서버는 헤더에 담긴 정보를 인식해 인증을 진행하고 적절한 응답을 반환한다. 직관적이고 구현이 간단한 방법이다. 
이 방법의 경우 **로그인 페이지도 필요가 없는데, 어차피 매 요청마다 인증 정보를 헤더에 담아 전송하는 방식으로 동작하기 때문이다.**

**기본인증 방식의 문제점으로 자주 지목되는 것이 헤더 내부의 정보는 전부 열람 가능하다는 것인데 사실 HTTPS를 적용하면 헤더 정보를 읽는 것은 불가능하다.** 이에 따라 기본인증을 통한 로그인을 하는 것이 꼭 보안에 취약하다 할 수는 없다. (오히려 노출될 경우 아이디와 비밀번호 자체가 털린다는 점이 취약점이다)

[토스](https://docs.tosspayments.com/resources/glossary/basic-auth)의 사례를 확인해보면 토스는 코어 API에 한해서는 현재도 기본 인증을 통한 인증을 진행하고 있다. 토스는 기본 인증 방식의 장점과 단점을 아래와 정리하고 있다.

> Basic 인증 방식의 **가장 큰 장점은 간단함이에요**. 사용자 ID와 비밀번호 외에 **로그인 페이지나 별도의 인증 정보를 요구하지 않아요.** 이런 간편함 때문에 다수의 서비스가 Basic 인증 방식을 사용하고 있어요. 쉬운 접근이 중요시되는 웹 서비스를 기반으로 만들어진만큼 Basic 인증은 단순하고 구축하기 쉬워요.
> 
> 그러나 **Basic 인증 방식은 서버에 사용자 목록을 저장**해요. 요청한 리소스가 많거나 사용자가 많으면 목록에서 권한을 확인하는 시간이 길어지겠죠. 또한 서버에 현실적으로 저장할 수 있는 데이터는 한정되어 있어서 **사용자가 많거나 사용자 변화가 잦은 서비스가 Basic 인증을 사용하면 서버에 부담이 커져요.**
> 
> 또 Basic 인증 방식만으로는 **사용자 권한을 정교하게 제어할 수 없어요**. 사용자가 꼭 필요한 리소스에만 권한을 주는 게 좋은데, **Basic 인증 방식으로 세세하게 사용자의 권한을 설정하려면 추가 구현이 필요**해요. 사용자 ID, 비밀번호는 우리에게 가장 친근한 인증 방법이지만, 어떻게 보면 그만큼 구시대적이에요. 복잡한 현대 IT 서비스는 더 정교한 인증 방식을 요구해요.

> [!기본인증 정리]
> 기본인증을 사용하면 간편한 구현이 가능하나 사용자가 많아질 경우 부하가 발생한다. 또한 유저 아이디와 비밀번호 외에 정보를 사용할 수 없기에 사용 가능한 정보에 한계가 존재한다.

___
### 기본인증 파이썬 서버

```python title:server.py hl:14,28
from http.server import BaseHTTPRequestHandler, HTTPServer
import base64

# 유효한 사용자 이름과 비밀번호
USERNAME = "user"
PASSWORD = "pass"


class RequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        # 인증 헤더 확인
        auth_header = self.headers.get("Authorization")

        if auth_header is None or not self.check_auth(auth_header):
            self.send_auth_request()
        else:
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(b"Authentication successful!")

    def check_auth(self, auth_header):
        auth_type, encoded_credentials = auth_header.split()
        if auth_type != "Basic":
            return False

        # Base64로 인코딩된 인증 정보 디코딩
        decoded_credentials = base64.b64decode(encoded_credentials).decode("utf-8")
        username, password = decoded_credentials.split(":")

        return username == USERNAME and password == PASSWORD

    def send_auth_request(self):
        self.send_response(401)
        self.send_header("WWW-Authenticate", 'Basic realm="Login Required"')
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(b"Authentication required.")


def run(server_class=HTTPServer, handler_class=RequestHandler, port=8080):
    server_address = ("", port)
    httpd = server_class(server_address, handler_class)
    print(f"Starting httpd server on port {port}")
    httpd.serve_forever()


if __name__ == "__main__":
    run()

```

파이썬을 사용해 간단한 기본인증 서버를 구축해봤다. 코드는 정석적인 기본 인증 플로우를 따른다. 서버는 인증 헤더를 통해 전송된 Base64 인코딩 데이터를 디코딩하고 세션(메모리)에 저장된 유저 데이터와 비교해 일치할 경우 성공 응답을 실패할 경우 실패 응답을 반환한다.

클라이언트에서 서버에 요청을 보내보고 싶다면 아래와 같은 `curl` 요청을 전송하면 된다. `-u` 는 기본인증 옵션으로 사용시 해당 요청을 기본인증 방식을 적용해 전송한다.

```bash
 curl -u user:pass http://127.0.0.1:8080/
 
 Authentication successful! #Success!
 Authentication required. #Fail!
```

기본인증은 토스가 말한 그대로의 성질을 띄고 있는 것을 확인할 수 있다. 구현이 간단하지만 **[[쿠키와 세션#세션|세션]] 기반 이기에 서버 부하가 심한 편이고 오직 유저의 아이디와 비밀번호 만을 전달하기 때문에 별도의 데이터를 넘기기 위해서는 추가 작업이 필요하다.**

>[!기본인증이 커버하지 못하는 것들]
>로그인의 유효시간을 3시간으로 설정하고 싶다고 해보자. 토큰을 사용할 경우 토큰 생성시각 세션의 경우 세션 생성 시각을 사용하면 되지만, 기본 인증은 아이디, 비밀번호를 제외한 데이터를 저장하는 것이 불가해 서버에서 별도의 처리를 진행해줘야 한다. 

___
### Stateful한 로그인과 Stateless한 로그인

세션 로그인과 토큰 로그인으로 구분 하기도 한다. 

* **세션 로그인 (Stateful)**
	**세션 로그인은 상태를 가지는 로그인으로 서버 측에서 데이터를 저장해 인증을 진행한다.** 클라이언트가 로그인에 성공할 경우 서버는 세션 ID를 생성해 클라이언트에게 발급 해준다. 이후 클라이언트는 아이디, 비밀번호가 아닌 세션 아이디를 서버에 전달해 인증을 수행한다. **이때 클라이언트는 세션 정보를 가지는 서버에게만 요청을 전달해야 하기 때문에 스케일업 등의 작업이 어려워진다.**

* **토큰 로그인 (Stateless)**
	토큰 로그인은 토큰에 상태 데이터를 전부 기록하고 이를 통해 통신을 진행한다. 토큰 내부에는 인증 정보 및 만료 정보 등이 포함돼 있다. **정보가 토큰 내부에 저장되고 토큰은 클라이언트에서 관리하기 때문에 서버의 부하가 줄어든다는 장점이 존재한다.**

앞서 우리가 살펴본 기본 인증은 서버에 인증 정보를 기록하기 때문에 세션 기반 인증의 일종이 된다. 세션 기반 로그인의 핵심은 어떻게 정보를 저장하고 탐색하는 체계를 만들 것인가가 된다.

>[!세션]
>거듭 말하지만 세션은 정보를 서버가 저장하다보니 부하감당이 어려울 수 있다. 하지만 별도의 해싱이나 DB연결을 위한 네트워크 IO가 존재하지 않기 때문에 빠를 수도 있다.

___
### JWT

JWT(Json Web Token)는 JSON 형식의 페이로드를 저장하고 있는 해시 토큰을 의미한다. JWT는 헤더, 페이로드, 서명 세가지 파트로 구성되며 .을 통해 이를 구분할 수 있다.

![](https://obs3dian.s3.ap-northeast-2.amazonaws.com/%EB%A1%9C%EA%B7%B8%EC%9D%B8%EA%B3%BC%20JWT%20/%20Pasted%20image%2020240730173629.png)

실제 JWT는 다음과 같은 형태이다.  `eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c` 중간을 확인해보면 .을 통해 토큰이 구분되는 것을 확인할 수 있다.

이제 각 부분에 어떠한 정보가 들어가는지 확인해보자. 
* **헤더**
	사용한 암호화 알고리즘과 토큰의 타입에 대한 정보가 저장된다.

* **페이로드**
	추가적으로 저장하고 싶은 데이터가 저장된다.

* **서명**
	무결성 검사를 위한 장치이다. 서명은 헤더와 페이로드를 서버의 비밀 키를 활용한 해싱을 통해 생성되며 이를 통해 클라이언트는 헤더와 페이로드에 변형이 일어나지 않았다는 것을 확신할 수 있다.

#### 서명으로 어떻게 무결성을 검증해요?
JWT를 발급하는 서버는 비밀 키를 보유하고 있다. 서버는 토큰에 저장될 헤더와 페이로드를 자신의 비밀 키로 해싱하고 이를 서명으로 작성해 클라이언트에게 전송한**다. 클라이언트는 이후 서버의 공개 키나 서버가 전달한 대칭 키를 활용해 헤더와 페이로드를 동일하게 해싱하고 서명 값과의 일치 여부를 확인한다.** 이때 값이 일치하면 위조되지 않은 것이고 일치하지 않으면 변경이 발생한 것이다.

### HMAC VS RS256
**HMAC은 대칭키를 활용해 해싱을 진행하는 방식이다.** 서버와 클라이언트는 동일한 키를 공유해 해싱을 진행한다. 하나의 키만 사용하기 때문에 구현이 쉽고 간단하지만, 대칭 키가 노출될 경우 재앙이 발생한다. **또한 클라이언트 입장에서 해당 JWT가 실제로 서버에서 전달된 것인지를 구분하는 것도 어렵기 때문에 보안성이 RS256 방식에 비해 높지 않다.**

RS256은 비대칭키를 활용해 해싱을 진행하는 방식이다. 클라이언트는 서버의 공개키를 보유하고 있기 때문에 서버로부터 전달받은 서명이 실제로 서버의 비밀키로 서명된 것인지 확인할 수 있다. 또한 키를 주고 받는 과정이 없기 때문에 HMAC에 비해 보안성이 높은 방식이다. **다만 클라이언트에서 별도로 공개 키를 관리해야 한다는 점등 비대칭키를 사용함으로써 발생하는 오버헤드가 존재한다.** HTTPS에서 인증을 처리하는 방식과 몹시 흡사하다.

___
### 왜 JWT?

근본적인 질문으로 돌아와보자. 왜 다들 JWT를 사용하는 것일까? 해당 질문을 2개로 분리하면 왜 토큰 방식의 로그인인가JSON 형태의 데이터를 사용할까 정도로 정리할 수 있다. 

* **왜 토큰인가?**
	토큰을 선택한 이유는 Stateless와 연결된다. 토큰 기반 방식은 상태가 없기 때문에 확장이 간편하다. 자세한 설명은 [[Stateful과 Stateless#Stateless]]를 살펴보면 된다. 

* **왜 JSON인가?**
	**웹 어플리케이션이 복잡해지면서 인증과정에서도 다뤄야할 데이터의 수가 많아졌다**. 이에따라 효과적으로 데이터들을 관리할 방법이 필요해졌는데 JWT를 활용하면 이러한 데이터를 대다수의 언어에서 지원하는 JSON 포맷으로 관리하므로 클라이언트에서도 쉽게 사용이 가능하다. 또한 JSON은 Key-Val 형태이기 때문에 데이터의 유효성 체크 또한 빠르다.

* **DB를 사용해도 되지 않을까요?**
	DB를 사용해도 Stateless를 확보할 수 있지만, 네트워크 IO 딜레이가 발생한다. 반면 토큰의 경우 해싱만 진행하면 되기 때문에 더욱 빠른 처리가 가능하다.


>[!Why JWT]
>JWT는 토큰에 JSON 형식의 데이터를 담을 수 있기 때문에 사용한다. 

___
### 만들어보자 JWT

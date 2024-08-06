#jwt #로그인 #BasicAuth #refreshtoken #accesstoken
### 출처
* [토스 기본인증](https://docs.tosspayments.com/resources/glossary/basic-auth)
* [HMAC VS RS256](https://erjuer.tistory.com/83)
* [MAC이란?](https://kchanguk.tistory.com/137)
___
### 개요
* [[#로그인]]
* [[#Basic Auth]]
* [[#기본인증 파이썬 서버]]
* [[#Stateful한 로그인과 Stateless한 로그인]]
* [[#JWT]]
* [[#왜 JWT?]]
* [[#만들어보자 JWT]]
* [[#리프레시 토큰과 엑세스 토큰]]
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
### HMAC이 정확히 뭐에요?

HMAC을 이해하기 위해서는 우선적으로 MAC(Message Authentication Code)에 대해 이해할 필요가 있다. 네트워크는 상대방과 직접적으로 연결되지 않기 때문에 항상 데이터의 변조나 위조에 대해 고려할 필요가 존재한다. **이에따라 서버와 클라이언트가 서로 간의 데이터에 변조가 발생하지 않았다는 것을 확인하기 위해 별도의 코드를 전송해 이를 확인하는데 이때 사용하는 것이 MAC이다. MAC은 메시지에 덧붙여지는 정보로 각 네트워크 구성원은 해당 코드 값을 활용해 메시지의 무결성을 파악한다.**

![](https://obs3dian.s3.ap-northeast-2.amazonaws.com/%EB%A1%9C%EA%B7%B8%EC%9D%B8%EA%B3%BC%20JWT%20/%20Pasted%20image%2020240806113050.png)

HMAC은 MAC의 응용버전으로 해싱함수를 통해 코드 값을 생성하는 방식을 말한다. **HMAC은 전송하고자 하는 메시지를 특정한 키를 통해 해싱한 후 전송을 진행한다. 수신 측은 전달 받은 메시지를 동일한 키를 활용해 해싱하고 결과를 전달 받은 MAC과 비교해 무결성을 확인한다.** HMAC은 SHA256 해시 함수를 사용해 해시 값을 생성하고 이때 보안을 위해 키를 활용한다. (SHA256 자체는 원래 키가 필요없다)

![500](https://obs3dian.s3.ap-northeast-2.amazonaws.com/%EB%A1%9C%EA%B7%B8%EC%9D%B8%EA%B3%BC%20JWT%20/%20Pasted%20image%2020240806113555.png)

**HMAC 기법을 활용하면 해싱 함수를 활용해 메시지의 무결성만을 확인할 수 있고 해당 메시지가 실제 서버에서 전달된 것인지는 확인하지 못한다.** 추가적으로 동일한 해싱 결과를 얻기 위해서는 동일한 비밀 키를 활용해야 하기 때문에 단일 키가 갖는 취약점을 갖고 있다.

==**하지만 해싱 함수만을 적용해 메시지를 인증하기 때문에 RS256 방식과 비교했을 때 빠른 처리가 가능하다.**== 

>[!HMAC]
>**HMAC은 메시지의 무결성을 인증하는 방식 중 하나로 공유키와 해시를 통해 메시지의 무결성을 인증한다. 보안이 아닌 무결성만을 입증하기에 빠르고 구현이 편하다.**

___
### RS256이 정확히 뭐에요?

**RS256은 전자서명 방식을 의미하며 해싱과 암호화를 같이 사용한다고 이해하면 쉽다.** HMAC과 달리 해싱을 진행한 후 비대칭 키를 활용해 암호화를 한번 더 진행한다. 자세한 부분은 [[HTTPS#SSL 인증서]]
를 참고하자 흡사한 방식이다.

RS256은 비대칭 키를 활용해 서명을 진행하기 때문에 전송 주체의 신원을 파악하는 것이 가능하다. 또한 비밀 키를 서버에서만 관리 하므로 보안성이 높다. 하지만 공개 키를 별도로 관리해야하고 암,복호화를 추가로 진행해야 하는 만큼 오버헤드가 발생한다. **따라서 높은 보안성이 요구되는 상황이 아니라면 HMAC을 사용하는 것이 좋다.**

>[! 왜 암호화 함수가 해싱함수보다 느린가요?]
>암호화 함수는 복호화가 가능해야하고 해싱 함수는 롤백을 고려할 필요가 없다. **이에따라 해시 함수는 대부분 간단한 비트연산이 주를 이루지만, 암호화 연산은 산술 연산이 집약된 형태로 구현된다.** 이에 따른 연산 난이도 차이로 인해 차이가 발생한다.

___
### 만들어보자 JWT

이제 JWT를 직접 만들어보자. HMAC과 RS256 총 2가지 방식으로 JWT 코드를 작성해본다.
먼저 HMAC 방식을 활용한 JWT 토큰 발급 예제를 작성해보자.

```python hl:5,15,20
import jwt
import datetime

# 비밀 키 설정
secret_key = "your_secret_key"

# 페이로드 데이터 설정
payload = {
    "user_id": 123,
    "username": "testuser",
    "exp": datetime.datetime.utcnow() + datetime.timedelta(hours=1)  # 토큰 만료 시간
}

# JWT 토큰 생성
token = jwt.encode(payload, secret_key, algorithm="HS256")

print(f"Generated JWT Token: {token}")

# 토큰 디코딩 (검증)
decoded_payload = jwt.decode(token, secret_key, algorithms=["HS256"])

print(f"Decoded Payload: {decoded_payload}")

```

HMAC 방식은 대칭 키를 활용해 해싱을 진행하고 이를 통해 토큰을 발급한다. 해시 값 검증을 위해서는 대칭 키가 클라이언트에도 존재해야한다. 하지만 클라이언트 내부에 해시 키를 노출하는 형태는 치명적이다. **일반적으로 HMAC 방식의 jwt를 활용하면 클라이언트에선 페이로드만 확인 및 활용이 가능하고 토큰의 무결성을 확인하는 것은 불가능하다.** 

이제 RS256 방식으로 JWT 토큰을 발급 해보자. 우선적으로 개인키와 공개키를 openssl을 사용해 발급 받는다. 개인 키는 아래와 같이 발급 받을 수 있다.
`openssl genpkey -algorithm RSA -out private_key.pem -pkeyopt rsa_keygen_bits:2048` 

공개 키는 개인 키로부터 발급 받을 수 있다. `openssl rsa -pubout -in private_key.pem -out public_key.pem`

발급 받은 키를 토대로 jwt 토큰을 발급받는 예제를 작성한다. 비밀 키를 통해 해시 값을 암호화하고 공개 키를 통해 이를 복호화 한다. 해시 키는 별도로 활용하지 않는다.

```python
import jwt
import datetime

# 개인 키를 파일에서 읽어옵니다.
with open("private_key.pem", "r") as f:
    private_key = f.read()

# JWT 토큰을 생성하는 함수
def create_jwt_token():
    payload = {
        "user_id": 123,
        "exp": datetime.datetime.now() + datetime.timedelta(hours=1),
    }
    token = jwt.encode(payload, private_key, algorithm="RS256")
    return token

# 공개 키를 파일에서 읽어옵니다.
with open("public_key.pem", "r") as f:
    public_key = f.read()

# JWT 토큰을 검증하는 함수
def verify_jwt_token(token):
    try:
        payload = jwt.decode(token, public_key, algorithms=["RS256"])
        return payload
    except jwt.ExpiredSignatureError:
        print("토큰의 유효 기간이 만료되었습니다.")
    except jwt.InvalidTokenError:
        print("유효하지 않은 토큰입니다.")

# JWT 토큰 생성
jwt_token = create_jwt_token()
print("발급된 JWT 토큰:", jwt_token)

# JWT 토큰 검증
decoded_payload = verify_jwt_token(jwt_token)
print("검증된 페이로드:", decoded_payload)
```

RS256은 공개 키를 통해 검증을 진행하기 때문에 토큰이 서버로 부터 전달 됐는지, 무결성을 띄고 있는지를 검증할 수 있다. **이는 앞서 구현한 HMAC과의 차이점으로 클라이언트 단에서 나름의 인증 처리를 하는 것이 가능하다.**

>[!HMAC과 RS256중 어떤거?]
>**서버를 신뢰할 수 있고 빠른 속도가 중요하다면 HMAC을 사용하는 것이 좋다. 보안성을 높여야하고 속도가 중요하지 않다면 RS256이 유리하다.**

___
### HMAC VS RS256 퍼포먼스

위에서 작성한 간단한 토큰 발급 예제를 근거로 성능을 비교해보자. 아래는 10건 100건 1000건 별로 토큰을 인코딩하고 디코딩하는데 소요되는 시간이다. 측정은 파이썬의 `cProfile` 모듈을 활용해 진행했다.  HMAC 방식의 성능부터 측정한다.

```python
import jwt
import datetime
from cProfile import Profile
from pstats import Stats


# 비밀 키 설정
secret_key = "your_secret_key"


# 페이로드 데이터 설정
def jwt_encode():
    payload = {
        "user_id": 123,
        "username": "testuser",
        "exp": datetime.datetime.now() + datetime.timedelta(hours=1),  # 토큰 만료 시간
    }

    # JWT 토큰 생성
    token = jwt.encode(payload, secret_key, algorithm="HS256")
    return token


def jwt_decode(token):
    # 토큰 디코딩 (검증)
    decoded_payload = jwt.decode(token, secret_key, algorithms=["HS256"])
    return decoded_payload


def test():
    for _ in range(10000):
        token = jwt_encode()
        decode_token = jwt_decode(token)


profiler = Profile()
profiler.enable()
test()
profiler.disable()

stats = Stats(profiler)
stats.strip_dirs()
stats.sort_stats("cumulative")
stats.print_stats()
```

인코딩과 디코딩에 소요된 시간을 중점적으로 성능을 체크해보자. 결과는 아래와 같다

| count | type   | cumulative(sec) | per(sec) |     |
| ----- | ------ | --------------- | -------- | --- |
| 10    | encode | 0.001           | 0        |     |
| 10    | decode | 0               | 0        |     |
| 100   | encode | 0.003           | 0        |     |
| 100   | decode | 0.003           | 0        |     |
| 1000  | encode | 0.024           | 0        |     |
| 1000  | decode | 0.029           | 0        |     |
| 10000 | encode | 0.232           | 0        |     |
| 10000 | decode | 0.29            | 0        |     |
**인코딩과 디코딩 모두 아주 짧은 시간 밖에 소요되지 않으며 많은 요청을 처리하는데도 큰 시간이 소요되지 않는 다는 것을 확인할 수 있다.** 이제 RS256 방식을 확인 해보자.

```python
import jwt
import datetime

from cProfile import Profile
from pstats import Stats


# JWT 토큰을 생성하는 함수
def create_jwt_token(private_key):
    # 개인 키를 파일에서 읽어옵니다.
    payload = {
        "user_id": 123,
        "exp": datetime.datetime.now() + datetime.timedelta(hours=1),
    }
    token = jwt.encode(payload, private_key, algorithm="RS256")
    return token


# JWT 토큰을 검증하는 함수
def verify_jwt_token(token, public_key):
    try:
        # 공개 키를 파일에서 읽어옵니다.
        payload = jwt.decode(token, public_key, algorithms=["RS256"])
        return payload
    except jwt.ExpiredSignatureError:
        print("토큰의 유효 기간이 만료되었습니다.")
    except jwt.InvalidTokenError:
        print("유효하지 않은 토큰입니다.")


def test():
    with open("private_key.pem", "r") as f:
        private_key = f.read()
    with open("public_key.pem", "r") as f:
        public_key = f.read()

    for _ in range(100):
        token = create_jwt_token(private_key)
        decoded = verify_jwt_token(token, public_key)


profiler = Profile()
profiler.enable()
test()
profiler.disable()
stats = Stats(profiler).strip_dirs()
stats.sort_stats("cumulative", "filename")
stats.print_stats()

```

RS256 방식의 성능은 아래와 같다.

| count | type   | cumulative(sec) | per(sec) |
| ----- | ------ | --------------- | -------- |
| 10    | encode | 0.677           | 0.068    |
| 10    | decode | 0.02            | 0.002    |
| 100   | encode | 6.713           | 0.067    |
| 100   | decode | 0.037           | 0        |
| 1000  | encode | 67.24           | 0.067    |
| 1000  | decode | 0.191           | 0        |

**HMAC 방식과 인코딩 시간이 10배 넘는 차이가 발생하는 것을 확인할 수 있다.** 따라서 RS256 방식은 명확히 서버의 신원을 보장해야하는 경우가 아닌 경우에 사용하면 큰 오버헤드를 야기할 수 있다. **또한 디코딩도 명확히 느리기 때문에 모든 로그인 필요 API에 RS256 검증 과정을 추가하면 속도 저하를 야기 할 수 있다.**

>[!RS256의 인코딩은 느리다]
>RS256은 토큰 발급 오버헤드가 심하다. 따라서 RS256을 사용한다면 토큰 발급 주기를 신중히 조절할 필요가 존재한다.

___
### 리프레시 토큰과 엑세스 토큰

로그인 검증을 위해 매번 아이디와 비밀번호를 전송하는 것은 보안적으로 취약점을 갖기 때문에 우리는 토큰이나 세션 아이디를 통해 서버에 검증 요청을 전송했다. 문제점은 이러한 **토큰이나 세션 정보를 탈취당할 경우 아이디, 비밀번호를 도난당한 것과 동일한 현상이 발생한다는 것이다.** (해당 정보로 무제한 이용이 가능하므로)

이에 따라 세션 로그인의 경우 일반적으로 세션에 만료 시간을 설정해 특정한 기간이 지나면 서버에서 세션을 만료 시키고 다시금 로그인을 진행하게 하는 방식으로 동작한다.

**토큰의 경우 상태를 서버가 가지고 있지 않기 때문에 세션과 동일한 방식으로 처리할 수는 없고 토큰 내부에 유효기간 데이터를 심어 발급하는 형태로 유효기간을 설정**한다. 위의 토큰 발급 부분을 다시보자.

```python hl:5
def create_jwt_token(private_key):
    # 개인 키를 파일에서 읽어옵니다.
    payload = {
        "user_id": 123,
        "exp": datetime.datetime.now() + datetime.timedelta(hours=1),
    }
    token = jwt.encode(payload, private_key, algorithm="RS256")
    return token
```

살펴보면 실제로 토큰 내부에 만료 시간이 설정돼 있는 것을 확인할 수 있다. 해당 정보는 서버의 암호 키로 해싱돼 JWT의 페이로드와 서명에 저장된다. 서버는 토큰을 디코딩할 때 토큰의 만료시간을 파악하고 만약 만료 됐을 경우 유저가 토큰을 재발급 받게 한다. 이런 식으로 구조를 가져갈 경우 세션 로그인과 마찬가지로 **특정한 토큰의 유효기간을 설정해 토큰을 탈취 당했을 때의 위험을 최소화 할 수 있다.**

토큰에 만료기한을 설정함으로써 우리는 아이디와 비밀번호를 노출하지 않고 탈취 당했을 때의 리스크가 그나마 적은? 인증 수단을 얻었다. 이를 통해 우리는 서버에게 신원을 인증하고 서비스를 이용하는게 가능해진다. **==이렇게 서버의 서비스에 접근하기 위해 사용하는 토큰을 엑세스 토큰이라하고 엑세스 토큰은 유효기간이 짧다. (1~3시간)==**

엑세스 토큰만을 사용해 서비스를 사용해도 보안적으로는 큰 문제가 없다. 하지만 굉장히 번거로운 점이 하나 발생하는데 **바로 토큰이 만료될 때마다 로그인을 다시 진행해야 한다는 것이다.** 로그인 또한 서비스를 이용함에 있어 하나의 장벽이고 요즘 들어서는 아이디, 비밀번호를 명확히 기억하는 경우도 적기 때문에 **유저를 로그인에 자주 노출 시키는 행위는 유저 이탈과 곧장 이어질 수 있다.**

**==이에따라 엑세스 토큰을 갱신하기 위해 사용하는 별도의 유효기간이 긴 토큰이 필요해졌고 이것이 리프레시 토큰이다.==**

리프레시 토큰은 일반적으로 로그인을 했을 때 발급된다. 클라이언트는 리프레시 토큰을 저장하고 있다가 사용하는 엑세스 토큰이 만료 됐을 경우 리프레시 토큰을 서버에 전송하고 엑세스 토큰을 갱신한다. 만약 리프레시 토큰도 만료될 경우 로그인을 진행한다.

![https://colabear754.tistory.com/179|500](https://obs3dian.s3.ap-northeast-2.amazonaws.com/%EB%A1%9C%EA%B7%B8%EC%9D%B8%EA%B3%BC%20JWT%20/%20Pasted%20image%2020240806164111.png)

이런 플로우로 진행할 경우 **엑세스 토큰이 만료될 때마다 리프레시 토큰을 활용해 엑세스 토큰을 갱신하기 때문에 로그인을 자주 진행하는 현상을 방지할 수 있다**. 

>[!리프레시 토큰의 주기]
>**리프레시 토큰은 일반적으로 2달~3달 정도의 주기를 갖는다.**

___
### 리프레시 토큰 만들기

리프레시 토큰을 관리하는 방법은 여러가지이다. redis를 활용해 캐시 서버에 저장하는 방식도 존재하고 RDB에 저장하는 방식도 있다...

근데 굳이 DB에 물려야 할까..?
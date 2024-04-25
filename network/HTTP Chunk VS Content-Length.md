### 출처

* [인코딩 VS 컨텐츠 길이](https://gist.github.com/CMCDragonkai/6bfade6431e9ffb7fe88)
* [인코딩 VS 컨텐츠 길이 (스오플)](https://stackoverflow.com/questions/2419281/content-length-header-versus-chunked-encoding)
* [청크 인코딩](https://www.ioriver.io/terms/http-chunked-encoding)
* [http send big file](https://cabulous.medium.com/how-http-delivers-a-large-file-78af8840aad5)
___
### 개요
* [[#Chunking이란?]]
* [[#Chunking 코드 살펴보기]]
* [[#Content-Length]]
* [[#그래서 뭐가 더 좋아요?]]
* [[#POST 바디 제한 (+nginx)]]
* [[#nginx 실험 해보기]]
___
### Chunking이란?

![](https://newsimg.hankookilbo.com/cms/articlerelease/2021/05/20/9ddb25a9-a67f-4be2-aa31-36247b2961d8.jpg)

Chunking은 **HTTP에서 [[데이터 단위#스트림과 버퍼|스트리밍]]을 구현하기 위해 활용하는 기능으로 HTTP의 바디 데이터를 작은 조각으로 쪼개 연속으로 전송하는 기법을 말한다.** 청킹은 **전송하는 메시지의 크기를 정확히 파악하지 못하는 경우 주로 활용되며 `Content-Length` 헤더가 명시돼지 않았을 경우 자동으로 실행된다.**

청킹을 활용할 경우 메모리에 전체 파일을 적재하지 않고 부분적으로만 업로드를 진행해 메모리 사용량을 단축할 수 있다는 장점이 존재한다. **문제점은 청크를 쪼갤 수록 메모리 IO가 자주 발생하며 오버헤드가 발생할 수도 있다는 점이다. 따라서 청킹을 진행한다면 되도록 큰 크기의 파일을 전송할 때 활용하는 것이 유리하다.**

청킹을 진행하고나면 메시지가 아래와 같은 형태로 생성된다. 앞의 숫자는 바이트 수를 의미하며 전송을 마칠 때는 0을 전송해 통신의 끝을 고지한다. **청킹을 통한 전송은 하나의 HTTP 리퀘스트를 통해 처리되며 각 청크가 별도의 HTTP 리퀘스트를 통해 전송 되진 않는다.**

```bash
4\r\n
Wiki\r\n
5\r\n
pedia\r\n
e\r\n
 in\r\n\r\nchunks.\r\n
0\r\n
\r\n
```

청킹은 위에서 정의된 **청크 단위로 데이터를 곧장 전송하기 때문에 청킹은 통신을 빠르게 시작하는 것이 가능하다. 만약 데이터를 되도록 빠르게 전송하고 처리할 필요가 있다면, 청킹을 활용하는 것이 유리**하다.

**청킹은 전송해야할 전체 크기의 파일을 모르기 때문에 웹서버가 요청을 처리하거나 사용자가 직관적으로 전송 진행률을 파악할 수 없다**. 따라서 전송하는 파일의 크기를 파악하기 어려운 경우 (여러 파일을 압축해서 전송하는 경우)가 아닌 이상 되도록 Content-Length를 활용하는 것이 더욱 유리하다.

> [!info]
>**청킹은 HTTP 레벨에서 [[데이터 단위#스트림과 버퍼|스트리밍]]을 구현하는 방법이지만, 위와 같은 이슈들로 인해 되도록 Content-Length를 사용하는 것을 선호한다.** 

별도의 이야기지만 네이버에선 메인 페이지를 로딩할 때 청킹을 전혀 활용하고 있지 않다.
___
### Chunking 코드 살펴보기

아래는 실제 urllib3에서 사용하는 파일 전송을 위한 청킹 코드이다. 

```python
# 청크 제네레이터 생성 파트 (request.py)
 def chunk_readable() -> typing.Iterable[bytes]:
	nonlocal body, blocksize
	encode = isinstance(body, io.TextIOBase)
	while True:
		datablock = body.read(blocksize) #블록 사이즈 만큼 데이터 읽기
		if not datablock:
			break
		if encode:
			datablock = datablock.encode("iso-8859-1")
		yield datablock

chunks = chunk_readable()
content_length = None

#생성된 청크 전송파트 (connections.py)
 for chunk in chunks:
	# Sending empty chunks isn't allowed for TE: chunked
	# as it indicates the end of the body.
	if not chunk:
		continue
	if isinstance(chunk, str):
		chunk = chunk.encode("utf-8")
	if chunked:
		self.send(b"%x\r\n%b\r\n" % (len(chunk), chunk)) #청크 단위 전송
	else:
		self.send(chunk) 

```

확인해보면 실제로 제네레이터의 형태로 부분적으로만 데이터를 읽어 청크를 생성한 후 청크가 만들어질 때 마다 곧장 전송을 진행하는 모습을 확인할 수 있다. 따라서 ==**청킹을 진행할 경우 메모리 사용량을 단축할 수 있고 청크 단위로 곧장 전송하기 때문에 더욱 빠른 송신이 가능해진다**==는 것을 확인해 볼 수 있다.
___
### Content-Length
![https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQjks2IeyZYgx9rKYsiQF6FH0H8DTqcbd-0ie7JkIOSZQ&s](https://my-study.s3.ap-northeast-2.amazonaws.com/HTTP%20Chunk%20VS%20Content-Length%20/%20Pasted%20image%2020240408004218.png)

**컨텐츠 길이 헤더는 전송할 컨텐츠의 길이를 명시하는 헤더**로 서버나 클라이언트는 자신이 송,수신해야하는 데이터의 크기를 이를 통해 파악할 수 있다. 이를 명시해주면 **클라이언트나 서버는 자신의 통신이 언제 끝날지 직관적으로 파악할 수 있다.** [[HTTP Connection#지속 커넥션과 Content-Length|컨텐츠 길이와 크롬 무한 로딩]]

컨텐츠 길이를 활용할 경우 다음과 같은 이점을 가질 수 있다.
* 통신할 데이터의 크기를 알기 때문에 미리 적절한 처리를 구현할 수 있다.
* [Range](https://en.wikipedia.org/wiki/Byte_serving) 헤더를 활용해 전송 일시정지나 전송 재개를 구현할 수 있다.

<span class="red red-bg">컨텐츠 길이 헤더의 가장 큰 장점은 길이를 통한 적절한 대비를 진행할 수 있다는 것이다.</span> 만약 전송하려는 파일의 크기가 너무 클 경우 서버에선 실질적인 요청 처리 이전에 거절할 수 있고, 소요시간 등을 짐작하는 것도 가능하다. 클라이언트의 경우에도 파일을 업로드하거나 전송할 때 현재 전송 진행률을 파악할 수 있다. 

**컨텐츠 길이 헤더의 장점은 통신 데이터에 대한 정보를 제공함으로써 선험적으로 대비할 수 있게 하는 것이다.**

반면 **컨텐츠 길이 헤더의 단점은 파일의 길이를 전송 이전에 파악하고 있어야 한다는 점**이다. 전송하려는 데이터가 정적 파일이면 상관 없지만, 동적으로 생성되는 파일일 경우 파일의 길이를 파악하기에 어려움이 발생할 수가 있다. 또한 전송할 데이터의 **크기가 명확히 정해져야 송신을 시작 하므로 크기를 몰라도 송신을 진행할 수 있는 청킹 기법에 비해 약간의 딜레이가 존재할 수도 있다.**

추가적으로 ==**청킹과 달리 버퍼가 전송 하려는 파일 크기만큼 커야 한다는 단점이 존재**==한다. 청킹과 컨텐츠 길이 방식이 전송하는 **리퀘스트 메시지 자체의 크기는 큰 차이가 없지만 (청킹 데이터 정도의 차이) 컨텐츠 길이는 전체 파일을 읽고 모든 내용이 포함된 리퀘스트 메시지를 작성해야 하므로 메모리 사용량이 높은 편**이다.
____
### 그래서 뭐가 더 좋아요?

예상 했겠지만, 정답은 없다. 적절한 방법을 그때 그때 활용하는 것이 최선이라고 생각한다. 이전까지의 이야기들을 정리하며 사례를 들어보자.

* 청크 기법
	* 청크 크기만큼 쌓이면 곧장 전송이 가능해 응답성이 높다.
	* 파일 전체를 메모리에 업로드 할 필요가 없다.
	* 컨베이어 벨트를 타고 끊임 없이 들어오는 박스와 같다.

	* 통신의 상태를 짐작할 수 없음 (얼만큼 왔는지.. 언제 끝날지..)
	* 상태를 파악할 수 없기 때문에 통신 오류시 복구에 어려움이 있음
	* 청킹 오버헤드가 존재한다. (데이터 자르고, 일부분만 전송하고..)

* 컨텐츠 길이 기법
	* 전송 상태를 파악하기 용이하다.
	* 청킹으로 인한 오버헤드가 없다. 
	* 한번에 묶어서 전송하는 컨테이너와 같다.

	* 파일을 다 읽고 파악해야 전송이 가능하다.
	* 메모리 사용량이 높다.


청킹의 유용한 경우는 빠른 전송이 가장 우선시 되는 경우이다. 롤 같은 게임을 구현하다고 생각해보자. 프로그램은 1초에 한번 씩 서버에 캐릭터의 좌표와 유저의 입력 값을 전송한다. 이때 유저가 손이 빠른 사람이라 1초 사이에 Q-평-E-평-R-Q-평을 전부 넣었다고 가정해보자. 

청킹을 활용한다면, 모션 별로 청크 단위를 구분해 모션이 발생했을 때마다 데이터를 전송하는 방식으로 처리할 수 있다. 서버는 모션 단위 입력을 곧장 전송 받고 바로 게임에 반영한다. 따라서 실시간 성이 최대한 보장되는 플레이가 가능해진다.

반대로 컨텐츠 길이 방식을 활용한다면, 각 모션의 데이터를 전부 읽고 압축해서 길이를 측정한 다음 한번에 전송을 할 것이다. 이후 서버는 한번에 데이터를 읽고 처리를 진행할 것이다. 따라서 운이 나쁘면 내가 넣은 콤보 전체가 처리되는데 1초 이상 소요될 수도 있다.

>[!info]
>**실 시간성과 빠른 응답성이 요구된다면 청킹을 선택하는게 좋다. 일반적인 정적 파일 전송이면 파일이 지나치게 크지 않은 경우 보통 컨텐츠 길이 방식이 더 적합하다.** 

___
### POST 바디 제한 (+nginx)

HTTP의 바디 전송 방식에 대한 고민을 하다 보니 위와 같은 내용이 궁금 해졌다. 특히 **청킹의 경우 전송할 파일 크기를 명확히 설정하지 않았는데 이런 방식은 웹서버에 부하를 주지 않는지 확인하고 싶어졌다.** 

우선적으로 HTTP 자체에서 이러한 요청에 대한 제한 조치를 해뒀을 것이라 생각했지만, 놀랍게도 **HTTP 자체에서 헤더나 바디에 크기 제한을 두고 있지는 않았다.** [참고](https://serverfault.com/questions/151090/is-there-a-maximum-size-for-content-of-an-http-post)
따라서 이론 상 서버는 한번에 100GB 짜리 파일을 서버로 전송하는 것이 가능해야 한다. 하지만 당연하게도 **이러한 통신 방식은 서버에 엄청난 부하를 발생 시키므로 웹서버 자체에서 리퀘스트당 수신할 수 있는 헤더의 크기나 바디의 크기 등을 옵션으로 지정해 관리**한다.

nginx 공식 문서를 읽으면 웹 서버에서 이러한 설정을 어떻게 관리하는지 살펴볼 수 있다. 일단 nginx 자체에서 기본으로 설정하는 리퀘스트당 최대 사용 가능한 바디 크기는 1MB이다. 이를 늘리고 싶다면 설정 파일을 수정하면 되는데 크기에 제한은 딱히 없다. 하지만 크기 제한을 너무 넉넉하게 설정할 경우 서버의 메모리가 전부 터져 뻗는 상황이 발생할 수 있다. [참고](https://dewble.tistory.com/entry/nginx-config-client-max-body-size)

> [!info]
> 웹 서버는 요청당 수신할 수 있는 최대 바디 사이즈 등을 관리하고 해당 사이즈를 넘어가면 전송이 실패한다. 이에 따라 엄청난 크기의 파일을 청킹해 공격하는 방식을 방어할 수 있다.

___
### nginx 실험 해보기

테스트 재현을 위한 nginx 설정은 다음과 같다.  FastAPI 포워딩을 위한 설정과 클라이언트 리퀘스트 최대 값 설정만 신경 써주면 된다.

```conf
 upstream backend{
      server localhost:8000;
    }


  server {
        listen       80;
        server_name  localhost;

        client_max_body_size 3M;
```

nginx 서버의 최대 수신 크기를 넘어서는 파일을 전송 해본다. 체크 해보고 싶은 요소는 2 가지이다. 

* 컨텐츠 길이와 청킹의 동작 차이 확인
* 실제로 연결 오류가 나는지 확인

우선 파일 업로드를 위한 API 서버 코드부터 작성하자 FastAPI를 활용해 초 간단한 파일 업로드 API를 작성했다.

```python
from fastapi import FastAPI, File, UploadFile

app = FastAPI()

@app.get("/")
async def index():
    return {"msg": "Hello World"}

@app.post("/files/")
async def create_file(file: bytes = File()):
    return {"file_size": len(file)}

@app.post("/uploadfile/")
async def create_upload_file(file: UploadFile):
    return {"filename": file.filename}

```

테스트를 위한 시스템 구조는 nginx를 리버스 프록시로 FastAPI 서버 앞단에 놓는 방식으로 설계돼 있다.
user <-> nginx:80 <--> FastAPI:8000 의 형태라고 생각하면 된다.

따라서 모든 요청은 곧장 FastAPI로 전달되기 이전에 nginx를 거쳐서 전달된다. 

이제 청킹을 진행해보자! 청킹을 활용해 전송을 진행하는 클라이언트 코드는 아래와 같다.

```python
from http.client import HTTPConnection
import time


def do_content_length(conn, body):
    data = bytearray()
    for line in body.readlines():
        for b in line:
            data.append(b)
    conn.putheader("Content-Length", f"{len(data)}")
    conn.endheaders()
    conn.send(data)


def chunk_readable(body, blocksize):
    while True:
        datablock = body.read(blocksize)
        if not datablock:
            break
        yield datablock


def do_chunk(conn, body):
    conn.putheader("Transfer-Encoding", "chunked")
    conn.endheaders()

    chunks = chunk_readable(body, 1024)
    for i, chunk in enumerate(chunks):
        print(f"chunking... size: {i * 1024} bytes")
        # if i > 4000:
        # time.sleep(1) #연결 종료 확인을 위해
        conn.send(b"%x\r\n%b\r\n" % (len(chunk), chunk))

    conn.send(b"0\r\n\r\n")
    conn.send(body)


conn = HTTPConnection(host="127.0.0.1", port=80)
conn.connect()
conn.putrequest(method="POST", url="/files")


body = open(
    "/Users/jin/Library/Mobile /test.gif",
    "rb",
)

do_chunk(conn, body)
do_content_length(conn, body)
print(conn.getresponse().read())

```

청킹을 통한 전송을 진행하면 부분적인 데이터가 계속해서 nginx에 전송된다. nginx는 설정에 `client_max_body_size` 라는 속성 값을 갖는데 여기에 클라이언트의 단독 리퀘스트로 부터 최대 수신할 수 있는 데이터의 양을 지정할 수 있다. 

**nginx는 청크 데이터를 연속적으로 수신하다 적재된 데이터가 설정 값을 초과하는 순간 413 에러 메시지를 클라이언트에게 전송한다.** 위의 코드에서 이 과정은 클라이언트의 전송 과정 도중에 발생하며, 초과하는 순간 연결을 종료한다.
```html
b'<html>\r\n<head><title>413 Request Entity Too Large</title></head>\r\n<body>\r\n<center><h1>413 Request Entity Too Large</h1></center>\r\n<hr><center>nginx/1.25.4</center>\r\n</body>\r\n</html>\r\n'
```

연결이 종료되는 과정은 아래의 이미지를 보면 확인할 수 있다. 확인해보면 서버의 설정 값 크기를 넘어가는 순간 연결이 곧장 종료되는 것을 확인할 수 있다.
![](https://my-study.s3.ap-northeast-2.amazonaws.com/HTTP%20Chunk%20VS%20Content-Length%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-04-09%20%EC%98%A4%EC%A0%84%2011.16.11.png)

이에 따라 nginx는 설정 값 이상의 리퀘스트 데이터를 메모리에 로딩하는 상황이 발생하지 않으므로 Ddos와 같은 공격에 강건할 수 있게된다. 

wireshark를 통해 확인해봐도, 서버의 413 에러가 클라이언트의 리퀘스트가 전부 전송되기 이전에 미리 와있는 것을 확인할 수 있다. 

![](https://my-study.s3.ap-northeast-2.amazonaws.com/HTTP%20Chunk%20VS%20Content-Length%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-04-09%20%EC%98%A4%EC%A0%84%2010.51.40.png)

nginx 로그는 다음과 같이 기록된다.  

```
2024/04/08 23:31:54 [error] 30210#0: *56 client intended to send too large chunked body: 3145700+100 bytes (이 숫자가 옵션 값에 따라 변화한다), client: 127.0.0.1, server: localhost, request    : "POST /files HTTP/1.1", host: "127.0.0.1"
```

**청킹의 특이한 점은 어느 정도 통신을 진행 해야만 파일의 크기를 파악할 수 있다는 것이다.** 이는 청킹의 특성상 nginx가 클라이언트가 전송하고자 하는 바디의 크기를 곧 바로 특정할 수 없기 때문이다. 따라서 택도 없이 큰 파일이여도 nginx의 설정 값 까지는 전송을 진행 해야만 요청을 거절할 수 있다.

이러한 부분을 더욱 잘 관리하고 설정하기 위해서는 nginx의 버퍼링을 추가적으로 찾아보면 좋다.

#### Content-Length 예제
컨텐츠 길이를 활용하는 방법은 단순하다. 청크 인코딩 헤더를 제거하고 다음과 같은 헤더를 활용하면 된다.
`conn.putheader("Content-Length", "75000000")` 이후 전송을 진행하면 바디를 다 수신하기 이전에 곧장 nginx로 부터 거절 당하는 현상을 확인할 수 있다.

**컨텐츠 길이를 활용하면 웹서버가 리퀘스트의 크기를 바로 파악할 수 있으므로 빠르게 반응하고 처리하는 것이 가능한 모습을 확인**할 수 있다. 또한 하나의 리퀘스트 메시지 안에 전체 내용이 포함돼야 하므로 클라아인트 측에서 파일 전체 크기만큼 메모리를 물고 있어야 한다는 특징도 확인할 수 있다.

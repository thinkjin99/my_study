### HTTPConnection 

[[HTTPConnection 부터 send까지#HTTPConnection|HTTPConnection]] 에서 우선적으로 다뤘지만, 자세히는 다루지 못해 이하에서 깊게 다루는 작업을 진행해보고자 한다. 앞서 정리했던 내용을 상기해보면, 해당 클래스는 파이썬 내장 라이브러리인 `http.client.HTTPConnection` 을 상속헤 구현되고 있다.  실질적인 HTTP 통신에서 활용되는 연결을 생성하고 메시지를 전송, 수신하며 인코딩, 디코딩 등의 작업도 수행한다.

이제 자세하게 커넥션 클래스를 분석해보자. 우리가 분석해야할 요소는 다음과 같다.
* 연결을 어떻게 생성하는가?
* 메시지를 어떻게 전송하는가?
* 메시지를 어떻게 수신하는가?
* 연결을 어떻게 종료하는가?
___
### 연결을 어떻게 생성하는가?

연결을 생성하는 코드는 아래에 대부분 정의 돼있다.
```python
    @property
    def host(self) -> str:
        """
        Getter method to remove any trailing dots that indicate the hostname is an FQDN.
            
            google.com.
            
            gooogle.com

        In general, SSL certificates don't include the trailing dot indicating a
        fully-qualified domain name, and thus, they don't validate properly when
        checked against a domain name that includes the dot. In addition, some
        servers may not expect to receive the trailing dot when provided.

        However, the hostname with trailing dot is critical to DNS resolution; doing a
        lookup with the trailing dot will properly only resolve the appropriate FQDN,
        whereas a lookup without a trailing dot will search the system's search domain
        list. Thus, it's important to keep the original host around for use only in
        those cases where it's appropriate (i.e., when doing DNS lookup to establish the
        actual TCP connection across which we're going to send HTTP requests).
        """
        return self._dns_host.rstrip(".")
 
    @host.setter
    def host(self, value: str) -> None:
        """
        Setter for the `host` property.

        We assume that only urllib3 uses the _dns_host attribute; httplib itself
        only uses `host`, and it seems reasonable that other libraries follow suit.
        """
        self._dns_host = value

    def _new_conn(self) -> socket.socket:
        """Establish a socket connection and set nodelay settings on it.

        :return: New socket connection.
        """
        try:
            sock = connection.create_connection(
                (self._dns_host, self.port),
                self.timeout,
                source_address=self.source_address,
                socket_options=self.socket_options,
            )
        except socket.gaierror as e:
            raise NameResolutionError(self.host, self, e) from e
        except SocketTimeout as e:
            raise ConnectTimeoutError(
                self,
                f"Connection to {self.host} timed out. (connect timeout={self.timeout})",
            ) from e

        except OSError as e:
            raise NewConnectionError(
                self, f"Failed to establish a new connection: {e}"
            ) from e

        # Audit hooks are only available in Python 3.8+
        if _HAS_SYS_AUDIT:
            sys.audit("http.client.connect", self, self.host, self.port)

        return sock
```

우선적으로 눈여겨봐야할 부분은 host인데 사실 커넥션 클래스 자체는 host 속성을 갖고 있지 않다. 실질적인 host 속성은 부모 클래스에 지정 돼있다. 또한 host 속성을 프로퍼티로 정의해 setter에서 host를 수정할 경우 `_dns_host` 의 값을 변경하게 된다. 따라서 초창기 커넥션 객체를 생성하기 위해 host를 전달하면 `__init__` 메서드가 실행되며 부모 클래스를 생성하는데, 이때 프로퍼티의 setter가 오버라이딩 돼 host 속성이 아닌 `_dns_host` 가 초기화 된다.

정리하자면 host 속성을 부모 클래스를 생성하며 초기화 하는데 이때 오버라이딩된 setter가 동작해 실질적으로는 `_dns_host`의 속성 값이 초기화 된다.

이렇게 번잡한 작업을 수행한 이유는 전부 [[FQDN]] 때문이다. 종종 FQDN의 호스트와 일반적인 URL의 호스팅을 다르게 하는 서비스들이 존재해 이에 대응하고자 이런 식으로 호스트 속성을 관리한다. (exmaple.com과 example.com.이 다른 ip를 갖는 경우가 있다고 한다...) [참고](https://github.com/urllib3/urllib3/pull/1255)
____
### HTTP Connection의 메시지 전송

연결을 생성한 후 메시지의 전송은 모두 `request` 함수에서 진행된다. 아래는 함수의 구현부이다.

```python
	# `request` method's signature intentionally violates LSP.
    # urllib3's API is different from `http.client.HTTPConnection` and the subclassing is only incidental.
    def request(  # type: ignore[override]
        self,
        method: str,
        url: str,
        body: _TYPE_BODY | None = None,
        headers: typing.Mapping[str, str] | None = None,
        *,
        chunked: bool = False,
        preload_content: bool = True,
        decode_content: bool = True,
        enforce_content_length: bool = True,
    ) -> None:
        # Update the inner socket's timeout value to send the request.
        # This only triggers if the connection is re-used.
        if self.sock is not None:
            self.sock.settimeout(self.timeout)

        # Store these values to be fed into the HTTPResponse
        # object later. TODO: Remove this in favor of a real
        # HTTP lifecycle mechanism.

        # We have to store these before we call .request()
        # because sometimes we can still salvage a response
        # off the wire even if we aren't able to completely
        # send the request body.
        self._response_options = _ResponseOptions(
            request_method=method,
            request_url=url,
            preload_content=preload_content,
            decode_content=decode_content,
            enforce_content_length=enforce_content_length,
        )

        if headers is None:
            headers = {}
        header_keys = frozenset(to_str(k.lower()) for k in headers)
        skip_accept_encoding = "accept-encoding" in header_keys
        skip_host = "host" in header_keys
        self.putrequest(
            method, url, skip_accept_encoding=skip_accept_encoding, skip_host=skip_host
        )

        # Transform the body into an iterable of sendall()-able chunks
        # and detect if an explicit Content-Length is doable.
        chunks_and_cl = body_to_chunks(body, method=method, blocksize=self.blocksize)
        chunks = chunks_and_cl.chunks
        content_length = chunks_and_cl.content_length

        # When chunked is explicit set to 'True' we respect that.
        if chunked:
            if "transfer-encoding" not in header_keys:
                self.putheader("Transfer-Encoding", "chunked")
        else:
            # Detect whether a framing mechanism is already in use. If so
            # we respect that value, otherwise we pick chunked vs content-length
            # depending on the type of 'body'.
            if "content-length" in header_keys:
                chunked = False
            elif "transfer-encoding" in header_keys:
                chunked = True

            # Otherwise we go off the recommendation of 'body_to_chunks()'.
            else:
                chunked = False
                if content_length is None:
                    if chunks is not None:
                        chunked = True
                        self.putheader("Transfer-Encoding", "chunked")
                else:
                    self.putheader("Content-Length", str(content_length))

        # Now that framing headers are out of the way we send all the other headers.
        if "user-agent" not in header_keys:
            self.putheader("User-Agent", _get_default_user_agent())
        for header, value in headers.items():
            self.putheader(header, value)
        self.endheaders()

        # If we're given a body we start sending that in chunks.
        if chunks is not None:
            for chunk in chunks:
                # Sending empty chunks isn't allowed for TE: chunked
                # as it indicates the end of the body.
                if not chunk:
                    continue
                if isinstance(chunk, str):
                    chunk = chunk.encode("utf-8")
                if chunked:
                    self.send(b"%x\r\n%b\r\n" % (len(chunk), chunk))
                else:
                    self.send(chunk)

        # Regardless of whether we have a body or not, if we're in
        # chunked mode we want to send an explicit empty chunk.
        if chunked:
            self.send(b"0\r\n\r\n")
```

함수는 LSP를 지키지 않는다는 충격적인 내용으로 시작하는데 이는 곧 부모 메서드의 기능을 자식 메서드에서 보장하지 않음을 의미한다. 이는 파이썬 내장 라이브러리인 `http.client` 를 상속 받았지만, 별개의 기능을 하기 위함 이라는데 솔직히 잘 모르겠다. (이럴거면 다른 명칭을 붙이는게 맞지 않나?)

함수가 수행하는 작업은 다음과 같다.
* 요청 메시지 파싱 및 인코딩
* 청크 단위 전송이 필요한 경우 바디를 청크 단위로 분할
* 헤더 파싱 및 인코딩
* 바디 파싱 및 인코딩
* 인코딩된 데이터를 소켓의 버퍼에 저장 및 전송

#### 요청 메시지 파싱 및 인코딩
요청 메시지의 파싱은 아래 부분의 `put_request` 에서 진행된다.
```python
if headers is None:
        headers = {}
        header_keys = frozenset(to_str(k.lower()) for k in headers)
        skip_accept_encoding = "accept-encoding" in header_keys
        skip_host = "host" in header_keys
        self.putrequest(
            method, url, skip_accept_encoding=skip_accept_encoding, skip_host=skip_host
        )
```

`put_request`는 `http.client`를 오버 라이딩하는 함수로 매개변수를 HTTP 요청 메시지 형태로 인코딩해 소켓 버퍼에 넣어준다. 이때 전송을 진행하지는 않고 버퍼에만 값을 보관한다는 것을 주의하자. 해당 함수를 수행하고 나면 아래와 같이 **`Host` 헤더 정보 까지만 입력된 가장 기초적인 요청 메시지가 소켓 버퍼에 저장된다.**

```http
GET /index.html HTTP/1.1
Host: www.example.com
```

#### 청크 단위 전송이 필요한 경우 바디를 청크 단위로 분할
이후 만약 청크 단위의 전송이 필요하다면, 바디를 청크 단위로 분할하는 작업을 수행한다. 코드는 아래와 같다.

```python
chunks_and_cl = body_to_chunks(body, method=method, blocksize=self.blocksize)
chunks = chunks_and_cl.chunks
content_length = chunks_and_cl.content_length

# When chunked is explicit set to 'True' we respect that.
if chunked:
	if "transfer-encoding" not in header_keys:
		self.putheader("Transfer-Encoding", "chunked")
else:
	# Detect whether a framing mechanism is already in use. If so
	# we respect that value, otherwise we pick chunked vs content-length
	# depending on the type of 'body'.
	if "content-length" in header_keys:
		chunked = False
	elif "transfer-encoding" in header_keys:
		chunked = True

	# Otherwise we go off the recommendation of 'body_to_chunks()'.
	else:
		chunked = False
		if content_length is None:
			if chunks is not None:
				chunked = True
				self.putheader("Transfer-Encoding", "chunked")
		else:
			self.putheader("Content-Length", str(content_length))

```

함수는 사용자가 데이터를 전송한다고 가정할 때 분기를 크게 4가지로 구분한다.
1. 청크 단위로 분할 전송할 것을 명시한 경우
	* 이 경우 `Transfer-Encoding` 헤더를 추가한다.
2. Content-Length 헤더가 존재하는 경우
	* 이 경우 청킹을 실시하지 않는다.
3. 인코딩 헤더가 존재하는 경우
	* 이 경우 청킹을 실시한다.
4. 어떻게 전송할지 아무것도 명시하지 않은 경우
	* 만약 바디의 크기를 특정할 수 없을 경우 청킹을 실시한다.

4번의 경우만 좀 더 살펴보자. 4번 케이스로 넘어가려면 `body_to_chunks`의 실행 결과에서 컨텐츠 길이가 None으로 도출돼야 한다. 아래는 `body_to_chunks`의 코드이다.

```python
def body_to_chunks(
    body: typing.Any | None, method: str, blocksize: int
) -> ChunksAndContentLength:
    """Takes the HTTP request method, body, and blocksize and
    transforms them into an iterable of chunks to pass to
    socket.sendall() and an optional 'Content-Length' header.

    A 'Content-Length' of 'None' indicates the length of the body
    can't be determined so should use 'Transfer-Encoding: chunked'
    for framing instead.
    """

    chunks: typing.Iterable[bytes] | None
    content_length: int | None

    # No body, we need to make a recommendation on 'Content-Length'
    # based on whether that request method is expected to have
    # a body or not.
    if body is None:
        chunks = None
        if method.upper() not in _METHODS_NOT_EXPECTING_BODY:
            content_length = 0
        else:
            content_length = None

    # Bytes or strings become bytes
    elif isinstance(body, (str, bytes)):
        chunks = (to_bytes(body),)
        content_length = len(chunks[0])

    # File-like object, TODO: use seek() and tell() for length?
    elif hasattr(body, "read"):
        def chunk_readable() -> typing.Iterable[bytes]:
            nonlocal body, blocksize
            encode = isinstance(body, io.TextIOBase)
            while True:
                datablock = body.read(blocksize) #블락 사이즈로 파일을 끊어서 처리한다
                if not datablock:
                    break
                if encode:
                    datablock = datablock.encode("iso-8859-1")
                yield datablock #제네레이터로 데이터 블락을 계속 반환한다.

        chunks = chunk_readable()
        content_length = None

    # Otherwise we need to start checking via duck-typing.
    else:
        try:
            # Check if the body implements the buffer API.
            mv = memoryview(body)
        except TypeError:
            try:
                # Check if the body is an iterable
                chunks = iter(body)
                content_length = None
            except TypeError:
                raise TypeError(
                    f"'body' must be a bytes-like object, file-like "
                    f"object, or iterable. Instead was {body!r}"
                ) from None
        else:
            # Since it implements the buffer API can be passed directly to socket.sendall()
            chunks = (body,)
            content_length = mv.nbytes

    return ChunksAndContentLength(chunks=chunks, content_length=content_length)
```

컨텐츠 길이가 None으로 설정되는 경우는 다음과 같다.
1. 메서드가 바디가 존재하지 않는 GET, HEAD 등의 메서드인 경우
2. 전송 하려는 데이터가 길이를 곧장 특정할 수 없는 파일 비슷한 타입인 경우
3. 바디가 메모리 뷰 타입을 지원하지 않는 이터러블 객체인 경우 (이 경우 바디의 크기를 곧장 측정할 수 없다)

다시 `request` 함수로 돌아가보면 **청크 할게 없는 1번의 경우를 제외하고 2,3의 경우 청킹을 적용해서 메시지를 전송하는 것을 확인할 수 있다.** 

```python
	# Otherwise we go off the recommendation of 'body_to_chunks()'.
	else:
		chunked = False
		if content_length is None:
			if chunks is not None:
				chunked = True
				self.putheader("Transfer-Encoding", "chunked")
		else: #컨텐츠의 길이를 명확히 파악한 경우
			self.putheader("Content-Length", str(content_length))
```

흥미롭게 생각해 볼만한 부분은 아래의 부분인데, 전송할 바디의 크기를 명확히 파악하지 못해 청킹으로 전송하는 방식을 길이를 명확히 파악해 전송하는 방식으로 변환하고자 하는 TODO가 작성 돼있다.

```python
    # File-like object, TODO: use seek() and tell() for length?
    elif hasattr(body, "read"):
        def chunk_readable() -> typing.Iterable[bytes]:
            nonlocal body, blocksize
            encode = isinstance(body, io.TextIOBase)
            while True:
                datablock = body.read(blocksize)
                if not datablock:
                    break
                if encode:
                    datablock = datablock.encode("iso-8859-1")
                yield datablock #제네레이터로 데이터 블락을 계속 반환한다.

        chunks = chunk_readable()
        content_length = None
```

___
### 답답해서 내가 뛴다 청킹 고쳐보기

청킹 로직을 보며 느낀 가장 큰 문제점은 코드가 예상한대로 동작하지 않는다는 것이였다. 위의 청킹 로직을 보면 c**hunked 파라미터의 값이 false로 설정돼도 Contents-Length 헤더가 명시 돼있지 않으면 자동적으로 청킹을 진행하는 방식으로 동작**한다.

이는 모호한 동작이므로 chunked 파라미터의 값이 false로 설정돼 있을 경우 자동적으로 전송 하려는 데이터의 크기를 측정한 후 Contents-Length를 활용해 통신하는 방식으로 코드를 수정했다. 아래는 수정된 코드이다.

```python
def request(  # type: ignore[override]
        self,
        method: str,
        url: str,
        body: _TYPE_BODY | None = None,
        headers: typing.Mapping[str, str] | None = None,
        *,
        chunked: bool = False,
        preload_content: bool = True,
        decode_content: bool = True,
        enforce_content_length: bool = True,
    ) -> None:
        # Update the inner socket's timeout value to send the request.
        # This only triggers if the connection is re-used.
        if self.sock is not None:
            self.sock.settimeout(self.timeout)

        # Store these values to be fed into the HTTPResponse
        # object later. TODO: Remove this in favor of a real
        # HTTP lifecycle mechanism.

        # We have to store these before we call .request()
        # because sometimes we can still salvage a response
        # off the wire even if we aren't able to completely
        # send the request body.
        self._response_options = _ResponseOptions(
            request_method=method,
            request_url=url,
            preload_content=preload_content,
            decode_content=decode_content,
            enforce_content_length=enforce_content_length,
        )

        if headers is None:
            headers = {}
        header_keys = frozenset(to_str(k.lower()) for k in headers)
        skip_accept_encoding = "accept-encoding" in header_keys
        skip_host = "host" in header_keys
        self.putrequest(
            method, url, skip_accept_encoding=skip_accept_encoding, skip_host=skip_host
        )
        # The body is sent by content-length header when chunked is False or
        # transfer-encoding header doesn't exist.
        # The sending header is only affected by chunked parameter or header
        # type of body doesn't affect to header.

        if "content-length" in header_keys:
            chunked = False

        elif "transfer-encoding" in header_keys:
            chunked = True

        chunks_and_cl = None
        content_length = None

        # Transform the body into an iterable of sendall()-able chunks

        # If No body, we need to make a recommendation on 'Content-Length'
        # based on whether that request method is expected to have
        # a body or not.

        if body is None:
            if method.upper() not in _METHODS_NOT_EXPECTING_BODY:
                content_length = 0

        elif chunked:
            chunks_and_cl = body_to_chunks(body, self.blocksize)  # use chunking

        else:
            chunks_and_cl = body_to_bytes(body, self.blocksize)
            content_length = chunks_and_cl.content_length

        # Now that framing headers are out of the way we send all the other headers.
        if "user-agent" not in header_keys:
            self.putheader("User-Agent", _get_default_user_agent())

        # Detect whether a framing mechanism is already in use. If so we respect that value.
        if chunked and "transfer-encoding" not in header_keys:
            self.putheader("Transfer-Encoding", "chunked")

        elif content_length is not None:
            self.putheader("Content-Length", str(content_length))

        for header, value in headers.items():
            self.putheader(header, value)

        self.endheaders()

        if chunks_and_cl:
            chunks = chunks_and_cl.chunks

            # If we're given a body we start sending that in chunks.
            for chunk in chunks:
                # Sending empty chunks isn't allowed for TE: chunked
                # as it indicates the end of the body.
                if not chunk:
                    continue
                if isinstance(chunk, str):
                    chunk = chunk.encode("utf-8")
                if chunked:
                    self.send(b"%x\r\n%b\r\n" % (len(chunk), chunk))
                else:
                    self.send(chunk)

        # Regardless of whether we have a body or not, if we're in
        # chunked mode we want to send an explicit empty chunk.
        if chunked:
            self.send(b"0\r\n\r\n")

```

수정을 통해 개선하고 싶었던 점은 추측을 통해 청킹을 진행하는 현상 제거하고 유저가 명시한 경우에만 진행하는 것이다. **즉, chunked라고 명확히 주어진 매개변수 값을 활용해서만 청킹의 진행 여부를 판단하게 하는 것이 이번 수정의 가장 큰 목표**였다.

이러한 수정을 진행하게 된 계기는 명확 했는데, 컨텐츠 길이를 파악할 수 있다면, 청킹보다 데이터를 덩어리로 한번에 던지는 것이 대부분 더욱 효율적이기 때문이다. ([[HTTP Chunk VS Content-Length]])

이를 구현하기 위해 수정한 부분은 단순했다. 우선 청킹을 추측하는 로직을 제거했다.
```python
if body is None:
	if method.upper() not in _METHODS_NOT_EXPECTING_BODY:
			content_length = 0

	elif chunked:
		chunks_and_cl = body_to_chunks(body, self.blocksize)  # use chunking

	else:
		chunks_and_cl = body_to_bytes(body, self.blocksize)
		content_length = chunks_and_cl.content_length

```

이전의 코드와 달리 청킹 값이 참으로 설정돼 있는 경우에만 청킹을 진행하고 아닐 경우 전송 하려는 파일의 크기를 측정하는 코드를 작성했다. 이제 chunked 변수의 값이 실제 청킹 여부를 결정한다.

이외에는 파일의 크기를 측정하는 부분을 구현할 필요가 있다.
```python
def chunk_readable(body: typing.Any, blocksize: int) ->typing.Iterable[bytes]:
    #Get file content length
    encode = isinstance(body, io.TextIOBase)
    while True:
        datablock = body.read(blocksize)
        if not datablock:
            break
        if encode:
            datablock = datablock.encode("iso-8859-1")
        yield datablock

def body_to_bytes(body: typing.Any, blocksize: int) -> ChunksAndContentLength:
    """Convert body data to sendable bytes and measures the length of it.
    If body is iterable data then iter whole data and buffer it.
    """

    if isinstance(body, (str, bytes)):
        chunks = (to_bytes(body),)
        content_length = len(chunks[0])

    elif hasattr(body, "read"):
        chunks = bytearray()
        for chunk in chunk_readable(body, blocksize):
            chunks += chunk

        chunks = (chunks,)
        content_length = len(chunks[0])

    elif hasattr(body, "__next__"):  # check iterator type
        chunks = bytearray()
        for chunk in body:
            try:
                chunks += chunk
            except TypeError:
                # if chunk is not byte
                raise TypeError(
                    f"'body' must be a bytes-like object, file-like "
                    f"object, or iterable. Instead was {body!r}"
                ) from None

        chunks = (chunks,)
        content_length = len(chunks[0])

    else:
        # Otherwise we need to start checking via duck-typing.
        try:
            mv = memoryview(body)
            chunks = (body,)
            content_length = mv.nbytes
        except TypeError:
            raise TypeError(
                f"'body' must be a bytes-like object, file-like "
                f"object, or iterable. Instead was {body!r}"
            ) from None

    return ChunksAndContentLength(chunks=chunks, content_length=content_length)

```
제네레이터를 활용해 바이트를 청크 단위로 잘라가며 파일을 읽고 이후 전체 길이를 측정해 반환하는 함수이다. 이후 해당 길이를 활용해 파일을 길이만큼 읽고 전송을 하는 방식으로 처리한다.
___
### 결과

코드 자체는 큰 문제가 없다. 테스트도 잘 통과하고 큰 이슈도 존재하지 않는다. **문제는 이를 반영할 경우 다른 라이브러리나 시스템에 혼란을 줄 수 있다는 것이다. 기존의 청킹을 기본적으로 실행하던 방식에서 이러한 방식으로 변경할 경우 큰 크기의 파일을 전송하는 서버의 경우 메모리에서 이슈가 발생할 수도 있다.**

실제로 이슈를 리젝먹은 이유를 살펴보면 이와 같은 내용들이 내포 돼 있다. [이슈](https://github.com/urllib3/urllib3/issues/3379)

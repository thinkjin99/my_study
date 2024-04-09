### HTTPConnectionPool과 HTTPConneciton

**urllib3는 HTTPConnectionPool과 HTTPConneciton 객체를 활용해 실질적인 HTTP 커넥션을 관리한다.** PoolManager는 유저가 접근하 핸들러로 기능하고 urllib3 내부에서 실제 HTTP 연결의 처리와 관리는 앞서 언급한 두개의 클래스에서 처리한다.
___
### HTTPConnectionPool

실질적인 커넥션 풀을 의미하며 쓰레드 안정성을 보장하는 커넥션 객체를 제공한다. ([[커넥션 풀이란|커넥션 풀 참고자료]])
**커넥션 풀은 특정 호스트를 향한 여러 커넥션을 관리하는 것이 목표이기 때문에 커넥션들을 저장할 공간이 요구된다. urllib3에서는 큐를 활용해 이를 관리**하는데, 아래의 초기화 코드를 보면 확인할 수 있다.

```python
def __init__(
	self,
	host: str,
	port: int | None = None,
	timeout: _TYPE_TIMEOUT | None = _DEFAULT_TIMEOUT,
	maxsize: int = 1,
	block: bool = False,
	headers: typing.Mapping[str, str] | None = None,
	retries: Retry | bool | int | None = None,
	_proxy: Url | None = None,
	_proxy_headers: typing.Mapping[str, str] | None = None,
	_proxy_config: ProxyConfig | None = None,
	**conn_kw: typing.Any,
):
	ConnectionPool.__init__(self, host, port)
	RequestMethods.__init__(self, headers)

	if not isinstance(timeout, Timeout):
		timeout = Timeout.from_float(timeout)

	if retries is None:
		retries = Retry.DEFAULT

	self.timeout = timeout
	self.retries = retries
	#Lifo 큐를 활용해 커넥션을 관리한다.
	self.pool: queue.LifoQueue[typing.Any] | None = self.QueueCls(maxsize)
	self.block = block

	self.proxy = _proxy
	self.proxy_headers = _proxy_headers or {}
	self.proxy_config = _proxy_config

	# Fill the queue up so that doing get() on it will block properly
	for _ in range(maxsize):
		self.pool.put(None)
```

호스트와 포트는 하나씩 전달 받아 동일한 엔드 포인트로의 커넥션을 여러개 생성하는 것이라는 걸 유심히 생각하자. <b><u>urllib3에서의 커넥션 풀은 하나의 엔드 포인트로의 연결을 여러개 관리하는 것이지 여러 엔드포인트로의 연결을 관리하진 않는다.</u></b>

`block`  파라미터도 유심히 볼 가치가 있는 파라미터이다. 만약 `block`의 값이 `False`로 설정될 경우 `urrlib3`는 새로운 커넥션을 만들어 요청을 처리하고 해당 요청을 재사용하지 않고 버리는 방식으로 동작한다. `maxsize` 값을 설정해도 동작중인 커넥션의 수가 순간적으로 최대 값을 뛰어 넘는 것이 가능해진다.

하지만 `block`이 `True`일 경우 커넥션 풀이 가질 수 있는 커넥션의 수가 `maxsize`를 초과할 수 없게 되고 이에 따라 **커넥션 풀의 커넥션들이 전부 사용중일 경우 신규 요청은 대기를 진행한다. 이 방식을 활용하면 특정 멀티쓰레드 환경에서 연결 시도를 동시에 너무 많이 하는 문제가 발생하는 상황 등을 억제할 수 있다.**
___
### HTTPConnectionPool.urlopen

커넥션 풀의 `urlopen`은 `RequestMethods`의 추상 메서드를 구현한 메서드이다. 해당 메서드는 커넥션 풀에서 커넥션을 추출해 실질적인 리퀘스트를 전송하는 기능을 수행한다. `urlopen` 에서 전송 작업을 모두 담당할 것처럼 보이나 실제 리퀘스트는 `_make_request` 메서드 내부에서 실행된다.

아래는 실제 코드의 일부분이다. 리퀘스트를 생성하는 메서드가 별도로 `urlopen` 내부에 존재하는 것을 확인할 수 있다.

```python
    #urlopen
    ...
    ...
    conn = self._get_conn(timeout=pool_timeout)
    ...
	...
    # If we're going to release the connection in ``finally:``, then
	# the response doesn't need to know about the connection. Otherwise
	# it will also try to release it and we'll have a double-release
	# mess.
	response_conn = conn if not release_conn else None

	# Make the request on the HTTPConnection object
	response = self._make_request(
		conn,
		method,
		url,
		timeout=timeout_obj,
		body=body,
		headers=headers,
		chunked=chunked,
		retries=retries,
		response_conn=response_conn,
		preload_content=preload_content,
		decode_content=decode_content,
		**response_kw,
	)

```

이제 `_make_request` 메서드의 내부를 살펴보자. 메서드의 내부를 살펴보면 실질적인 요청을 처리하는 부분이 또다시 `request` 라는 별도의 함수로 구분된 것을 확인할 수 있다.

```python
#_make_request
...
...

  # conn.request() calls http.client.*.request, not the method in
	# urllib3.request. It also calls makefile (recv) on the socket.
	try:
		conn.request(
			method,
			url,
			body=body,
			headers=headers,
			chunked=chunked,
			preload_content=preload_content,
			decode_content=decode_content,
			enforce_content_length=enforce_content_length,
		)
	
	# We are swallowing BrokenPipeError (errno.EPIPE) since the server is
	# legitimately able to close the connection after sending a valid response.
	# With this behaviour, the received response is still readable.
	except BrokenPipeError:
		pass
```

이곳에서 사용하는 `conn` 객체는 `HTTPConnectionPool` 에서 추출한 커넥션 객체로 `_get_conn` 메서드를 통해서 획득 가능하다. `_get_conn`은 풀의 **커넥션 큐에서 커넥션을 추출해 반환하는 방식으로 동작한다. 이때 만약 큐에 커넥션이 존재하지 않을 경우 새 커넥션을 셍성해 전달한다.** 커넥션을 생성하는 부분의 코드를 유심히 볼만한데 아래와 같다.

```python
 def _new_conn(self) -> BaseHTTPConnection:
	"""
	Return a fresh :class:`HTTPConnection`.
	"""
	self.num_connections += 1
	log.debug(
		"Starting new HTTP connection (%d): %s:%s",
		self.num_connections,
		self.host,
		self.port or "80",
	)

	conn = self.ConnectionCls( #이 클래스의 정체가 뭐냐
		host=self.host,
		port=self.port,
		timeout=self.timeout.connect_timeout,
		**self.conn_kw,
	)
	return conn
```

커넥션을 신규로 생성해 반환하는 메서드이다 보니 반환 값이 당연히 [`HTTPConnection`](https://urllib3.readthedocs.io/en/stable/reference/urllib3.connection.html#urllib3.connection.HTTPConnection "urllib3.connection.HTTPConnection") 일거라 생각했지만, **타입힌팅에 사용된 객체는 `BaseHTTPConnection` 을 사용한다. 그렇지만 막상 반환하는 객체는 `ConnectionCls`객체이다. 이건 도대체 뭐란 말인가..?**

공식 문서에서는 `ConnectionCls`가 `HTTPConnection`의 별명이라고 간략하게 소개한다. 직접 코드 선언 부분을 확인 해보자.

```python
ConnectionCls: (type[BaseHTTPConnection] | type[BaseHTTPSConnection]) = HTTPConnection
```

코드를 살펴보면 `ConnectionCls`가 `BaseHTTPConnection`의 타입을 갖고 디폴트로는 `HTTPConneciton`을 가리킨다고 지정돼 있다. 이젠 `BaseHTTPConnection과` `HTTPConnection` 이 도대체 어떤 상관 관계를 갖고 있는지를 확인해봐야 한다. 
___
### 중간 정리

머리가 복잡해지니 한번 정리를 하고 넘어가자. 우린 `PoolManager`의 `request`가 어떻게 처리 되는지 궁금 했다. 코드를 살펴보니 `request`는 `urlopen`에서 처리되는 함수였고 `urlopen`은 `HTTPConnecitonPool `객체에서 처리했다. 

커넥션 풀 객체 내부를 살펴보니 `urlopen은` 다시 `_make_request` 함수에서 처리했고 `_make_reques`t 내부에서는 커넥션 객체가 지닌 `request` 함수를 통해 다시금 처리됐다.

따라서 지금까지 상황을 요약하면 아래와 같다.

`PoolManage.request` -> `HTTPConnectionPool.urlopen` -> `HTTPConnectionPool._make_request` ->` conn.request` -> ???
___
### BaseHTTPConnection

앞서 살펴 봤듯이 ConnectionCls는 BaseHTTPConnection과 연관이 깊다. 뿐만 아니라 BaseHTTPConnection은 커넥션 풀 내부 메서드의 반환 형으로 꽤 자주 활용된다. 대체 BaseHTTPConnection은 뭐길래 온갖 곳에서 사용될까?

BaseHTTPConnection은 `_base_connection.py`에 다음과 같이 정의돼 있다.
```python
if typing.TYPE_CHECKING: #run only in typing time...
    import ssl
    from typing import Literal, Protocol

    from .response import BaseHTTPResponse

    class BaseHTTPConnection(Protocol): #duck typing check
        default_port: typing.ClassVar[int]
        default_socket_options: typing.ClassVar[_TYPE_SOCKET_OPTIONS]

        host: str
        port: int
        timeout: None | (
            float
        )  # Instance doesn't store _DEFAULT_TIMEOUT, must be resolved.
        blocksize: int
        source_address: tuple[str, int] | None
        socket_options: _TYPE_SOCKET_OPTIONS | None

        proxy: Url | None
        proxy_config: ProxyConfig | None
        ...
        ...

```

기묘한 문법들이 많으므로 한 줄씩 천천히 분석해보자. 우선적으로 `typing.TYPE_CHECKING`은 오직 타입 체킹을 위해 인터프리터가 실행되는 경우에만 참이다. 즉 런타임에는 if문 내부의 코드가 실행되지 않는다.

따라서 `BaseHTTPConnection은` **오직 타입 체킹을 위해서 생성된 클래스라는 것을 확인할 수 있다**. 클래스는 `Protocol`**이라는 클래스를 상속 받는데 이를 통해 덕 타이핑을 활용한 타입 체킹 만을 위한 클래스를 생성할 수 있다.**

`Protocol`은 `tpying` 모듈 내부에 존재하는 객체로 덕 타이핑 타입 검사를 할때 활용한다. `Protocol`은  <span class="red red-bg">런 타임 이전에 특정 클래스가 프로토콜을 상속 받은 클래스와 대조했을 때 덕 타이핑의 성질을 만족하는지 체크할 때 활용한다. </span>

따라서 `BaseHTTPConnection`은 `Protocol`을 통해 생성된 **일종의 인터페이스로 해당 클래스를 타입으로 사용하는 객체들은 모두 지정한 메서드들을 구현하고 있어야 한다.** 이제 다시 위의 구절을 살펴보면 이해가 된다.
```python
ConnectionCls: (type[BaseHTTPConnection] | type[BaseHTTPSConnection]) = HTTPConnection
```

`ConnectionCls는`  `BaseHTTPConnection`과 비교했을 때 덕 타이핑의 성질을 만족하는 객체들을 가리킬 수 있고 디폴트로는 `HTTPConnection을` 가리킨다.
___
### HTTPConnection

다시 `_get_conneciton`으로 돌아가면 이제 커넥션 큐에서 빼오는 커넥션 객체의 정체가 `HTTPConnection`이라는 것을 확인할 수 있다. 그렇다면 `_make_request`에서 다루는 `conn` 객체가 `HTTPConnection`이라는 뜻이고 **`HTTPConnection`의 `request`를 찾으면 정말 실질적인 전송 처리부분을 파악할 수 있게 될 것이다.**

이제 `HTTPConnection`에 대해서 살펴보자. **`HTTPConnection`은 파이썬 기본 모듈인 `http.client`의 `HTTPConnection` 클래스를 상속받아 구현된다**. http.client 모듈의 경우 `urllib`에서 클라이언트 사이드의 커넥션 처리를 위해 활용하는 모듈이다. `urllib3`의 `HTTPConnection`에서는 기존 `urllib` 모듈에 약간의 처리를 더 보탰다.

실질적으로 소켓을 열고 커넥션을 맺는 작업은 전부 이 단계에서 처리된다. 아래는 `HTTPConnection`을 초기화 하는 `__init__`메서드이다.

```python
# HTTPConneciton
def __init__(
	self,
	host: str,
	port: int | None = None,
	*,
	timeout: _TYPE_TIMEOUT = _DEFAULT_TIMEOUT,
	source_address: tuple[str, int] | None = None,
	blocksize: int = 16384,
	socket_options: None
	| (connection._TYPE_SOCKET_OPTIONS) = default_socket_options,
	proxy: Url | None = None,
	proxy_config: ProxyConfig | None = None,
) -> None:
	super().__init__(
		host=host,
		port=port,
		timeout=Timeout.resolve_default_timeout(timeout),
		source_address=source_address,
		blocksize=blocksize,
	)
	self.socket_options = socket_options
	self.proxy = proxy
	self.proxy_config = proxy_config

	self._has_connected_to_proxy = False
	self._response_options = None
	self._tunnel_host: str | None = None
	self._tunnel_port: int | None = None
	self._tunnel_scheme: str | None = None
	...
	...
```

`socket_options`나 `blocksize` 같은 실질적인 소켓 생성에 필요한 속성들이 전부 요구된다. 드디어 우리가 가장 보고 싶었던 `request`로 이동하자. 인코딩 부분은 미뤄두고 메세지를 전송하는 부분을 살펴보면 `send`를 통해 전송하는 것을 확인할 수 있다.

```python
# request
...
...
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
	self.send(b"0\r\n\r\n") #data를 여기서 전송한다.
```

send는 HTTPConnection의 부모에서 구현해 사용한다. send의 내용은 아래와 같다.

```python
def send(self, data):
	"""Send `data' to the server.
	``data`` can be a string object, a bytes object, an array object, a
	file-like object that supports a .read() method, or an iterable object.
	"""

	if self.sock is None: #소켓이 없는 경우 생성
		if self.auto_open:
			self.connect()
		else:
			raise NotConnected()

	if self.debuglevel > 0:
		print("send:", repr(data))
	if hasattr(data, "read") :
		if self.debuglevel > 0:
			print("sending a readable")
		encode = self._is_textIO(data)
		if encode and self.debuglevel > 0:
			print("encoding file using iso-8859-1")
		while datablock := data.read(self.blocksize):
			if encode:
				datablock = datablock.encode("iso-8859-1")
			sys.audit("http.client.send", self, datablock)
			self.sock.sendall(datablock) # 소켓을 통해 전송한다.
		return
	sys.audit("http.client.send", self, data)
	try:
		self.sock.sendall(data)
	except TypeError:
		if isinstance(data, collections.abc.Iterable):
			for d in data:
				self.sock.sendall(d)
		else:
			raise TypeError("data should be a bytes-like object "
							"or an iterable, got %r" % type(data))

```

`send`는 커넥션 객체에 소켓이 존재하지 않을 경우 연결된 소켓을 `connect`를 통해 생성한다. 이후 생성된 소켓을 사용해 실제 메세지를 전송한다. 
___
### Connect

실질적인 커넥션은 `connect` 함수에서 진행된다. `connect` 함수 자체는 파이썬 내장 패키지인 `http.client`의 `HTTPConnection` 내부에도 존재하지만, 실제 `send` 함수에서 호출하는 함수는 `urllib`의 `HTTPConnection`에서 오버라이딩한 함수이다. 아래는 함수의 구조이다.

```python
  def  connect(self) -> None:
	self.sock = self._new_conn()
	if self._tunnel_host:
		# If we're tunneling it means we're connected to our proxy.
		self._has_connected_to_proxy = True

		# TODO: Fix tunnel so it doesn't depend on self.sock state.
		self._tunnel()  # type: ignore[attr-defined]

	# If there's a proxy to be connected to we are fully connected.
	# This is set twice (once above and here) due to forwarding proxies
	# not using tunnelling.
	self._has_connected_to_proxy = bool(self.proxy)

	if self._has_connected_to_proxy:
		self.proxy_is_verified = False

```

함수 구조를 살펴보면 실질적인 커넥션은 `_new_conn`에서 처리한다. 아래서 코드를 살펴보자.
```python
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
```

코드를 살펴보면 실질적인 연결 처리는 또다시 `create_connection`이라는 곳에서 처리한다는 것을 확인할 수 있다. 해당 함수는 `util/conneciton.py`에 존재한다.

``` python

def create_connection(
    address: tuple[str, int],
    timeout: _TYPE_TIMEOUT = _DEFAULT_TIMEOUT,
    source_address: tuple[str, int] | None = None,
    socket_options: _TYPE_SOCKET_OPTIONS | None = None,
) -> socket.socket:
    """Connect to *address* and return the socket object.

    Convenience function.  Connect to *address* (a 2-tuple ``(host,
    port)``) and return the socket object.  Passing the optional
    *timeout* parameter will set the timeout on the socket instance
    before attempting to connect.  If no *timeout* is supplied, the
    global default timeout setting returned by :func:`socket.getdefaulttimeout`
    is used.  If *source_address* is set it must be a tuple of (host, port)
    for the socket to bind as a source address before making the connection.
    An host of '' or port 0 tells the OS to use the default.
    """

    host, port = address
    if host.startswith("["):
        host = host.strip("[]")
    err = None

    # Using the value from allowed_gai_family() in the context of getaddrinfo lets
    # us select whether to work with IPv4 DNS records, IPv6 records, or both.
    # The original create_connection function always returns all records.
    family = allowed_gai_family()

    try:
        host.encode("idna") #idna는 아스키가 아닌 도메인을 아스키 형태로 변경하는 인코딩이다.
    except UnicodeError:
        raise LocationParseError(f"'{host}', label empty or too long") from None

    for res in socket.getaddrinfo(host, port, family, socket.SOCK_STREAM):
        af, socktype, proto, canonname, sa = res
        sock = None
        try:
            sock = socket.socket(af, socktype, proto)

            # If provided, set socket level options before connecting.
            _set_socket_options(sock, socket_options)

            if timeout is not _DEFAULT_TIMEOUT:
                sock.settimeout(timeout)
            if source_address:
                sock.bind(source_address)
            sock.connect(sa)
            # Break explicitly a reference cycle
            err = None
            return sock

        except OSError as _:
            err = _
            if sock is not None:
                sock.close()

    if err is not None:
        try:
            raise err
        finally:
            # Break explicitly a reference cycle
            err = None
    else:
        raise OSError("getaddrinfo returns an empty list")

```
#### *익셉션 순환 참조 문제*
**흥미로운 점은 에러 핸들링 부분인데 언더바를 활용해 에러 객체를 저장하고 핸들링 하는 모습을 확인해볼 수 있다.** 이는 예외 객체에서 발생하는 순환 참조 오류를 해결하기 위함인대, 예외 객체는 traceback을 포함하기 위해 스택 영역의 변수들을 참조하고 있다. 따라서 다음과 같은 코드가 있다고 가정하면

```python
try:
	#do something
except Exception as e:
	raise e
```

**스택 영역에 예외 객체 e가 생기고 예외 객체 e는 스택 영역에 생성된 변수 e를 참조하는 순환 참조 문제가 발생한다.** 해결하는 방법은 간단한데 예외를 발생시키고 참조를 제거해주면 된다. [참고](https://github.com/python/cpython/issues/81001)
```python
try:
	#do something
except Exception as e:
	raise e
finally:
	 e = None
```
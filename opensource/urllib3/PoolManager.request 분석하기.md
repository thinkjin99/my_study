
### PoolManager

PoolManager는 다음과 같은 주석을 갖고 있다. 

> Allows for arbitrary requests while transparently keeping track of necessary connection pools for you.

해석하면 풀 매니저를 통해 여러 종류의 HTTP 리퀘스트를 처리할 수 있고 이들을 투명하게 관리할 수 있다는 의미이다. 

<b><u>풀 매니저 클래스는 HTTP 연결에 대한 커넥션 풀을 자체적으로 처리함으로써 개발자가 리퀘스트의 송신 아래에 존재하는 요소에 대해선 신경쓰지 않게 만들어 준다.</u></b>
따라서 개발자는 커넥션 풀을 활용해 리퀘스트를 송신할 수는 있지만, 실질적인 TCP 커넥션이 어떻게 생성되는지는 파악할 방법이 없다. 내부를 파악하려면 [[HTTPConnection 부터 send까지]] 을 참고하자.

풀 매니저의 초기화 코드는 아래와 같다.

```python
  def __init__(
        self,
        num_pools: int = 10, #풀에서 사용할 커넥션의 수 (해당 수보다 커넥션이 많을 경우 가장 LRU 방식으로 커넥션을 닫는다)
        headers: typing.Mapping[str, str] | None = None, #커넥션 풀에서 나가는 모든 리퀘스트에 적용되는 헤더
        **connection_pool_kw: typing.Any,
    ) -> None:
        super().__init__(headers)
        self.connection_pool_kw = connection_pool_kw

        self.pools: RecentlyUsedContainer[PoolKey, HTTPConnectionPool] #타입힌트가 길어서 두줄에 나눠서 선언
        self.pools = RecentlyUsedContainer(num_pools)

        # Locally set the pool classes and keys so other PoolManagers can
        # override them.
        self.pool_classes_by_scheme = pool_classes_by_scheme
        self.key_fn_by_scheme = key_fn_by_scheme.copy()
```

풀 매니저는 말 그대로 커넥션 풀을 매니징하는 객체로 커넥션 자체는 `HTTPConnectionPool` 객체를 통해 관리 된다. 이제 우리가 앞서 궁금했던 `request` 메서드를 살펴보자.
___
### RequestMethods

우선적으로 `request` 는 풀 매니저 클래스가 아닌 부모 클래스인 `RequestMethods` 아래에 위치한다. 해당 클래스는 `urlopen()` 메서드를 쉽게 구현하기 위해 사용하는 클래스로 추상 메서드 `urlopen`을 갖고 있는 추상 클래스이다.

 `request` 메서드는  `urlopen`을 활용해 리퀘스트를 송신하고  `BaseHTTPResponse` 객체를 반환한다.
 결국 **`request` 메서드는 `urlopen` 메서드의 wrapper이고 `urlopen` 함수 자체는 `PoolManager와` 같은 객체에서 직접적으로 구현하기 때문에 실질적인 커넥션 활용 및 처리 부분을 살펴보기 위해선 `urlopen` 메서드를 살펴봐야 한다.**

request는 아래와 같이 적절한 인코딩 처리 후 urlopen을 호출하는 작업만 수행한다.
```python
 if headers is None:
	headers = self.headers

	extra_kw: dict[str, typing.Any] = {"headers": headers}
	extra_kw.update(urlopen_kw)

	if fields:
		url += "?" + urlencode(fields)

	return self.urlopen(method, url, **extra_kw) #return response...
```
____
### PoolManager.urlopen()

이제 다시 풀 매니저 객체의 urlopen을 살펴보자. 문서를 살펴보면 [`urllib3.HTTPConnectionPool.urlopen()`](https://urllib3.readthedocs.io/en/stable/reference/urllib3.connectionpool.html#urllib3.HTTPConnectionPool.urlopen "urllib3.HTTPConnectionPool.urlopen")와 동일한 메서드라는 설명이 적혀있다. 또한 코드 내부를 살펴보면 실제로 `HTTPConnectionPool` 객체를 통해 `urlopen을` 전부 처리하는 모습을 확인할 수 있다.
따라서 urlopen의 실질적인 정의를 확인하기 위해선 [[HTTPConnection 부터 send까지#HTTPConnectionPool|HTTPConnectionPool]] 객체를 우선적으로 살펴봐야 한다.

```python
	conn = self.connection_from_host(u.host, port=u.port, scheme=u.scheme)
	kw["assert_same_host"] = False
	kw["redirect"] = False
	
	if "headers" not in kw:
		kw["headers"] = self.headers
	
	if self._proxy_requires_url_absolute_form(u):
		response = conn.urlopen(method, url, **kw) #응답을 얻어오는 과정은 여기서 처리
	else:
		response = conn.urlopen(method, u.request_uri, **kw) #응답을 얻어오는 과정은 여기서 처리
	
	redirect_location = redirect and response.get_redirect_location() #이후 리다이렉트 등을 처리
	if not redirect_location:
		return response
```

___

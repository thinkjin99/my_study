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


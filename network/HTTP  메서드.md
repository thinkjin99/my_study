### 출처

* [RFC 문서](https://httpwg.org/specs/rfc9110.html#idempotent.methods)
* [POST VS PUT](https://stackoverflow.com/questions/630453/what-is-the-difference-between-post-and-put-in-http)
* [MDN](https://developer.mozilla.org/ko/docs/Web/HTTP/Methods/PUT)
* [Delete](https://humblego.tistory.com/18)
___
### 개요
* [[#HTTP 메서드]]
* [[#멱등성과 안전함]]
* [[#GET]]
* [[#HEAD]]
* [[#POST]]
* [[#PUT VS POST]]
* [[#DELETE]]
___
### HTTP 메서드

HTTP 메서드는 요청 메시지의 첫째줄에 기록되며 클라이언트가 서버에게 요청하는 행동에 대한 정보가 저장돼 있다.
___
### 멱등성과 안전함

* **멱등성**
	==**동일한 요청을 반복해서 보내는 것과 한번 보내는 것이 같은 효과를 지니는 경우를 말한다.**==
	멱등성은 명확히 구분하는 것이 좋은데 이는 네트워크 상에서는 자동적으로 retry를 실시하는 경우가 존재하기 때문이다. 안전성과 구분해서 생각할 필요가 있다. **멱등성은 동일한 결과가 반복되기만 하면 된다. 어떠한 변화도 없는 것은 안전성이다.**

* **안전성**
	안젼성은 특정 메서드를 서버가 처리할때 서버 리소스에 일어나는 일이 로깅을 제외하고 없다는 것을 의미한다. 즉, **읽기 작업만 수행하고 서버의 리소스에 어떤 변경 사항이 없는 메서드의 경우 안전 메서드**라 칭한다. ==**모든 안전 메서드는 멱등성을 갖지만, 멱등성을 갖는 모든 메서드가 안전한 것은 아니다.**==
___
### GET

==**GET은 가장 흔히 쓰이는 메서드로 주로 서버에게 리소스를 요청하기 위해 사용한다.**== 서버는 GET 요청을 수신하면 전달 받은 URI를 해석하고 해당 URI가 지칭하는 자원을 클라이언트에게 전달한다.

**GET과 URI를 통해 다양한 애플리케이션에서 자원을 손쉽게 공유하는 작업이 가능**해졌고 이를 통해 네트워크 상에서 자원을 재사용하거나 효율적으로 관리하는 것이 가능해졌다. (URI만 존재한다면 누구나 동일한 자원에 접근이 가능하다.)

<b><u>GET 요청에 대한 결과는 캐싱 가능하다.</u></b> 또한 range-request와 같이 정보의 일부분만 요청하는 헤더도 존재해 효율을 더욱 증대 시킬 수 있다.

|           |     |
| --------- | --- |
| 요청에 본문 존재 | 아니오 |
| 응답에 본문 존재 | 예   |
| 안전함       | 예   |
| 멱등성       | 보장  |
| 캐시 가능 여부  | 가능  |
|           |     |

___
### HEAD

**HEAD는 GET과 거의 동일 하지만 서버는 응답으로 본문을 제외한 헤더만 돌려줘야 한다.** 따라서 GET에 비해 빠른 응답 처리가 가능하고 리소스의 타입이나 간략한 정보들을 파악할 수 있다. **본문이 없는 GET으로 이해하면 직관적이다.** HEAD 메서드는 주로 다음과 같은 경우에서 자주 사용한다. 

* **본문 엔티티의 변경 발생 여부 파악을 위해**
* **본문의 타입 파악을 위해**
* **본문 컨텐츠의 존재 여부 파악을 위해**

AWS에서 특정 버킷이 존재하는지 확인할 때 사용하는 head_bucket 메서드에서 HEAD를 활용해 버킷의 존재 유무를 파악한다.

```python
 def head_bucket(self, bucket_name, headers=None):
	"""
	Determines if a bucket exists by name.

	If the bucket does not exist, an ``S3ResponseError`` will be raised.

	:type bucket_name: string
	:param bucket_name: The name of the bucket

	:type headers: dict
	:param headers: Additional headers to pass along with the request to
		AWS.

	:returns: A <Bucket> object
	"""
	response = self.make_request('HEAD', bucket_name, headers=headers)
	body = response.read()
	if response.status == 200:
		return self.bucket_class(self, bucket_name)
	elif response.status == 403:
		# For backward-compatibility, we'll populate part of the exception
		# with the most-common default.
		err = self.provider.storage_response_error(
			response.status,
			response.reason,
			body
        )
```
____
### POST

==**서버에 입력 데이터를 전송하기 위해 설계 됐으며 HTML 폼 데이터 및 각종 MIME 데이터를 전송하기 위해 사용된다.**== 데이터는 본문 영역에 담기며 전부 인코딩 돼 전송된다. <b><u>POST는 서버에 변경사항을 만드는 요청으로 멱등성을 보장하지 않는다. </u></b>

**POST는 주로 어떠한 리소스를 생성하는 경우 사용**한다. 혹은 URL이나 헤더 만으로 전달하기에 너무 긴 데이터일 경우에도 활용한다. 추가적으로 로그인 등의 작업을 하는 경우에도 POST를 활용하는데 이는 개인정보를 URL에서 노출하는 일을 방지하기 위함이다.

| 요청에 본문 존재 | 예         |     |
| --------- | --------- | --- |
| 응답에 본문 존재 | 예         |     |
| 안전함       | 아니오       |     |
| 멱등성       | 아니오       |     |
| 캐시 가능     | 조건에 따라 다름 |     |
___
### PUT VS POST

PUT은 POST와 흡사하지만 멱등성을 갖는다. **PUT은 리소스가 존재하면 해당 리소스를 수정하고 리소스가 존재하지 않을 경우 리소스를 생성하는 기능을 수행**한다. ==**POST와 PUT은 용도가 흡사해 헷갈리는 구석이 존재하지만 PUT은 멱등성을 갖는 것에 비해 POST는 멱등성을 갖지 않는다는 특징이 존재한다.

PUT은 요청한 엔티티가 없을 경우 생성하고 이미 존재할 경우 덮어 씌운다. 몇번을 반복해서 요청을 송신해도 동일한 결과가 반환될 것이다. POST도 그렇게 동작할 수 있지만 POST는 그러면 안된다 **POST는 요청을 송신할 때마다 서버의 상태가 변화해야한다.**

예제로 생각해보자 우리는 게시글 관련 API를 작성해야 한다. 해당 API는 게시글을 작성할 수 있고 게시글을 수정할 수 있어야 한다. 이 경우 아래와 같이 API를 설계할 수 있을 것이다.

* **방법 1**
	* POST: https://my_service/article (게시글 ID는 서버가 부여한다)
	* PUT: https://my_service/article/article_id

* **방법 2**
	* PUT: https://my_service/aticle/optional (id가 있으면 수정 없으면 생성한다)

정답이 정해져 있는 문제는 아니다. 각각 장단이 존재한다. 하지만 오답은 존재한다 방법 2의 API를 POST를 활용해 구현한다면 **POST는 항상 변화를 발생시켜야 한다는 성질을 만족하지 않으므로 RESTful하지 않게 된다.**

스택 오버 플로우 친구들은 아래의 기준으로 POST,PUT을 구분해 사용할 것을 권유한다.
* 생성되는 컨텐츠의 URL을 사용자가 지정할 수 없다 -> POST
* 동일한 URL로 수정, 생성이 처리 가능하다 -> PUT
* URL로 명확하게 id까지 지정가능하다 -> PUT

>[!info]
>**가장 주요한 것은 멱등성을 만족하는지 여부에 따라 PUT과 POST를 구분해서 사용해야 한다는 것이다.**

___
### DELETE

DELETE는 요청 받은 URI에 존재하는 리소스를 삭제하는 기능을 수행한다. ==**DELETE 메서드는 멱등성을 보장한다.**== 클라이언트는 삭제를 보장할 수는 없는데 이는 서버가 삭제 명령을 무시할 수도 있기 때문이다.

흥미로운 점은 DELETE의 경우 요청 본문을 비우는 것을 권장한다는 것이다. 삭제는 안정성을 보장하지 않는 메서드이기에 권한 검사가 필수적인 작업인데 이 경우 인증 정보를 넘길 곳이 마땅치 않아진다. (헤더에 담아 전달하는 것이 베스트로 보인다..)

DELETE의 경우 곧장 삭제를 진행할 수 없는 경우 202(삭제 예정) 등의 응답 코드를 반환 하기도 한다.

>[!info]
>POST로 삭제를 구현하는 경우도 있는데, 별로 좋은 생각은 아니다. 삭제는 보통 멱등성을 보장하는 작업이기 때문이다.

|         |         |     |
| ------- | ------- | --- |
| 본문 존재   | 없는 걸 권장 |     |
| 안전성     | 없음      |     |
| 멱등성     | 예       |     |
| 캐시가능 여부 | 아니오     |     |
___
#### Basic auth와 HTTPS
### 출처

* [[#HTTP는 상태가 없다]](https://stackoverflow.com/questions/19899236/is-tcp-protocol-stateless)
* [[#Stateful]](https://www.freecodecamp.org/news/stateful-vs-stateless-architectures-explained/)
* [코딩애플 뚱뚱한 URL](https://www.youtube.com/watch?v=pCOBmmJARPE)
____
### 개요

____
### Stateful

**Stateful은 각 통신주체가 서로의 이전 통신 결과를 기억하고 있음을 말한다.** 상태를 갖는 프로토콜은 대표적으로 TCP가 존재하는데 TCP는 이전 통신의 결과를 활용해 윈도우 사이즈를 조절하거나 전송 받은 패킷에 대한 응답을 보내며 서로의 상태를 확인한다. 

예시를 들자면 레스토랑의 웨이터를 생각해볼 수 있다. 웨이터에게 주문을 하면 해당 웨이터는 주문을 노트 패드에 기록한다. 이후 메뉴를 변경하고자 한다면 해당 웨이터에게 부탁을 하면 된다. 요청사항은 전담 웨이터를 통해야만 하며 이는 전담 웨이터만이 히스토리를 공유하고 있기 때문이다.

이를 실제 서버-클라이언트 환경으로 변경해 다이어그램으로 표현하면 아래와 같다. **유저 1의 전담은 서버 1이다. 유저2의 전담은 서버 2이다. 로드밸런서가 중간에 리다이렉팅을 수행하며 세션이 절대 끊어지지 않게 끈끈한 관계를 생성 해준다.**

![https://www.freecodecamp.org/news/stateful-vs-stateless-architectures-explained/](https://my-study.s3.ap-northeast-2.amazonaws.com/HTTP%EC%9D%98%20%EC%BF%A0%ED%82%A4%EC%99%80%20%EC%84%B8%EC%85%98%20%2B%20JWT%20/%20Pasted%20image%2020240402114422.png)

이러한 시스템 구조를 사용하면 **서로가 히스토리를 공유하기 때문에 요청사항 만을 간략하게 전달돼 통신 오버헤드가 줄어든다는 장점이 있다.** 아래와 같은 상황을 가정해보자. 나는 레스토랑에서 와인재고 요청을 전담 웨이터에게 전달했다.

* **전담 웨이터와 말하는 경우**
	`웨이터`: 이전에 찾으셨던 와인이 2병 있습니다. 몇 병 주문하시겠습니까?
	
	`나`: 2병
	
	`웨이터`: 확인했습니다.

* **새로운 웨이터와 말하는 경우**
	`웨이터`: 필요한게 있으실까요?
	
	`나`: 아까 레드와인 재고 확인 요청을 드렸는데요
	
	`웨이터`: 확인해보겠습니다.
	
	(찾는 중)
	
	`웨이터`: 이전에 찾으셨던 와인이 2병 있습니다. 몇 병 주문하시겠습니까?
	
	`나`: 2병
	
	`웨이터`: 확인했습니다.

<span class="red red-bg">만약 상태를 가지지 않는 서버와 통신한다면 상대는 이전의 히스토리를 기억하지 못하기 때문에 관련된 정보를 다시금 전송해줘야 하는 오버헤드가 발생한다.</span>

**Stateful이 이상적으로 진행된다면 Stateless한 방식보다 통신 오버헤드가 적으므로 더 빠른 처리가 가능할 것이다. 하지만 네트워크 환경에서 이상적인 진행이라는 것은 불가하다.**
____
### Stateful의 함정

상태를 갖는다는 것은 누군가와 히스토리를 공유하고 있음을 말한다. **서버 - 클라이언트 구조로 말하자면 서버의 메모리 어딘가에 클라이언트와의 통신 데이터가 저장되고 있음을 의미**한다. 이 구조에서 몇가지 문제가 발생한다.

#### *스케일링의 어려움*
다시 레스토랑으로 돌아가보자. 이제 레스토랑에 손님이 많아져 전담 웨이터를 부르면 요청을 처리하기까지 너무 오랜 시간이 걸린다. <b><u>문제는 다른 웨이터를 부르고 싶더라도 한번 배정된 웨이터를 바꾸는 것은 오버헤드가 큰 작업이라는 것이다. 이에 따라 웨이터를 신규 고용 하더라도 해당 웨이터가 내 히스토리를 전부 전수받고 나의 주문을 처리하기 까진 오랜 시간이 소요된다.</u></b>

![500](https://my-study.s3.ap-northeast-2.amazonaws.com/HTTP%EC%9D%98%20%EC%BF%A0%ED%82%A4%EC%99%80%20%EC%84%B8%EC%85%98%20%2B%20JWT%20/%20Pasted%20image%2020240402121129.png)

심지어 만약 로드밸런서가 존재하지 않는다고 가정하면 서버가 증가 됐을 때 모든 서버에 요청을 보내보며 내 히스토리를 갖고 있는 서버를 찾아내야 한다.

#### *장애 회복 문제*
이번엔 웨이터가 갑자기 빈혈로 인해 쓰러진 상황을 생각해보자. 웨이터가 자리를 비우면 이제 해당 웨이터가 담당하던 클라이언트의 주문 정보를 파악하고 있는 웨이터가 존재하지 않게된다. 추후 이에 따른 대혼란이 발생하게 된다. 

**서버 역시 마찬가지로 담당하던 서버에 장애가 발생할 경우 해당 서버에서 처리하던 커넥션의 히스토리를 전부 파악하지 못하게 되는 문제가 발생한다.** 이는 재앙 같은 일이며 만약 서버를 재부팅하게 돼 메모리에 적재됐던 커넥션 데이터가 전부 휘발 됐다면... 그날 퇴근은 물건너갔다고 해도 과언이 아니다. (오히려 야근으로 해결되면 다행인 문제이다.)

#### *불균형한 업무 분배 (로드 밸런싱 문제)*
웨이터 선의는 10명의 손님을 담당하고 영민은 2명만 담당하는 상황을 가정해보자. 선의는 10명의 손님을 반드시 전담해야하기 때문에 발생하는 요청을 영민이에게 전가할 수 없다. 이에 따라 영민이는 여유롭게 웹툰 볼 틈도 있는 반면에 선의는 숨돌릴 틈도 없는 업무 불균형 현상이 발생한다.

<b><u>서버에서도 마찬가지로 특정 서버에서만 전담하는 커넥션의 수가 많을 경우 해당 서버만 히스토리를 보유하고 있으므로 다른 서버는 도와주지 못하고 놀게된다. 이에 따라 리퀘스트를 특정 서버에서만 집중적으로 처리하는 불균형 현상이 발생할 수 있다.</u></b>

>[!info]
>**상태를 가질 경우 요청의 처리가 신속해 질수도 있지만, 문제가 발생할 경우 재앙과 같은 일이 발생한다.**
____

____
### Stateless

Stateless는 Stateful의 반대 개념으로 **서버가 이전 요청으로 인해 발생한 데이터들을 서버 내부에 기록하지 않고 각 요청을 독립된 요청으로 처리하는 성질을 말한다**.

상태가 없는 경우 이전 요청에서 발생한 데이터들을 별도의 데이터베이스나 쿠키 등으로 설정해 서버가 아닌 외부에서 관리한다.

다시 식당으로 가보자. **이전에는 웨이터 각자가 노트에 고객의 정보를 기록했지만 이제부터 포스기를 도입해 고객 정보를 각 웨이터의 노트가 아닌 가게 중앙에 위치한 포스기로 기록할 것이다.**

Stateless를 실천할 경우 앞서 발생했던 3가지 문제를 전부 해결할 수 있다.

* **스케일링의 어려움**
	이전에는 노트를 웨이터 각각이 소유하고 있기 때문에 신규 웨이터 확장에 어려움이 존재했다. 하지만 이젠 **포스기를 통해 모든 웨이터가 주문 정보를 접근할 수 있으므로 웨이터의 수를 늘리는 것이 이전처럼 부담스럽지 않아진다.**

* **장애회복 문제**
	이전에는 노트를 담당하던 웨이터가 자리를 비우면 담당 정보도 접근이 불가해져 곤란한 상황이 발생했지만, 이젠 포스기에 모든 정보가 저장돼 있으므로 곧장 다른 웨이터가 해당 손님을 담당할 수 있다.

* **불균형한 업무 분배**
	이전에는 담당하는 웨이터만 요청을 파악할 수 있어 특정 웨이터에만 일감이 쏠리는 현상이 발생했지만, 이제 **포스기를 통해 모두가 요청을 처리할 수 있으므로 균등하게 업무를 분담하는 것이 가능해졌다.**

![https://www.freecodecamp.org/news/stateful-vs-stateless-architectures-explained/](https://my-study.s3.ap-northeast-2.amazonaws.com/HTTP%EC%9D%98%20%EC%BF%A0%ED%82%A4%EC%99%80%20%EC%84%B8%EC%85%98%20%2B%20JWT%20/%20Pasted%20image%2020240402144059.png)

서버-클라이언트 구조의 다이어그램으로 표현하면 위와 같은 모습이며, **커넥션의 상태를 외부에 저장하기 위한 캐시가 추가된 것을 확인할 수 있다.**

> [!info]
> **Stateless는 Stateful의 큰 문제들을 전부 해결하지만, 통신 속도 자체는 느릴 수 있다.**

___
### HTTP는 상태가 없다

<span class="red red-bg">HTTP가 상태가 없다는 말과 웹 어플리케이션이 상태가 없다는 말은 다른 의미이다. 웹 어플리케이션은 HTTP를 활용해 구현되는 것으로 어플리케이션 자체는 상태를 가질 수도 가지지 않을 수도 있다. </span>

HTTP는 규약 자체가 상태를 가지지 않는 통신으로 지정돼 있다. **HTTP는 상대방의 상태를 고려하지 않고 각각의 요청을 개별적으로 취급한다.** TCP 처럼 ACK를 보내지도 않고 윈도우 사이즈 처럼 서로 주고 받는 속성이 존재하지도 않는다. 따라서 HTTP 자체는 메시지의 형태와 의미만 정의할 뿐 상태를 가지지는 않는 프로토콜이다.

위키백과의 정의를 참조해보면 아래와 같다.
`HTTP 프로토콜은 요청 간 사용자 데이터를 저장하는 수단을 제공하지 않는다.` 쿠키나 세션 등을 활용하면 상태를 저장할 수 있지 않냐는 생각을 할 수도 있지만, **쿠키나 세션은 HTTP에서 반드시 사용하는 것이 아닌 웹 어플리케이션에서 구현해 사용하는 것**이다. 따라서 쿠키나 세션을 통한 상태의 저장은 HTTP 자체에 정의된 것이 아니라 웹 어플리케이션에서 활용하는 요소라고 생각해야 한다.

>[!info]
>**HTTP 자체는 Stateless이다 메시지를 쏘고 나면 그 뒤로 신경쓰지 않는다. 상태를 신경쓰지 않기 때문에 각각의 요청은 독립적으로 처리된다.**

___
### 웹 어플리케이션의 상태 관리

#### *헤더로 관리하기*
상태를 관리하며 사용자를 식별하는 첫번째 방법은 헤더를 사용하는 것이다. 헤더를 활용해 유저의 정보를 곧장 서버로 전송한다. 이 방법은 쉽고 직관적이지만, **헤더 정보는 누구나 확인할 수 있기 때문에 보안에 취약하다는 문제점이 존재한다.**

![400](https://my-study.s3.ap-northeast-2.amazonaws.com/HTTP%EC%9D%98%20%EC%BF%A0%ED%82%A4%EC%99%80%20%EC%84%B8%EC%85%98%20%2B%20JWT%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-04-02%20%EC%98%A4%ED%9B%84%205.51.45.png)
주요한 헤더로 User-Agent와 Refer 등의 헤더가 존재하는데 **User-Agent는 유저가 접속한 기기 정보와 브라우저 정보를 확인**할 수 있고 Refer 같은 경우 사용자가 이전에 머물렀던 페이지를 기록해 마케팅 회사 등에서 용이하게 사용한다.

User-Agent를 토대로 요청을 거절하는 웹서버도 왕왕 존재한다. 브라우저가 아닌 CURL이나 코드를 통해 작성된 Agent를 주로 거절한다.
#### *IP나 소켓으로 식별하기*
IP나 소켓을 활용해 식별하는 방법도 존재한다. 하지만 이 경우 유저의 IP가 변경되거나 연결을 끊고 새로운 포트로 커넥션을 맺을 경우 유저의 정보를 파악할 수 없다는 문제가 발생한다.

#### *로그인 인증 활용하기*
로그인등의 인증 절차를 활용해 토큰을 발급받고 매 요청마다 이를 같이 전송하는 방식도 존재한다. 이 방식은 추후에 더욱 자세히 다뤄 보도록 하자.

#### *URL에 다 넣기*
관련 정보를 전부 URL에 저장하는 방법도 있다. 유저가 발생시킨 정보를 전부 URL에 포함시켜 서버에서 URL을 토대로 데이터를 파싱하는 것이다. 이 방법은 단순하지만 **URL을 활용한다는 점에서 보안에 취약점이 존재하고 URL이 못생겨진다는 단점이 발생한다. 또한 URL이 계속해서 변경 되기에 캐싱 작업이 어렵다는 문제도 존재**한다.

* **이거 준내 위험한 방법 아닌가요?**
	흥미로운 내용이지만 실제로 이걸 운영 레벨에서 사용하는 서비스사 존재한다. [코딩애플](https://www.youtube.com/watch?v=pCOBmmJARPE)
	에서 소개한 내용을 확인하면 야후 파이낸스에서는 위 방법으로 유저의 환경설정 데이터를 저장한다. 실제 서비스를 확인해보자. [야후 파이낸스](https://finance.yahoo.com/chart/OXY?showOptin=1#eyJpbnRlcnZhbCI6MSwicGVyaW9kaWNpdHkiOjEsInRpbWVVbml0IjoibWludXRlIiwiY2FuZGxlV2lkdGgiOjcuMzg4NTM1MDMxODQ3MTM0LCJmbGlwcGVkIjpmYWxzZSwidm9sdW1lVW5kZXJsYXkiOnRydWUsImFkaiI6dHJ1ZSwiY3Jvc3NoYWlyIjp0cnVlLCJjaGFydFR5cGUiOiJsaW5lIiwiZXh0ZW5kZWQiOmZhbHNlLCJtYXJrZXRTZXNzaW9ucyI6e30sImFnZ3JlZ2F0aW9uVHlwZSI6Im9obGMiLCJjaGFydFNjYWxlIjoibGluZWFyIiwicGFuZWxzIjp7ImNoYXJ0Ijp7InBlcmNlbnQiOjEsImRpc3BsYXkiOiJPWFkiLCJjaGFydE5hbWUiOiJjaGFydCIsImluZGV4IjowLCJ5QXhpcyI6eyJuYW1lIjoiY2hhcnQiLCJwb3NpdGlvbiI6bnVsbH0sInlheGlzTEhTIjpbXSwieWF4aXNSSFMiOlsiY2hhcnQiLCLigIx2b2wgdW5kcuKAjCJdfX0sInNldFNwYW4iOm51bGwsImxpbmVXaWR0aCI6Miwic3RyaXBlZEJhY2tncm91bmQiOnRydWUsImV2ZW50cyI6dHJ1ZSwiY29sb3IiOiIjMDA4MWYyIiwic3RyaXBlZEJhY2tncm91ZCI6dHJ1ZSwicmFuZ2UiOm51bGwsImV2ZW50TWFwIjp7ImNvcnBvcmF0ZSI6eyJkaXZzIjp0cnVlLCJzcGxpdHMiOnRydWV9LCJzaWdEZXYiOnt9fSwic3ltYm9scyI6W3sic3ltYm9sIjoiT1hZIiwic3ltYm9sT2JqZWN0Ijp7InN5bWJvbCI6Ik9YWSIsInF1b3RlVHlwZSI6IkVRVUlUWSIsImV4Y2hhbmdlVGltZVpvbmUiOiJBbWVyaWNhL05ld19Zb3JrIn0sInBlcmlvZGljaXR5IjoxLCJpbnRlcnZhbCI6MSwidGltZVVuaXQiOiJtaW51dGUiLCJzZXRTcGFuIjpudWxsfV0sImN1c3RvbVJhbmdlIjpudWxsLCJzdHVkaWVzIjp7IuKAjHZvbCB1bmRy4oCMIjp7InR5cGUiOiJ2b2wgdW5kciIsImlucHV0cyI6eyJpZCI6IuKAjHZvbCB1bmRy4oCMIiwiZGlzcGxheSI6IuKAjHZvbCB1bmRy4oCMIn0sIm91dHB1dHMiOnsiVXAgVm9sdW1lIjoiIzAwYjA2MSIsIkRvd24gVm9sdW1lIjoiI2ZmMzMzYSJ9LCJwYW5lbCI6ImNoYXJ0IiwicGFyYW1ldGVycyI6eyJ3aWR0aEZhY3RvciI6MC40NSwiY2hhcnROYW1lIjoiY2hhcnQiLCJwYW5lbE5hbWUiOiJjaGFydCJ9fSwi4oCMbWHigIwgKDUwLEMsbWEsMCkiOnsidHlwZSI6Im1hIiwiaW5wdXRzIjp7IlBlcmlvZCI6NTAsIkZpZWxkIjoiQ2xvc2UiLCJUeXBlIjoic2ltcGxlIiwiT2Zmc2V0IjowLCJpZCI6IuKAjG1h4oCMICg1MCxDLG1hLDApIiwiZGlzcGxheSI6IuKAjG1h4oCMICg1MCxDLG1hLDApIn0sIm91dHB1dHMiOnsiTUEiOiIjYWQ2ZWZmIn0sInBhbmVsIjoiY2hhcnQiLCJwYXJhbWV0ZXJzIjp7ImNoYXJ0TmFtZSI6ImNoYXJ0IiwicGFuZWxOYW1lIjoiY2hhcnQifX0sIuKAjG1h4oCMICgyMCxDLG1hLDApIjp7InR5cGUiOiJtYSIsImlucHV0cyI6eyJQZXJpb2QiOiIyMCIsIkZpZWxkIjoiQ2xvc2UiLCJUeXBlIjoic2ltcGxlIiwiT2Zmc2V0IjowLCJpZCI6IuKAjG1h4oCMICgyMCxDLG1hLDApIiwiZGlzcGxheSI6IuKAjG1h4oCMICgyMCxDLG1hLDApIn0sIm91dHB1dHMiOnsiTUEiOiIjZmY0YWFkIn0sInBhbmVsIjoiY2hhcnQiLCJwYXJhbWV0ZXJzIjp7ImNoYXJ0TmFtZSI6ImNoYXJ0IiwicGFuZWxOYW1lIjoiY2hhcnQifX19fQ--)
	**공개해도 되는 데이터, DB에 저장할 필요가 없는 데이터, 공유가 쉬워야하는 데이터의 경우** 이런 방법이 장점을 가질 수 있다.

___
### 쿠키와 세션

쿠키는 사용자를 식별하고 정보를 유지하는 방식 중에서 현재까지 가장 널리 사용 되는 방식이다.

쿠키란?
쿠키 써먹기
쿠키의 장단

세션이란?
세션 써먹기
세션의 장단

쿠키 VS 세션
___
### 왜 이렇게 쿠키를 허가 해달라고 할까?
쿠키 허용시 무슨 일이 발생할까?
쿠키의 악용성
___
### 어떻게 상태 관리를 해야 안전?

쿠키로 상태 관리를 하면 안전할까?
프론트엔드 단에서의 상태 관리
___
### JWT

jwt는 무엇?
jwt는 왜 필요?
jwt의 장단
jwt를 적절히 관리하는 법 (벡엔드, 프론트엔드)


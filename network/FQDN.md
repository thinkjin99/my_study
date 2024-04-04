
#### FQDN이란
 `www.example.com.`이러한 도메인 Fully Qqualified Domain Name이라고 하는데 직역 하자면 완벽한 도메인이다. 우리가 흔히 인터넷을 사용할때 브라우저에 입력하는 url과 흡사하다. FQDN은 다음과 같이 구성된다.

![https://www.hostinger.in/tutorials/fqdn](https://my-study.s3.ap-northeast-2.amazonaws.com/FQDN%20/%20Pasted%20image%2020240404215524.png)

**FQDN은 한가지 특징이 존재하는데 바로 도메인의 끝이 `.` 으로 끝난다는 점이다.** 통상 도메인은 계층 구조로 구성되고 가장 오른쪽에 위치한 도메인이 탑 레벨 도메인이 된다 (주로 .com, .net). 흥미로운 점은 일반적으로 **도메인의 마지막에 위치하는 `.com`이 도메인의 끝을 의미해야한다는 규칙은 없다는 것이다**. 따라서 이론상 `example.com.something.com` 도 가능하다. 실제로 `www.naver.com.nheos.com` 이 존재한다.

**FQDN의 또 다른 특징은 검색 도메인을 활용하지 않는다는 점이다.** 검색 도메인은 네트워크 기기나 DNS등 에서 설정할 수 있는데, 리졸빙을 하기전에 우선적으로 검색해보는 도메인들을 말한다. 예를 들어 mac에서 다음과 같이 검색 도메인을 추가했다고 하자. (설정에서 추가할 수 있다)

![400](https://my-study.s3.ap-northeast-2.amazonaws.com/FQDN%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-04-04%20%EC%98%A4%ED%9B%84%209.44.48.png)

이 경우 `www`에 `nslookup`을 시도할 경우 다음과 같은 결과가 도출된다.

![400](https://my-study.s3.ap-northeast-2.amazonaws.com/FQDN%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-04-04%20%EC%98%A4%ED%9B%84%209.45.24.png)

이는 `www`라는 도메인을 탐색하기 전에 우선적으로 검색 도메인인 `naver.com`을 붙여 탐색을 진행했기 때문에 발생하는 현상이다. 이는 모든 DNS 활용에 적용되는 요소인데, 내가 `example.com`을 등록하고 `bar.com`에 접속한다고 하면 `bar.com.example.com`을 우선적으로 DNS에 쿼리하고 이후 `bar.com`에 접속한다. **이를 사용하면 불편하게 `example.com`을 매 접속마다 입력하지 않아도 된다는 장점이 있지만, 잘못하면 DNS 오버헤드가 발생할 수 있다.**

이때 FQDN을 활용하면 이러한 현상을 방지할 수 있다. FQDN을 사용할 경우 검색 도메인을 전부 무시하고 해당 도메인에 대한 쿼리를 곧장 날린다. **해당 도메인이 그 자체만으로 완전한 도메인이기 때문에 별도의 처리를 진행하지 않는 것이다.** (`www.`에 nslookup을 해보자)

이에 따라 본래 **DNS에 레코드를 등록하기 위해서는 완전한 도메인인 것을 나타낼 수 있는 FQDN의 형태로 등록해야 했지만** 근래에 들어서는 .com이나 .net으로 끝나는 경우 FQDN이라 감지하고 검색 도메인의 사용을 진행하지 않는다.

FQDN의 형태로 DNS에 등록한 회사는 대다수 인것 같다. 우리가 DNS를 사용할때 이를 느끼지 못한 것은 아마 DNS 제공 업체에서 이를 자동적으로 처리 해주기 때문일 수도 있다. (깃헙, 네이버, 유튜브 전부 FQDN을 사용한다)

![400](https://my-study.s3.ap-northeast-2.amazonaws.com/FQDN%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-04-04%20%EC%98%A4%ED%9B%84%209.53.44.png)
___


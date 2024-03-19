### urllib3 분석을 들어가며

파이썬에서 HTTP 요청을 쉽게 처리하게 해주는 urrlib3 라이브러리의 코드를 분석하고자 한다. 분석을 결심한 까닭은 다음과 같다.

* requests에서도 사용하는 근본적인 TCP 연결 처리 방식이 궁금해서
	* requests는 urllib를 사용하기 쉽게 감싸 놓은 라이브러이다.
* 효율적인 커넥션 관리를 어떻게 구현하는지 파악하고 싶어서 (커넥션 풀 등..)
* SSH 등을 소켓 레벨에서 지원하려면 어떻게 해야하는지 확인하고 싶어서
* 세션, 쿠키 등의 데이터를 어떻게 관리하는지 배우고 싶어서
___
### 진행방식

오픈소스 분석의 절차는 직접 사용하는 함수 부터 타고 들어가 해당 함수의 구현 부까지 파악하는 것을 목표로 한다. 

urllib3의 경우 일반적으로 사용하는 함수가 `urllib.Poolmanager().request()` 이므로 해당 함수의 구현 부분과 어떻게 구성됐는지를 파악하는 것을 목표로 한다.

___
### 구조도

![](https://my-study.s3.ap-northeast-2.amazonaws.com/Readme.md%20/%20REALTION.png)
___
### 흐름도

![](https://my-study.s3.ap-northeast-2.amazonaws.com/Readme.md%20/%20FLOW.png)

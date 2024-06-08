
### 출처
* [SDN이란](https://www.comparitech.com/net-admin/software-defined-networking/)
* [SDN의 장점](https://movefast.tistory.com/54)
___
### 개요
* [[#SDN?]]
* [[#SDN의 구조]]
* [[#Open Flow]]
___
### SDN?

**SDN은 네트워크 관리와 제어를 소프트웨어로 구현하고 중앙 집중식으로 관리하는 네트워크 아키텍쳐를 말한다.** 

<span class="red red-bg">SDN의 핵심은 가상화이다. SDN을 활용하면 직접 해당 장치에 접근해 설정을 수정할 필요 없이 관리 장치에서 모든 네트워크 설정을 수정할 수 있다.</span> 

**기존에는 하드웨어 기반의 라우팅을 주로 활용 했지만 이는 동적 대응이 어렵고 중앙에서 한번에 관리가 어렵다는 한계점이 존재**했다. 현대에 이르러 네트워크가 점점 커지고 발전하면서 이러한 문제점은 부각됐고 네트워크를 가상화 하고자 SDN이 개발됐다.

> **Traditional networks rely on physical infrastructure** such as switches and routers to make connections and run properly. In contrast, a **software-based network allows the user to control the allocation of resources at a virtual network level** through the control plane. Rather than interacting with physical infrastructure, the user is interacting with software to provision new devices.

가상화를 통해 직접 장치에 접근할 필요 없이 관리 장치에서 소프트웨어를 활용해 몇가지 설정을 수정해 네트워크 설정을 변경하는 것이 가능해졌다. 

<b><u>SDN의 또 다른 주요 특징은 실제 데이터를 다루는 부분과 관리를 하는 부분을 분리해 소프트웨어 형식으로 네트워크를 구성한다는 점이다.</u></b>

SDN을 사용하면 이를 소프트웨어적으로 구분해 관리 소프트웨어가 설치된 기기에선 관리 작업만 수행하고 데이터 관련 소프트웨어가 설치된 곳에서는 데이터 전송 및 포워딩 작업만 수행한다. 이런 식으로 계층을 분리하면 유연한 대응이 가능해진다.

> [! SDN의 핵심]
> **SDN의 핵심은 네트워크의 가상화이다.** 기존에는 직접 하드웨어에 접근해서 설정을 변경해야 네트워크를 수정할 수 있었다. 하지만 SDN이 등장함으로써 네트워크를 가상화해 소프트웨어 레벨에서 관리하는 것이 가능해졌고 이로 인해 동적인 네트워크 설정이 가능해졌다.

___
### SDN의 구조

![](https://obs3dian.s3.ap-northeast-2.amazonaws.com/SDN%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-06-08%20%EC%98%A4%ED%9B%84%2012.40.34.png)

SDN은 위와 같은 구조로 설정되며 관리 영역과 데이터 영역이 구분된 것을 확인할 수 있다. 관리 영역이 데이터 영역을 전부 관리하고 데이터 영역은 데이터의 전송에만 신경쓴다. 

이때 각각의 데이터 영역은 flow table을 보유하고 있는데 이를 활용해 라우팅 및 포워딩 방법을 결정한다. 
___
### Open Flow

**오픈 플로우는 SDN을 활용하기 위해 사용하는 프로토콜로 SDN을 구현하기 위해 처음으로 제정된 표준 인터페이스이다.** 오픈 플로우는 다음과 같은 구조를 가진다. 

규칙과 액션 그리고 데이터로 구성되며 규칙은 해당 액션을 실행하는 조건, 액션은 실행할 동작을 의미한다.

![500](https://obs3dian.s3.ap-northeast-2.amazonaws.com/SDN%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-06-08%20%EC%98%A4%ED%9B%84%201.04.44.png)

규칙에는 주로 IP 주소와 포트번호가 사용되며 액션으로는 라우팅을 진행하거나 데이터를 버리는 등의 동작이 기록된다.

![500](https://obs3dian.s3.ap-northeast-2.amazonaws.com/SDN%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-06-08%20%EC%98%A4%ED%9B%84%201.05.56.png)

예를 들어 위의 이미지를 보면 10.3.으로 시작하는 모든 IP 주소는 3번 포트로 라우팅 된다.
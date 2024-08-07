#tcp 
### 출처
* https://evan-moon.github.io/2019/11/26/tcp-congestion-control/
___
### 개요
* [[#TCP 혼잡제어]]
* [[#혼잡 윈도우 (CWND)]]
* [[#AIMD]]
* [[#Slow Start]]
* [[#혼잡제어 정책들]]
* [[#TCP의 공정성]]
___
### TCP 혼잡제어

네트워크는 수많은 호스트들의 집합체인 만큼 명확히 규명할 수 없는 현상들이 종종 발생한다. <b><u>호스트들이 통신 하면서 네트워크 부하로 인해 통신 속도가 느려지면 네트워크가 혼잡하다는 것을 짐작할 수 있는데, 이러한 혼잡 상황을 해결하고 관리하는 행위를 혼잡 제어라 한다.</u></b>

혼잡제어는 흐름 제어와는 다른 개념으로 흐름제어는 데이터의 무결성이 중점으로 보는 반면 ==**혼잡 제어는 네트워크 상태 개선을 중점**==으로 본다. 또한 혼잡제어는 라우터가 진행하지만, 흐름제어는 통신의 양 끝단에서 진행한다.

혼잡 제어는 네트워크 상태 개선을 위해 혼잡 윈도우 사이즈를 활용하는데 이는 [[TCP 첫 걸음#TCP 세그먼트|TCP 세그먼트]]에 존재하는 속성으로 **송신자는 수신 측이 전송한 윈도우 사이즈와 혼잡 윈도우 사이즈를 비교해 더 작은 값만큼 패킷을 전송한다.**
___
### 혼잡 윈도우 (CWND)

**혼잡 윈도우는 수신 측에서 관리하지 않고 송신 측에서 네트워크 상황을 고려해 설정**한다. 따라서 수신 측의 상태를 기준으로 설정 했던  RWND(기존의 윈도우 사이즈)와 달리 다른 방법을 활용해 초기화를 진행한다. 

혼잡 윈도우의 초기 사이즈는 헤더 등을 제거한 순수 데이터의 크기인 MSS로 설정되고 이후 네트워크 상황을 반영해 크기를 조절한다. 크기를 조절하는 방식은 기본적으로 AIMD와 Slow-Start가 존재하고 현재 사용하는 방식의 원조격인 Tahoe와 Reno등이 존재한다.
___
### AIMD

AIMD는 윈도우 사이즈를 1씩 선형적으로 증가 시키다. 재전송이 발생하면 윈도우 사이즈를 절반으로 줄이는 방식이다. <b><u>AIMD는 단순하지만 공평한 방식인데, 이는 네트워크에 후반에 접근한 사용자도 결국 비슷한 윈도우 사이즈를 갖게 되기 때문이다. </u></b>

예를 들어 초기 사용자의 경우 트래픽 독점이 가능하므로 큰 혼잡 윈도우 사이즈를 갖지만, 이후 트래픽이 증가하면서 패킷이 유실될 확률이 증가하고 윈도우 사이즈가 줄어들게 된다. 반대로 후반에 접속한 사용자의 윈도우 사이즈는 작게 시작해 점진적으로 증가하게 된다.

AIMD는 결국 윈도우 사이즈를 공평하게 수렴 시키지만 수렴까지 오랜 시간이 소요되고 사이즈가 1씩 증가하기 때문에 그동안 대역폭을 전부 활용하지 못하는 비효율이 발생한다. **하지만 윈도우 사이즈가 어느 정도의 범위 내에서 유지돼 일정량의 전송을 꾸준히 지속할 수 있다.**
___
### Slow Start

**AIMD와 달리 윈도우 사이즈를 1씩 선형적으로 증가 시키지 않고 2의 제곱 단위로 증가 시킨다. 윈도우 사이즈는 2,4,8,..로 증가하다 오류가 발생하면 크기가 1이 된다.** 2의 지수 승으로 사이즈가 증가하기 때문에 대역폭의 임계치에 빠르게 도달해 네트워크 대역폭을 더욱 효과적으로 활용할 수 있는 방법이다.

**AIMD와 달리 윈도우 사이즈가 1로 급감하는 방식이기 때문에 윈도우 사이즈가 적절 범위에서 유지되지 않고 급등,급감이 발생한다.**

![500](https://obs3dian.s3.ap-northeast-2.amazonaws.com/TCP%20%ED%98%BC%EC%9E%A1%20%EC%A0%9C%EC%96%B4%20/%20Pasted%20image%2020231227184550.png)
___
### 혼잡제어 정책들

**혼잡제어 정책들은 혼잡 상태를 빠르게 감지해 재전송을 더욱 빠르게 실시하고 혼잡 상태에 최대한 영향을 받지 않도록 적절한 크기의 윈도우 사이즈를 설정하는 것을 목표로 한다.** 혼잡 제어 정책은 Tahoe, Reno등 여러개가 존재하는데 여기선 Tahoe만 간략하게 학습하도록 하자.

**Tahoe는 기본적으로 처음에는 Slow Start 방식을 사용하다가 네트워크가 혼잡하다고 느껴졌을 때는 AIMD 방식으로 전환하는 방법을 사용**하는 정책이다. 윈도우 사이즈는 지수 단위로 증가하다 특정 기준 이후로는 선형적으로 증가한다. 아래는 Tahoe와 Reno를 적용했을 때 혼잡 윈도우 사이즈의 변화 그래프이다.

![](https://obs3dian.s3.ap-northeast-2.amazonaws.com/TCP%20%ED%98%BC%EC%9E%A1%20%EC%A0%9C%EC%96%B4%20/%20Pasted%20image%2020231227185850.png)

**Tahoe는 윈도우 사이즈가 Threshold 값에 다다들 때 까진 Slow Start 방식을 사용하다 이후엔 AIMD로 전환한다.** Threshold는 임계치를 의미하며 이보다 윈도우 사이즈가 커질 경우 안전한 전송을 보장하지 못하는 단계를 말한다.

전송이 실패했을 경우에는 윈도우 사이즈를 1로 설정하고 **Fast Retransmit** 기법을 활용해 재전송을 재빠르게 처리하는 기법을 활용한다. **추가적으로 전송에 실패했을 경우 Threshold가 줄어드는 모습을 확인 할 수 있는데 이는 실패 지점을 반영해 더욱 안전한 통신을 위해 진행하는 조치**이다.

* **Fast Retransmit**
	**Tahoe에서 사용되는 기법중 하나로 TimeOut이 발생하기 이전에 오류를 감지하고 재전송을 실시하는 기법**을 말한다. Fast Retransmit은 동일한 ACK 패킷이 3번 전달 됐을 경우 전송 오류가 발생했다 판단해 타임아웃 이전에 재 전송을 실시한다.

> [!info]
> **혼잡제어 정책들의 목표는 "어떻게 혼잡을 빠르게 감지하고 윈도우 사이즈를 키우냐" 이다.**

___
### TCP의 공정성

<span class="red red-bg">TCP는 혼잡제어를 사용하는 방식으로 동작하기 때문에 같은 전송량을 가진 TCP 커넥션이 하나의 데이터 링크를 공유하면 평등한 데이터 처리량을 갖게 된다. </span>  하나의 데이터 링크를 공유하는 커넥션 A와 B가 존재하고 A가 먼저 네트워크를 전부 선점했다 가정해보자.

이 경우 이후에 접속한 B는 데이터를 전송하지 못할 것 같지만, 데이터 전송률은 동일하기 때문에 A와 동일한 비율로 B 패킷이 링크를 타고 전송된다. **이후 A는 B로 인해 링크를 나누면서 많은 패킷 손실이 발생하게 되고 이에따라 윈도우 사이즈를 줄이게 된다. 반대로 B는 점차 윈도우 사이즈를 키워나가면서 추후에는 비슷한 윈도우 사이즈에 수렴하게 된다.**

> [!info]
> **TCP는 결국 평등한 윈도우 사이즈를 가지게 된다.**

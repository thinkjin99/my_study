### 출처
* [파이썬 공식 문서](https://docs.python.org/ko/3.9/library/enum.html#enum.auto)
* [달서 블로그](https://www.daleseo.com/python-enum/)
___
### 개요
* [[#Enum]]
* [[#열거형과 패턴 매칭]]
* [[#Enum 믹스인]]
* [[#Flag]]
* [[#auto]]
___
### Enum

열거형은 고유한 상수 값을 잘 식별하고 구분하기 위해 사용하는 자료형으로 고유한 심벌과 값을 매핑하는 작업을 수행한다. 일반적인 열거형의 생김새는 아래와 같다.

```python
from enum import Enum
class Color(Enum):
	RED = 1
	GREEN = 2
	BLUE = 3
```

열거형은 유효한 값의 범위를 지정할 수 있기 때문에 여러개의 상수를 통해 관리를 하는 것에 비해 관리가 간편하다. 이는 가능한 타입이나 가질 수 있는 값들을 정의할 때 유용하게 활용할 수 있다.

또한 열거형을 활용할 경우 코드에 더욱 명확한 의미를 부여하는 것이 가능해진다. 아래의 코드를 살펴보자.

```python
RED = 1
print(RED)
>> 1

print(Color(RED))
>> <Color.Red 1>
```

일반 상수를 사용해서 활용할 경우 단순 값으로만 관리되기 때문에 값의 명확한 의미를 파악하기가 어렵다. 이때 Color를 활용하면 상수와 더불어 별도의 정보를 기록할 수 있기 때문에 해당 값의 의미가 무엇인지 명확히 기록할 수 있다.****

>[!info]
>**열거형을 활용하면 유효 값 집합을 정의하는 것이 가능하고 명확한 의미 부여 또한 가능해진다.** 

___
### 열거형과 패턴 매칭

열거형은 패턴매칭과 활용하면 직관적인 코드를 작성할 때 유리하다. 아래의 코드를 보자.

```python
color = Color(2)
match color:
	case Color.RED:
		print("red team")
	case Color.GREEN:
		print("gren team")
	case Color.YELLOW:
		print("yellow team")
```

만약 enum 타입을 활용하지 않는다면, 직접 if-else를 통한 분기를 진행해야 하는데 조건문의 나열로 인해 코드가 더욱 번잡해 질 수 있다.
___
### Enum 믹스인

enum 값은 그 자체로는 Enum 타입을 갖는다. 따라서 **실제 값이나 이름에 접근하기 위해서는 `.value`나 `.name`의 속성을 활용**해야 한다. 아래의 예제를 보자.

```python
_color = "RED"
if _color == Color.RED:
	#타입이 달라서 불일치 발생

if _color == Color.RED.name:
	#이런 식으로 비교를 진행해야 한다.
```

이는 생각보다 번거로움이 많다. 비교하려는 리터럴을 Enum으로 다시 변환하는 것도 불편하고 Enum을 리터럴과 비교하기 위해 속성 접근을 한번더 활용해야 하는 것도 편리하진 않다.

 이때 `__str__` 메서드를 오버라이딩하면 이러한 문제를 쉽게 해결할 수 있다. **믹스인 기법을 활용한 다중상속으로 처리 할 수 있다.** 

```python
from enum import Enum

class StrEnum:
    def __str__(self):
        return self.name

    def __eq__(self, other):
	    # == 연산자를 오버로딩해준다
        return str(self) == str(other)

class Color(StrEnum, Enum):
    RED = 1


red = Color(1)
print(red == "RED")

```

___
### Flag

**Flag는 enum타입의 멤버들 사이에서 비트 연산을 손쉽게 하기 위해 만들어진 베이스 클래스**이다. 해당 클래스를 상속 받으면 enum 클래스 멤버 속성을 기반으로 비트연산을 진행하는 것이 가능해진다. 아래의 코드를 보자.

```python
>>> from enum import Flag, auto
>>> class Color(Flag):
...     RED = auto()
...     BLUE = auto()
...     GREEN = auto()
...
color = Color(1)
#컬러가 빨간색 혹은 초록색인지 확인
color == Color.RED or color == Color.GREEN
>> True
# 비트 연산을 활용하면 아래와 같이 표현 가능하다
color & (Color.RED | Color.GREEN)
>> True

bool(Color.RED & Color.GREEN)
>> False
```

**Flag를 활용하면 멤버 요소를 비트 연산을 통해 조합해 손쉽게 비교하는 것이 가능해진다.** 특정 멤버 조합에 포함되지 않거나 포함되거나 하는 구분이 필요할 때 유용하게 사용할 수 있다.

Flag를 사용할 때 유의할 점은 멤버 변수에 직접적으로 값을 할당한다면 비트 연산을 위해 2의 배수 형태로 할당해야 한다는 점이다. 이에 따라서 특별한 경우가 아닌 이상 되도록 `auto`를 활용해 파이썬 인터프리터가 값을 자체 할당하게 설정하는 것이 편리하다.

>[!info]
>**Flag는 권한 관리 등을 검사할 때 편리하다.**

___
### auto

`auto`는 enum 멤버의 값을 자동으로 할당하는 기능을 하는 메서드이다. `auto`를 활용할 경우 1부터 시작해 1씩 증가하는 방식으로 값이 할당 되는데, 이는 enum 멤버의 경우 참으로 인식 돼야 하기 때문에 발생하는 현상이다.

또한 auto는 `_generate_next_value_` 메서드를 오버라이딩 함으로써 동작 방식을 변경할 수 있다. 예를 들어 자동으로 멤버 변수의 값이 멤버 이름으로 설정되는 것을 원할 경우 아래와 같이 설정할 수 있다.

```python
>>> class AutoName(Enum):
...     def _generate_next_value_(name, start, count, last_values):
...         return name

>>> class Ordinal(AutoName):
...     NORTH = auto()
...     SOUTH = auto()
...     EAST = auto()
...     WEST = auto()
```



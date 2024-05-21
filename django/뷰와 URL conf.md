
### 출처

___
### 개요

___
### 간단한 뷰와 URL 만들기

장고를 활용하기 위한 초 간단한 뷰를 하나 생성 해보자. 코드는 아래와 같다.

```python
#views.py
from django.http import HttpResponse, HttpRequest

def hello(request: HttpRequest):
	return HttpResponse("Hello World")
```

장고의 각 뷰 함수는 규칙에 의한 요청 이라는 매개변수를 하나 이상 사용한다. 여기서는 request를 통해 해당 매개변수를 받고 있고 이는 HTTP 요청 정보를 포함한다. 또한 뷰 함수는 반환 값으로 HTTPResponse를 반환해야 한다. (이 역시 규칙이다.)

이제 위에서 작성한 뷰를 호출하기 위해선 적절한 URL과 해당 뷰를 매핑하는 작업이 필요하다. urls.py로 이동해 URL 매핑을 수정해주자. /hello 위치로 요청이 들어오면 hello 함수를 실행하게 매핑을 진행한다.

```python
from django.contrib import admin
from django.urls import path
from views import hello

urlpatterns = [path("admin/", admin.site.urls), path("hello", hello)]
```

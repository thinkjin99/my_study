
### 출처

* [DRF 공식문서](https://www.django-rest-framework.org/api-guide/generic-views/)
___
### 개요

* [[#View]]
* [[#Generic Views]]
* [[#queryset]]
* [[#Generic View Methods]]
* [[#Mixin]]
___
### View

DRF의 뷰는 장고의 뷰 클래스와 흡사해 대부분의 내용은 [[뷰와 URL conf]]에서 다뤘던 뷰와 크게 다르지 않다. 

이전에 장고의 뷰를 다루며 우리는 뷰가 `HTTPRequest` 수신 및 응답하는 주체로 장고 내부에서 HTTP 요청 객체를 조작하고 적절한 응답을 작성해 내보내는 기능을 수행한다는 것을 학습했다.

DRF의 뷰 역시 동일한 기능을 수행하나 몇가지 차이점이 존재한다. DRF의 `APIView`는 장고가 사용하는 HTTP 응답과 요청을 나타내는 `HTTPRequest, HTTPResponse` 객체를 활용하지 않고 별도의 `Request, Response`라는 객체를 활용한다.

또한 `APIView`는 별도의 `authenication_classes`나 `permission_classes`를 가질 수 있으며 이는 인증과 허가 작업에서 활용된다. 아래에서 자세히 설명한다.

이외의 부분은 장고의 뷰와 거진 흡사하며, 리퀘스트가 전달 됐을 때 `dispatch`를 통해 요청에 따른 `get,post` 등의 적절한 메서드를 호출한다.

>[!Why API View?]
>API View를 활용하면 인증, 허가 등의 기능 구현이 쉬워지고 JSON을 통한 응답 직렬화 등에 간편함이 생긴다. 이 점이 기존 Django View와의 차이점이다.
>

___
### Generic Views

**`Generic View`는 `APIView`를 상속 받는 클래스로 각종 작업에 특화된 뷰를 만들기 위해 자주 활용되는 속성들을 모아 놓은 추상화된 뷰**를 의미한다. 제네릭 뷰에는 다음과 같은 속성들과 메서드들이 정의돼 있고 이러한 속성들은 이후에 이를 활용해 만들 `ListView, DetailView` 등에서 활용된다.
#### queryset
쿼리셋은 뷰에서 활용할 DB 데이터를 저장하는 속성으로 여기에는 ORM의 쿼리셋 객체가 저장된다. 쿼리셋 속성은 클래스가 생성될 때 만들어지고 이후 모든 인스턴스에서 공유된다. **쿼리셋은 lazy-loading하는 속성을 갖고 있기 때문에 실질적으로 쿼리 값을 활용하는 상황이 발생할 때까진 실제 쿼리가 처리되지 않는다**. 따라서 쿼리셋의 결과를 획득하고 싶다면 명시적으로 `get_queryset()`에 접근하는 것이 적절하다. 

물론 쿼리셋 속성에 직접 접근한 후 쿼리셋의 평가를 시도한다면 결과 값을 획득할 수도 있겠지만, 이렇게 할 경우 특정 메서드에서 평가한 시점을 기준으로 캐싱이 발생하고 이것이 다른 메서드에서도 전부 재사용 된다. **따라서 매 요청마다 DB의 상태를 반영하고 싶다면 `get_queryset()`을 통해 새로운 쿼리셋을 생성한 후에 접근해야 한다.**

```python hl:8,9,10,11,12,25
 def get_queryset(self):
        """
        Get the list of items for this view.
        This must be an iterable, and may be a queryset.
        Defaults to using `self.queryset`.

        This method should always be used rather than accessing `self.queryset`
        directly, as `self.queryset` gets evaluated only once, and those results
        are cached for all subsequent requests.

        You may want to override this if you need to provide different
        querysets depending on the incoming request.

        (Eg. return a list of items that is specific to the user)
        """
        assert self.queryset is not None, (
            "'%s' should either include a `queryset` attribute, "
            "or override the `get_queryset()` method."
            % self.__class__.__name__
        )

        queryset = self.queryset
        if isinstance(queryset, QuerySet):
            # Ensure queryset is re-evaluated on each request.
            queryset = queryset.all()
        return queryset
```

`get_queryset()`을 활용할 경우 쿼리셋의 결과 값이 반환되고 만약 캐시 데이터가 존재한다면 캐싱돼 있는 쿼리의 결과 값이 반환된다.
**쿼리셋의 결과를 획득하고 싶다면 `queryset` 속성에 직접적으로 접근 할 것이 아니라.  `get_queryset`을 명시적으로 호출해줘야 한다.** 

#### serializer_class
시리얼라이져 클래스 속성은 해당 뷰에서 직렬화 및 역 직렬화를 위해 사용할 시리얼라이져의 종류를 결정한다. 해당 속성을 통해 정의된 시리얼라이져는 `get_serializer()`에서 인스턴스로 생성돼 반환된다.

뷰에서 정의된 시리얼라이져는 `GET`요청을 수신했을 경우 쿼리셋의 결과 데이터를 직렬화해 전달하는 작업을 수행하고 `POST` 요청을 수신 받았을 경우 요청 내부의 데이터를 파이썬 객체로 역직렬화하고 유효성을 검증하는 작업을 수행한다.

#### lookup_fields
**룩업 필드는 모델에서 탐색을 위해 사용하는 필드로 기본 값은 `pk`이다.**  장고에서 url 설정을 진행하다보면 다음과 같이 설정하는 경우를 종종 확인했을 것이다. `path('books/<pk:int>/', BookDetailView.as_view(), name='book-detail'),` 이때 전달되는 파라미터의 중 `get_object`와 같은 작업에서 기본으로 활용할 키를 지정하는 속성이 바로 `loolup_fields` 속성이다.

기본 값으로는 기본 키를 활용해 탐색을 진행하기 때문에 복합 키를 활용하거나 별도의 탐색 로직이 필요한 경우 메서드를 오버라이딩해 구현해야 한다.

#### filter_backends
해당 속성은 클라이언트의 요청에 대한 결과를 필터링하는 것을 가능케 하는 속성으로 내부에는 쿼리셋을 필터링하기 위한 방법들을 정의하는 클래스들이 리스트 형태로 포함된다. 필터링, 검색, 정렬 등의 기능을 제공하며 다음과 같이 사용할 수 있다. 
필터는 주로 여러 개의 속성을 통해 값을 탐색해야할 때 활용하며 쿼리 파라미티러를 활용한다. 예를 들어 아래의 코드의 경우 `/api/books/?title=hello&author_name=kim` 과 같이 url을 전송하고 이를 토대로 필터링을 진행한다.

```python
from rest_framework import generics
from rest_framework import filters

class BookListView(generics.ListAPIView):
    queryset = Book.objects.all()
    serializer_class = BookSerializer
    filter_backends = [filters.SearchFilter, filters.OrderingFilter]
    search_fields = ['title', 'author__name']
    ordering_fields = ['title', 'publication_date']
```

___
### Generic View Methods

다음으로는 제네릭 뷰의 메서드들을 확인해보자. 제네릭 뷰에는 다음과 같은 메서드들이 존재한다. 
#### get_queryset
리스트 뷰 출력을 위한 쿼리셋을 반환한다. 해당 메서드를 호출 할 경우 쿼리셋의 실행 결과가 반환된다. 앞서 언급 했듯 직접 쿼리셋 속성에 접근하지 않고 해당 메서드를 호출해야 데이터베이스의 현재 데이터를 조회할 수 있고 결과 값을 가져오는 것이 가능하다.

#### get_object
하나의 객체의 속성을 자세히 보는 상세 뷰에 사용할 결과를 반환한다. 해당 메서드는 쿼리셋을 가져온 다음 `lookup_fileds`에서 정의한 필드 속성을 토대로 필터링을 적용한다. 즉 전체 쿼리셋에서 룩업 필드 속성으로 필터링한 결과를 반환한다 생각하면 된다.

``` python
def get_object(self):
    queryset = self.get_queryset()
    filter = {}
    for field in self.multiple_lookup_fields:
        filter[field] = self.kwargs[field]

    obj = get_object_or_404(queryset, **filter)
    self.check_object_permissions(self.request, obj) #객체에 대한 권한을 점검
    return obj
```

#### filter_queryset
필터 쿼리셋 속성을 활용해 현재 소유하고 있는 쿼리셋에 필터링을 적용한다. 이후 필터링을 적용한 쿼리셋을 반환한다. 

```python
def filter_queryset(self, queryset):
	"""
	Given a queryset, filter it with whichever filter backend is in use.

	You are unlikely to want to override this method, although you may need
	to call it either from a list view, or from a custom `get_object`
	method if you want to apply the configured filtering backend to the
	default queryset.
	"""
	for backend in list(self.filter_backends):
		queryset = backend().filter_queryset(self.request, queryset, self)
	return queryset
```

#### save and deletion hook
제네릭 뷰 내부에는 저장과 삭제를 위한 훅이 존재한다. 각각의 훅 메서드는 믹스인에서 DB에 인스턴스를 저장하거나 삭제할 때 호출된다. 이를 활용해 저장이전 혹은 이후에 수행해야할 작업을 쉽게 정의하는 것이 가능하다.
예를 들어 `perform_create` 메서드를 생각해보자. 이는 `createModelMixin` 내부의 `create`메서드가 호출하는 메서드이다.  아래 실제 코드를 확인해보자.

``` python hl:4
def create(self, request, *args, **kwargs):
	serializer = self.get_serializer(data=request.data)
	serializer.is_valid(raise_exception=True)
	self.perform_create(serializer)
	headers = self.get_success_headers(serializer.data)
	return Response(serializer.data, status=status.HTTP_201_CREATED, headers=headers)

def perform_create(self, serializer):
	serializer.save() #save만 수행
```

`perform_create` 메서드는 실질적으로 인스턴스를 저장하는 작업만을 수행한다. 따라서 이를 오버라이딩해 몇가지 작업을 추가해주면 인스턴스를 저장하기 이전 혹은 이후에 수행할 작업을 정의할 수 있다.

이와 같은 훅은 update와 delete에도 존재하기 떄문에 동일한 작업을 전부 수행하는 것이 가능하다.
___
### Mixin

믹스인에 대한 개념 자체는 [[뷰와 믹스인]]에서 설명했다. 짧게 복습하면 믹스인은 추가적인 기능을 덧 붙이기 위한 기능이 집합돼 있는 클래스이다. DRF의 믹스인에는 큰 특징이 존재하는데 **믹스인 내부의 메서드들은 각각의 메서드에 대응하지 않고 액션에 대응하게 설계 돼있다**.예를 들어 `get(), post()`와 같은 메서드들이 존재하지 않고 `retreive()`나 `create()`같은 메서드들이 존재한다. 

믹스인은 각각 리스트, 생성, 삭제, 획득, 수정 5개 존재한다.

#### Concrete View Classes
DRF 내부에는 이러한 믹스인과 제네릭 뷰를 상속받아 생성한 콘크리트 클래스 뷰가 존재한다. 해당 뷰는 상속받은 믹스인의 용도로만 사용하는 것이 일반적이다. 예를 들어 RetrieveAPIView의 경우 한개의 인스턴스만을 획득하는 read-only 엔드포인트에만 활용한다.






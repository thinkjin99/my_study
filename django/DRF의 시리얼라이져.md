#drf #serializer

### 출처
* [중첩 시리얼라이져](https://pyh0502.tistory.com/55)
* [공식문서](https://www.django-rest-framework.org/api-guide/serializers/)
___
### 개요
* [[#Serializer?]]
* [[#Validation]]
* [[#Nested Serilalizer]]
* [[#ModelSerializer]]
* [[#Context]]
___

### Serializer?

시리얼라이져는 말 그대로 직렬화와 역직렬화를 수행하는 객체이다. 복잡한 파이썬 객체를 JSON 형식의 데이터로 변환하거나 반대로 JSON 데이터를 파이썬 객체로 변환하는 작업을 수행한다.
추가적으로 시리얼라이져는 [[장고 폼]]과 비슷한 역할을 수행할 수 있으며 JSON 데이터의 유효성 검사, 필드 유형 변환, 관계 데이터 처리 등의 다양한 작업을 수행할 수 있다. 

시리얼라이져가 수행하는 작업을 정리하면 검증과 직렬화라고 생각하면 좋다. ==**시리얼라이져는 유저가 전달한 리퀘스트 파라미터를 검증하고 적절한 파이썬 객체로 역 직렬화한다. 또는 DB로 부터 원하는 형태의 데이터를 추출하고 이를 직렬화가 쉬운 형태로 파이썬 객체로 변환한다.**==


![](https://obs3dian.s3.ap-northeast-2.amazonaws.com/DRF%EC%9D%98%20%EC%8B%9C%EB%A6%AC%EC%96%BC%EB%9D%BC%EC%9D%B4%EC%A0%B8%20/%20%EC%8A%A4%ED%81%AC%EB%A6%B0%EC%83%B7%202024-08-23%20%EC%98%A4%ED%9B%84%208.43.46.png)

위의 이미지는 시리얼라이져와 모델 JSON 데이터 간의 상관 관계를 나타낸 이미지로 **시리얼라이져는 모델과 JSON Data사이의 API와 같은 역할을 수행한다.**

>[!info]
>시리얼라이져를 통해 리퀘스트를 검증하고 전송할 데이터를 시리얼라이져를 통해 직렬화해 전송한다. 시리얼라이져는 API의 시작과 끝에 존재한다.

시리얼라이져의 생김새는 다음과 같다. 기초적인 시리얼라이져를 생성해보자.

```python
from rest_framework import serializers

class CommentSerializer(serializers.Serializer):
    email = serializers.EmailField()
    content = serializers.CharField(max_length=200)
    created = serializers.DateTimeField()

comment = Comment.objects.get(id=1)
serializer = CommentSerializer(comment)
serializer.data
# {'email': 'leila@example.com', 'content': 'foo bar', 'created': '2016-01-27T15:17:10.375877'} 모델 인스턴스가 dict 형식으로 변환된 것을 확인할 수 있다.
```

위와 같이 시리얼라이져를 활용하면 모델 객체나 쿼리셋을 JSON 직렬화가 간단한 `dict`  형식으로 쉽게 변환 할 수 있다. 반대로 JSON 데이터를 파이썬 객체로 변환하는 것이 가능하다. JSON 형식의 리퀘스트 데이터를 파싱할 때 사용하면 유용하다.

```python
import io
from rest_framework.parsers import JSONParser

stream = io.BytesIO(json)
data = JSONParser().parse(stream)
```

시리얼라이져를 통해 파이썬 인스턴스 객체를 생성하기 위해서는 `create`나 `update`를 오버라이딩해야 한다. 

위의 코멘트 시리얼라이져를 확인해보자. 만약 해당 시리얼라이져를 통해 데이터를 역 직렬화하고 이를 DB에 신규 코멘트를 저장하고 싶다면 다음과 같이 작성해야 한다.

```python
class CommentSerializer(serializers.Serializer):
    email = serializers.EmailField()
    content = serializers.CharField(max_length=200)
    created = serializers.DateTimeField()

    def create(self, validated_data):
	    #신규 코멘트 인스턴스를 생성한다.
        return Comment.objects.create(**validated_data)

    def update(self, instance, validated_data):
        instance.email = validated_data.get('email', instance.email)
        instance.content = validated_data.get('content', instance.content)
        instance.created = validated_data.get('created', instance.created)
        instance.save()
        return instance
```

`create()`는 `serializer.save()`를 호출 했을 때 실행된다. 만약 **저장을 할때만 사용할 별도의 인수를 전달하고 싶다면 `save(your_arg=value)`형식으로 전달할 수 있다.** 이러한 방식을 사용하면 저장을 할 때만 사용할 인수를 넘길 수 있다. 주로 `user` 나 `created_at` 등의 유저의 입력과 별도로 결정돼 있는 데이터를 넘길 때 사용한다.

>[!info]
>**시리얼라이져는 장고의 form에 직렬화 기능과 역 직렬화 기능을 추가한 객체라고 생각하면 된다.**

___
### Validation

**역직렬화를 수행해 유효한 데이터에 접근하기 위해서는 항상 `is_valid()`를 실행해야 한다.** 이는 폼의 [[장고 폼#clean_{field_name}()|clean]] 과 흡사한 메서드로 **시리얼라이져로 들어온 데이터의 유효성을 검사하고 적절한 데이터를 초기화하는 작업을 수행한다.**

해당 메서드는 **데이터를 시리얼라이져의 각 필드와 바인딩하고 타입 등을 검사하는 작업을 수행**한다. 이후 유효한 데이터들이 `validated_data`에 저장된다. 만약 유효성 검사 중 에러가 발생하면 False를 검사를 전부 통과할 경우 True를 반환한다.

시리얼라이져에도 폼과 마찬가지로 필드가 존재하며 이를 통해 기본적인 인수검사와 타입 체크를 진행할 수 있다. 아래 코드를 확인해보자.

```python
from rest_framework import serializers

class BlogPostSerializer(serializers.Serializer):
    title = serializers.CharField(max_length=100)
    content = serializers.CharField()
	
	#value는 필드셋에 전달되는 값을 말한다.
    def validate_title(self, value):
        """
        Check that the blog post is about Django.
        """
        if 'django' not in value.lower():
            raise serializers.ValidationError("Blog post is not about Django") #Validation Error는 400번이다.
        return value
```

폼과 굉장히 흡사한 형태를 띄는 것을 확인할 수 있다. 몇가지 변화한 점이 있다면 **기존의 폼에서는 `clean`이라는 키워드로 유지되던 패턴이 `validate`로 변화 했을 뿐이다. 이외 요소는 실상 동일하게 동작**한다.

즉, `is_valid`가 호출될 때  `validate` 메서드가 호출되고 여기서 모든 `validate_fieldname` 형태의 메서드를 호출해 검증을 진행한다. **각 `validate_fieldname` 메서드에는 인자로 해당 필드에 입력된 값이 전달된다.** 
___
### Nested Serilalizer

시리얼라이져 내부의 필드에 시리얼라이져를 정의하는 것이 가능하다. 이 경우 필드의 타입이 해당 시리얼라이져가 된다. 아래의 코드를 보자.

```python
class CommentSerializer(serializers.Serializer):
    user = UserSerializer(required=False)
    edits = EditItemSerializer(many=True)  # A nested list of 'edit' items.
    content = serializers.CharField(max_length=200)
    created = serializers.DateTimeField()
```

시리얼라이져가 중첩돼 있을 경우 초기화는 아래와 같이 중첩 딕셔너리를 활용해 진행해야 한다. 

```python
serializer = CommentSerializer(data={'user': {'email': 'foobar', 'username': 'doe'}, 'content': 'baz'})
serializer.is_valid()
```

또한 만약 이러한 역직렬화 과정에서 문제가 발생할 경우 어떠한 시리얼라이져에서 문제가 발생했는지 알려 준다.

중첩 시리얼라이져에서 주요한 부분은 시리얼라이져를 중첩할 경우 `create`와 `update`를 직접 정의해줘야 한다는 것이다. 이는 `ModelSerializer` 등에서도 자동으로 구현해주지 않는다. 조회하는 기능은 자동으로 제공해 자동으로 여러 테이블의 정보를 엮어서 확인하는 것을 가능하게 해준다.
___
### ModelSerializer

**모델 시리얼라이져는 장고의 모델과 밀접한 관계를 갖는 시리얼라이져로 자동으로 특정한 모델에 알맞는 필드를 설정해준다**. 또한 각 모델의 알맞는 객체를 생성하고 업데이트하는 `create`와 `update` 메서드를 자동으로 구현 해준다. 이에따라 별도 코드 구현없이 `save` 메서드를 실행하는 것 만으로 DB에 모델을 저장하는 것이 가능해진다.

```python
class AccountSerializer(serializers.ModelSerializer):
    class Meta:
        model = Account
        fields = ['id', 'account_name', 'users', 'created'] #__all__을 할 경우 모든 필드이다.
 #exclude를 입력할 경우 특정 필드만 제외할 수 있다.
```

모델 객체는 시리얼라이져의 각 필드와 연동된다. 따라서 시리얼라이져의 필드 속성에 일부 속성만 정의하면 모델의 일부 속성만 가져오는 것이 가능해진다. **이때 주요한 점은 일부 속성만 가져온다고 해서 쿼리가 효율적으로 동작하는 것은 아니라는 것이다.** 필드는 시리얼라이져가 다룰 필드를 정의하는 곳이지, **실질적으로 쿼리를 효율적으로 생성하는 것은 쿼리셋에서 관리해야한다.**

#### 외래키 필드는?
`PrimaryKeyRelatedField`는 외래키를 다룰 때 사용하는 필드로 모델에 외래 키 제약이 존재하면 이 또한 자동으로 설정해 검사를 진행한다. 따라서 폼에서 별도로 검사를 진행했던 것 처럼 외래 키 제약 만족 검사를 진행할 필요가 없다. 외래키를 설정하는 방법으로는 중첩 시리얼라이져를 활용하거나 직접 커스텀을 하는 방법도 존재하기는 하나, 보통은 기본 키를 활용해 적용하는 방식을 활용한다.

>[!info]
>**모델 시리얼라이져는 연결된 모델의 컬럼 타입에 알맞게 자동으로 필드를 설정 해준다. 또한 DB에 객체를 추가하거나 업데이트 하는 기능들이 구현돼 있다.**

#### 모델 시리얼라이져와 CRUD
**모델 시리얼라이져는 특정한 모델과 곧장 연결되기 때문에 CRUD 작업을 구현할 때 유용**하다. 모델 시리얼라이져를 활용하면 리퀘스트가 특정 모델에 적절하게 전달 됐는지 검증하는 것이 가능하고, 모델의 쿼리셋 데이터를 JSON 형식으로 변환하는 것도 단순한다. 아래의 코드를 보자. 

```python
class CommentCreateSerializer(serializers.ModelSerializer):
    class Meta:
        model = Comment
        fields = ["id", "author", "post", "content", "created_at"]

    id = serializers.IntegerField(read_only=True)
    post = serializers.PrimaryKeyRelatedField(
        queryset=Post.objects.all(), required=True
    )
    author = serializers.PrimaryKeyRelatedField(read_only=True)
    content = serializers.CharField(max_length=100, required=True)

    def create(self, validated_data):
        validated_data["author"] = self.context[
            "request"
        ].user  # author값은 시스템에서 자동 초기화 해야한다. read_only는 save에 포함되지 않는다.
        return super().create(validated_data)
```

이러한 시리얼라이져가 존재할 때 뷰에서 모델을 생성하는 POST 요청을 처리한다고 해보자 생성을 수행해야 하므로 미리 정의된 `create` 메서드를 호출한다.

```python
 def create(self, request, *args, **kwargs):
	serializer = self.get_serializer(data=request.data)
	serializer.is_valid(raise_exception=True)
	self.perform_create(serializer)
	headers = self.get_success_headers(serializer.data)
	return Response(serializer.data, status=status.HTTP_201_CREATED, headers=headers)
```

시리얼라이져를 가져온 다음 `is_valid` 메서드를 호출만 하면 시리얼라이져에서 각 필드에 적절한 값이 전달 됐는지 검증하고 만약 문제가 발생했을 경우 `ValidationError`를 발생 시킨다. 이에따라 ==**전달 받은 리퀘스트를 기반으로 특정 모델을 생성할 때 복잡한 데이터 검증 작업을 수행할 필요가 없어진다.**==
이후에는 `perform_create` 를 통해 유효한 데이터를 모델 객체로 DB에 저장해주기만 하면 된다.

이번에는 GET 요청을 처리해보자. 쿼리셋 데이터를 JSON 포맷으로 반환하는 작업도 단순해진다. 쿼리셋을 가져온 다음 이를 시리얼라이져에 대입하기만 하면 된다. 

```python
def retrieve(self, request, *args, **kwargs):
	instance = self.get_object()
	serializer = self.get_serializer(instance)
	return Response(serializer.data)
```

**POST 요청을 처리할 때와 달리 별도의 유효성 검사를 진행하지 않는데, 이는 쿼리셋이 DB에서 추출한 데이터이기 때문이다.** 별도의 검증 로직이 존재하지 않는 이상 DB에서 추출한 데이터의 타입 검사 등을 수행할 필요는 없다. 이후 시리얼라이져의 `data` 속성을 추출하고 응답을 곧장 전송한다.

>[!info]
>**모델 시리얼라이져를 통해 CRUD 작업을 손쉽게 구현할 수 있다.**

___
### Context

시리얼라이져의 컨텍스트는 딕셔너리 객체로 직렬화나 역직렬화 과정중 **시리얼라이져에 특정한 데이터를 전달하기 위해 사용하는 공간을 말한다.**

컨텍스트를 통해 뷰에서 데이터를 전달할 수 있으며 이를 기반으로 시리얼라이져가 더욱 동적인 동작을 하는 것이 가능해진다.

예를 들어 시리얼라이져에서 현재 리퀘스트 정보를 활용해야 하는 상황이라 가정해보자. 이 경우 뷰에 존재하는 request 파라미터를 시리얼라이져에 전달하면 된다.

```python
class MyView(APIView):
    def get(self, request, *args, **kwargs):
        queryset = MyModel.objects.all()
        serializer = MyModelSerializer(queryset, many=True, context={'request': request}) #컨텍스트를 통해 데이터 전달
        return Response(serializer.data)
```

시리얼라이져에서는 컨텍스트를 통해 전달 받은 데이터를 다음과 같이 사용할 수 있다.

```python
class MyModelSerializer(serializers.ModelSerializer):
    class Meta:
        model = MyModel
        fields = '__all__'

    def to_representation(self, instance):
        representation = super().to_representation(instance)
        # context를 통해 request 객체에 접근
        request = self.context.get('request')
        if request and request.user.is_authenticated:
            representation['is_owner'] = (instance.owner == request.user)
        return representation
```

컨텍스트를 통해서 가장 많이 전달하는 객체는 request이며 ViewSet 클래스들은 기본적으로 시리얼라이져를 생성할 때 컨텍스트에 request를 전달한다. 

 




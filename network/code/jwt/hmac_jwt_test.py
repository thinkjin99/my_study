import jwt
import datetime
from cProfile import Profile
from pstats import Stats


# 비밀 키 설정
secret_key = "your_secret_key"


# 페이로드 데이터 설정
def jwt_encode():
    payload = {
        "user_id": 123,
        "username": "testuser",
        "exp": datetime.datetime.now() + datetime.timedelta(hours=1),  # 토큰 만료 시간
    }

    # JWT 토큰 생성
    token = jwt.encode(payload, secret_key, algorithm="HS256")
    return token


def jwt_decode(token):
    # 토큰 디코딩 (검증)
    decoded_payload = jwt.decode(token, secret_key, algorithms=["HS256"])
    return decoded_payload


def test():
    for _ in range(10000):
        token = jwt_encode()
        print(token)
        decode_token = jwt_decode(token)
        print(decode_token)


# profiler = Profile()
# profiler.enable()
test()
# profiler.disable()
# stats = Stats(profiler)
# stats.strip_dirs()
# stats.sort_stats("cumulative")
# stats.print_stats()

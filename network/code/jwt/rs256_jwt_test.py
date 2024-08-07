import jwt
import datetime

from cProfile import Profile
from pstats import Stats


# JWT 토큰을 생성하는 함수
def create_jwt_token(private_key):
    # 개인 키를 파일에서 읽어옵니다.
    payload = {
        "user_id": 123,
        "exp": datetime.datetime.now() + datetime.timedelta(hours=1),
    }
    token = jwt.encode(payload, private_key, algorithm="RS256")
    return token


# JWT 토큰을 검증하는 함수
def verify_jwt_token(token, public_key):
    try:
        # 공개 키를 파일에서 읽어옵니다.
        payload = jwt.decode(token, public_key, algorithms=["RS256"])
        return payload
    except jwt.ExpiredSignatureError:
        print("토큰의 유효 기간이 만료되었습니다.")
    except jwt.InvalidTokenError:
        print("유효하지 않은 토큰입니다.")


def test():
    with open("private_key.pem", "r") as f:
        private_key = f.read()
    with open("public_key.pem", "r") as f:
        public_key = f.read()

    for _ in range(1000):
        token = create_jwt_token(private_key)
        decoded = verify_jwt_token(token, public_key)


profiler = Profile()
profiler.enable()
test()
profiler.disable()
stats = Stats(profiler).strip_dirs()
stats.sort_stats("cumulative", "filename")
stats.print_stats()

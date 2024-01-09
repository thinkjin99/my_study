from fastapi import FastAPI
import time

import uvicorn
from uvicorn.config import LOGGING_CONFIG


app = FastAPI()


def run():
    LOGGING_CONFIG["formatters"]["access"]["fmt"] = (
        "%(asctime)s " + LOGGING_CONFIG["formatters"]["access"]["fmt"]
    )
    uvicorn.run("main:app", port=8000, reload=True)


# 블로킹 함수를 시뮬레이션하는 함수
def blocking_operation():
    time.sleep(5)  # 5초 동안 블로킹 작업 수행


@app.get("/")
async def read_root():
    blocking_operation()  # 블로킹 함수 호출
    return {"message": "Hello, World!"}


if __name__ == "__main__":
    run()

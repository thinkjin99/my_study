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


@app.get("/")
async def read_root():
    return {"message": "Hello, World!"}


if __name__ == "__main__":
    run()

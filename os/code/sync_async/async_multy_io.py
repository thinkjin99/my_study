import concurrent.futures
import urllib.request
import time
from functools import partial
import asyncio


COUNT = 10
URLS = [
    "http://www.foxnews.com/",
    "http://www.cnn.com/",
    "http://www.bbc.co.uk/",
] * COUNT


# Retrieve a single page and report the URL and contents
def load_url(url, timeout):
    with urllib.request.urlopen(url, timeout=timeout) as conn:
        print(f"Run {url}")
        # return conn.read()  # read is blocking
        return conn
        # return url, conn.status


def print_log(future: concurrent.futures.Future):
    res = future.result()
    print(f"page read {res}")


async def async_multy(url):
    loop = asyncio.get_event_loop()
    # partial을 통해 매개변수를 미리 입력해준다.
    requset = partial(urllib.request.urlopen, url, timeout=5)
    print(f"Run {url}")
    response = await loop.run_in_executor(
        None, requset
    )  # requset 대기 부분을 await로 처리해 비동기로 작동하게 한다.
    print(f"Done {url} page", end=" ")
    # print(f"Data bytes {len(response.read())}")
    return url, response


async def main():
    start = time.time()
    futures = [asyncio.create_task(async_multy(url)) for url in URLS]
    await asyncio.gather(*futures)
    end = time.time()
    print(f"Asyncio Runtime: {end - start}")


def future_multy():
    # We can use a with statement to ensure threads are cleaned up promptly
    with concurrent.futures.ThreadPoolExecutor(max_workers=32) as executor:
        # Start the load operations and mark each future with its URL
        for url in URLS:
            future = executor.submit(load_url, url, 5)
            future.add_done_callback(print_log)


def sync_multy():
    for url in URLS:
        res = load_url(url, 5)
        print(f"Done {url} page {res}")


def timer(func, *args, **kwargs):
    start = time.time()
    func(*args, **kwargs)
    end = time.time()
    print(f"{func.__name__} Runtime: {end - start}")
    print("-" * 40)


if __name__ == "__main__":
    # timer(sync_multy)
    # timer(future_multy)
    asyncio.run(main())

import time
import urllib3


# HOST = "10.21.20.226"
HOST = "https://c594-219-255-158-170.ngrok-free.app"


def connection_not_reuse_test(try_cnt: int):
    total = 0
    for i in range(try_cnt):
        start = time.time()
        pool_ = urllib3.PoolManager()
        resp = pool_.request("GET", f"{HOST}")
        end = time.time()
        print(f"request: {i} is done {resp} time:{end - start}")
        total += end - start
    return total


def connection_reuse_test(try_cnt):
    # pool = urllib3.HTTPConnectionPool(host=HOST, block=True, maxsize=1)
    pool = urllib3.connection_from_url(HOST)
    total = 0
    for i in range(try_cnt):
        start = time.time()
        resp = pool.urlopen("GET", HOST)
        end = time.time()
        print(f"request: {i} is done {resp} time:{end - start}")
        total += end - start
    return total


def run_test(reuse: bool, try_cnt: int = 10):
    if reuse:
        total = connection_reuse_test(try_cnt)
    else:
        total = connection_not_reuse_test(try_cnt)

    print(f"Total:{total}, avg sec: {total / try_cnt}")
    return


if __name__ == "__main__":
    run_test(False, 10)

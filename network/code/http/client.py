import time
import concurrent.futures
import urllib3


ENDPOINT = "http://10.14.5.88:8080"


def send_recv(pool: urllib3.HTTPConnectionPool | None = None):
    start = time.time()
    if pool:
        resp = pool.urlopen("GET", ENDPOINT)
    else:
        pool_ = urllib3.PoolManager()
        resp = pool_.request("GET", ENDPOINT)

    end = time.time()
    print(resp.data[:10])
    print(f"time: {end - start}")
    return end


if __name__ == "__main__":
    REQCNT = 200
    with concurrent.futures.ThreadPoolExecutor(max_workers=32) as executor:

        pool = urllib3.connection_from_url(ENDPOINT)

        futures = [executor.submit(send_recv) for _ in range(REQCNT)]  # no reuse
        # futures = [executor.submit(send_recv, pool) for _ in range(REQCNT)]  # yes reuse

        total = 0
        start = time.time()
        for i, f in enumerate(concurrent.futures.as_completed(futures), start=1):
            print(f"{i} is completed Sec: {f.result()}")
            total += f.result()

    end = time.time()
    print(f"Total:{end-start}, avg sec: {total / REQCNT}")

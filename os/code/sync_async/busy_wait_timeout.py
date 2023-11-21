import random
import time


def job(total_time: int):
    for i in range(total_time):
        time.sleep(1)
        run_time = time.time()
        print(f"Job runs {i} sec...")
        yield run_time


def timer(timeout: int):
    start_time = time.time()
    total_time = random.randint(5, 10)
    job_gen = job(total_time)
    print(f"Job takes {total_time} timeout:{timeout} seconds")
    while timeout > next(job_gen) - start_time:  # busy-waiting으로 구현
        pass


if __name__ == "__main__":
    timer(4)

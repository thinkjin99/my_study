import random
import time
import threading

mutex = threading.Lock()
run_time = 0
end_sig = 0


def job(total_time: int):
    for i in range(total_time):
        time.sleep(1)

        mutex.acquire()
        if end_sig:
            return

        global run_time
        run_time = time.time()

        print(f"Job runs {i} sec...")

        mutex.release()
    return


def timer(timeout: int):
    start_time = time.time()
    total_time = random.randint(5, 10)
    job_t = threading.Thread(target=job, args=(total_time,))
    print(f"Job takes {total_time} timeout:{timeout} seconds")
    job_t.start()

    while True:
        mutex.acquire()  # acquire lock is not busy wait..
        if (run_time - start_time) > timeout:
            global end_sig
            end_sig = 1
            break
        mutex.release()

    mutex.release()


if __name__ == "__main__":
    t1 = threading.Thread(target=timer, args=(3,))
    t1.start()

    # t2.join()

import random
import time
import threading

event = threading.Event()
run_time = 0
end_sig = 0


def job(total_time: int):
    for i in range(total_time):
        time.sleep(1)

        if end_sig:
            break

        global run_time
        run_time = time.time()
        event.set()
        print(f"Job runs {i} sec...")

    return


def timer(timeout: int):
    start_time = time.time()
    total_time = random.randint(5, 10)
    job_t = threading.Thread(target=job, args=(total_time,))  # start new thread
    print(f"Job takes {total_time} timeout:{timeout} seconds")
    job_t.start()

    while True:
        event.wait()  # use envent to remove busy - wait
        print("busy wait...")
        if (run_time - start_time) > timeout:
            global end_sig
            end_sig = 1
            break
        event.clear()


if __name__ == "__main__":
    t1 = threading.Thread(target=timer, args=(3,))
    t1.start()

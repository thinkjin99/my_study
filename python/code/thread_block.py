import threading
import time


def foo():
    print("sleep start")
    time.sleep(5)
    print("sleep end")


def foo2():
    for i in range(5):
        print(f"{i} is finished")


count_thread = threading.Thread(target=foo)
count_thread.start()

foo2()

count_thread.join(0)

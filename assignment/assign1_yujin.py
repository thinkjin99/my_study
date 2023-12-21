import time
import random
from threading import Event, Thread

def timer(stop_event):
    time.sleep(1) # n이 2일 경우 running은 1번만 출력돼야 함 (2초 실행되면 종료돼야 함으로) 그래서 1초 일부러 쉼
    m = random.randint(1, 5)
    print(m) #혹시나 해서 출력 해봄
    for _ in range(m):
        if stop_event.is_set(): #결국은 n초 동안해야 해서 
            break
        print("running..")
        time.sleep(1)

if __name__ == "__main__":
    stop_event = Event()
    n = int(input("Enter a number: "))
    t = Thread(target=timer, args=(stop_event,))
    t.start()
    time.sleep(n) # n초 세기
    stop_event.set() 
    t.join()

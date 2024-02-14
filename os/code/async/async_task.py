import asyncio
import time


async def timer(n):
    print(f"{n} wait")
    await asyncio.sleep(n + 1)
    print(f"{n} wait finish")


async def main():
    print("start:", time.time())
    tasks = [asyncio.create_task(timer(i)) for i in range(2)]
    await asyncio.sleep(1)
    for t in tasks:
        print(t._fut_waiter)
        # await t

    await asyncio.gather(*tasks)
    print("end:", time.time())


asyncio.run(main())

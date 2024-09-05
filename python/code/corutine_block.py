import asyncio
import time


async def foo():
    print("sleep")
    time.sleep(5)
    print("sleep end")


async def foo2():
    for i in range(3):
        print(f"{i} is finished")


async def main():
    task1 = asyncio.create_task(foo())
    task2 = asyncio.create_task(foo2())
    await asyncio.gather(task1, task2)


asyncio.run(main())

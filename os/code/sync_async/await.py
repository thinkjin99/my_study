import asyncio
import random


async def do_something(i: int):
    sec = random.randint(1, 10)
    print(f"{i} Wait for {sec}...")  # random wait...
    await asyncio.sleep(sec)  # sleep for secs
    print(f"{i} waiting done")


async def main():
    tasks = [asyncio.create_task(do_something(i)) for i in range(3)]
    # await asyncio.gather(*tasks) #wait for end...


if __name__ == "__main__":
    asyncio.run(main())

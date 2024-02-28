import asyncio
import random
import time


def saync_random_timer(i):
    pending_time = random.randint(1, 3)
    print(f"Wait {i} for {pending_time}")
    time.sleep(pending_time)
    print(f"{i} is finished...")
    return


def sync_main():
    for i in range(5):
        saync_random_timer(i)


async def random_timer(i):
    pending_time = random.randint(1, 3)
    print(f"Wait {i} for {pending_time}")
    await asyncio.sleep(pending_time)
    print(f"{i} is finished...")
    return


async def main():
    tasks = [asyncio.create_task(random_timer(i)) for i in range(5)]
    print(asyncio.all_tasks())
    await asyncio.gather(*tasks)

    # for i in range(5):
    # await random_timer(i)


asyncio.run(main())

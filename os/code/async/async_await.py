import asyncio


async def aprint(s):
    print(s)


async def forever(s):
    while True:
        await aprint(s)
        # await asyncio.sleep(0)


async def main():
    await asyncio.gather(forever("a"), forever("b"))


asyncio.run(main())

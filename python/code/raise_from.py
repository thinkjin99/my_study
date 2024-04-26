try:
    print("No exception")
    raise ValueError("Value Error1")

except ValueError as e:
    try:
        print(e.__cause__)
        print(e.__context__)
        raise IndexError("Index Error1") from None

    except IndexError as e:
        print(e.__cause__)
        print(e.__context__)
        print(e.__suppress_context__)
        raise Exception

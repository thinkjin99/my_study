class RangeIterator:
    def __init__(self, start, end):
        self.start = start
        self.end = end

    def __iter__(self):
        # 이 객체 자체가 반복 가능하므로 self를 반환
        return self

    def __next__(self):
        # 현재 값이 범위를 벗어나면 StopIteration 예외를 발생시킴
        if self.start >= self.end:
            raise StopIteration
        else:
            # 현재 값 반환 후 다음 값으로 이동
            result = self.start
            self.start += 1
            return result


# RangeIterator를 사용하는 예제
if __name__ == "__main__":
    my_range = RangeIterator(1, 6)

    # for 문을 통한 반복
    for num in my_range:
        print(num)

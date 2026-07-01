import time


def fib(n: int) -> int:
    if n <= 1:
        return n
    return fib(n - 1) + fib(n - 2)


t1 = time.perf_counter()
r = fib(30)
t2 = time.perf_counter()

print("Result:", r)
print("Time:", (t2 - t1) * 1000, "ms")
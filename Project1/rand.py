import random

def gen(n):
    for i in range(n):
        print(random.randint(1, 100_000))

gen(1_000)
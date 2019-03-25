def find_prime_iterative(initial):
    prime = 1
    curr = 1
    prime_count = 0

    while prime_count < initial:
        if prime_count % 1000 == 0:
            print("prime_count ", prime_count)
        for denom in range(2, curr):
            if curr % denom == 0:
                curr = curr + 1
        prime = curr
        curr = curr + 1
        prime_count = prime_count + 1

    return prime

result = find_prime_iterative(20000)
print("result = ", result)

function find_prime_iterative(initial)
	prime = 1
	curr = 1
	prime_count = 0

	while prime_count < initial do
		if prime_count % 1000 == 0 then
			print("prime_count = " .. tostring(prime_count))
		end
		for denom=2, curr do
			if curr % denom == 0 then
				curr = curr + 1
			end
		end
		prime = curr
		curr = curr + 1
		prime_count = prime_count + 1
	end

	return prime;
end

result = find_prime_iterative(20000)
print("result = " .. tostring(result))

table = []

# I first have to get the value from the array. Then I check if the array
# entry makes sense.
#
# There should only be one if statement. It will check if the value returned
# makes sense given the requested range.
#
# If we only allow 10 bits to be used, the mask should never be greater than
# 1111111111 in binary.
#
# Only after this check can the table be used.
#

def first_bit_set(val):
	for x in range(0,31):
		if ((val >> x) & 1) == 1:
			return x
	return 0xFF

for x in range(0,127):
	table.append(first_bit_set(x))

print("{")

for x in table:
	print("\t" + str(x) + ",")

print("}")

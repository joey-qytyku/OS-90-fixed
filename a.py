def scan(byte):
	b = byte
	for n in range(0, 8):
		if (b>>n) & 1 == 1:
			return n

print("unsigned char lut[256] = {")
for m in range(1, 256):
	print(f"\t[{m}] = {scan(m)}" + ("," * (m!=255)) )

print("};")
# This script adds the stack trace info using the sizetest object

import subprocess

result = subprocess.run("nm sizetest.o", shell=True, stdout=subprocess.PIPE, text=True)

table = result.stdout.split("\n")
table.pop()

functions = []

for x in table:
    if x[9] == "T" or x[9] == "t":
        functions.append(x)

data = []

for x in functions:
    data.append(x[11:])
    data.append(int(x[0:7], base = 16))

print(data)

# Generate the byte that will be written
bytelist = bytearray()

for x in data:
    if type(x) == str:
        bytelist += x.encode("utf-8") + b'\0'
    elif type(x) == int:
        bytelist += x.to_bytes(4, 'little')

bytelist += b'\0' * (4096 - (len(bytelist) % 4096))

kernel_image = open("build/KERNL386.EXE", "ab")

kernel_image.write(bytelist)

# Fast large memcpy

For very large blocks, there is no harm in performing fast block copies with extra calculations to ensure speed.

Memory alignment penalties are very harsh in 32-bit mode. Up to 6 clocks can be lost with each access. Even if a 32-bit access is done for less instructions executed, the result is still significantly slower.

To improve this, there is a method of auto-alignment.

## How It Works

ESI_d = if (src not aligned) (src + 3) & (~0b11);
EDI_d = if (dest not aligned) (dest + 3) & (~0b11);

>> Could be better, but for simplicity
ECX_d = (count - 8) >> 2

PERFORM DWORD COPY

The next part is to copy the head and the tail of the block.
Assume ECX_d is saved and restored.

With this knowledge, it is possible to calculate the tail and head bytes.
Then we do a movsb

number_of_unaligned_bytes = count - ECX_d*4

The number of unaligned bytes in the head and tail are inverse proportional.

The source pointer, when aligned and the original is subtracted, gives the tail bytes.

We do not use the destination pointer because that does not represent the block itself. That is where it goes to.

head_bytes = source & 0b11
tail_bytes = 4 - head_bytes

The head is simply enough to copy, just use the function parameters
The tail is copied using:
	ESI = source + count - head_bytes
	EDI = dest   + count - head_bytes
	ECX = head_bytes

NOTE: The input to this must be large, probably 80 bytes or more. That way it is actually worth the calculations.

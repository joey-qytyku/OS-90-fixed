$options = @(
	"-q"            # Do not display copyright notice line noise
	"-c"            # Compile to object only
	"-wx"           # Maximum warnings
	"-s"            # Disable stack overflow checking (need to? Think so.)
	"-onatx"        # Fastest code possible. Trust me bro.
			# Enables:
			#       on: Float opts. Not relevant because kernel.
			#       oa: Relax alias checking
			#       ot: Optimize for speed
			#       ox: Pipeline opts, branch opts, etc.
	"-oh"           # Multi-pass optimization
	"-ol"           # Loop opts
	"-or"           # Reorder instructions, not bad for i386, good for 486+
	"-fi=TYPE.H"    # Preinclude TYPE.H
	"-ecc"          # Use cdecl by default.
	"-zp8"          # Align data by 8. Better for i486.
	"-zl"           # Do not reference libraries in object files
	"-zld"          # Do not place dependency information in object files
	"-zls"          # Remove automatically inserted symbols
)

# /onatx /oh /oi+ /ei /zp8 /3 /fp3

wcl386 $options *.c
wasm $(dir -Name -Filter *.ASM)
cmd /c wlink `@`link.lnk
rm *.obj

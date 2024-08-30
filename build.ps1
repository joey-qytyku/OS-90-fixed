rm TRANSFER\*

cd KRNL
.\0BUILD.PS1
copy KRNL386.SYS ..\TRANSFER

cd ..

cmd /C bochs.exe -f runconf\bochsrc.bxrc

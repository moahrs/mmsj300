rem copy C:\Ide68k\Examples\mmsj_os.hex . /Y
mot2bin -p 0 mmsjos.hex
copy mmsjos.bin MMSJOS.SYS
copy /Y MMSJOS.SYS HD_ATU
copy /Y MMSJOS.SYS F:

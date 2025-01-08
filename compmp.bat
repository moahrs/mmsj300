REM copy D:\PROJETOS\MMSJ300\%1.hex . /Y
mot2bin -p 0 %1.hex
copy /Y %1.bin HD_ATU\%1.BIN

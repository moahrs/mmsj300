cls
c:\borland\bcc55\bin\bcc32 -c -tW -I"c:\borland\bcc55\include" -IH:\trabalho -L"c:\borland\bcc55\lib" %1.cpp
c:\borland\bcc55\bin\ilink32 -ap -c -x -Gn -L"c:\borland\bcc55\lib" %1.obj c0x32.obj,%1.exe,,import32.lib cw32.lib mpusbapi.lib


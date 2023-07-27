10 REM Sierpinski Carpet using recursion
20 HOME : HGR : HCOLOR=5
30 DIM XS(7): DIM YS(7): DIM XL(7):DIM YT(7)
40 LET XS(1) = 256 : YS(1) = 192 : XL(1)=0 : YT(1)=0
50 LET C = 1 : GOSUB 100
60 STOP
100 XD = XS(C) : YD = YS(C) : X = XL(C) : Y = YT(C) 
105 D%=INT(C):E%=INT(XD):F%=INT(YD):G%=INT(X):H%=INT(Y):
110 IF (C > 7 OR XD < 1) OR YD < 1 THEN RETURN
120 REM 
130 HPLOT X + XD/3, Y + YD/3 TO X + 2*XD/3, Y + YD/3
140 HPLOT X + 2*XD/3, Y + YD/3 TO X + 2*XD/3, Y + YD*2/3
150 HPLOT X + 2*XD/3, (Y + 2*YD/3) TO X + XD/3, Y + 2*YD/3
160 HPLOT (X + XD/3), (Y + YD/3) TO X + XD/3, Y + 2*YD/3
200 REM drawing TOP LEFT
210 XS(C+1) = XS(C)/3
220 YS(C+1) = YS(C)/3
230 XL(C+1) = XL(C) : YT(C+1) = YT(C) : C = C+1 : HCOLOR=6 : GOSUB 100
240 REM drawing TOP RIGHT
250 C = C - 1
260 XS(C+1) = XS(C)/3
270 YS(C+1) = YS(C)/3
280 XL(C+1) = XL(C) + XS(C+1)*2 : YT(C+1) = YT(C) : C = C+1 : HCOLOR=12 : GOSUB 100
290 REM drawing BOTTOM LEFT
300 C = C - 1
310 XS(C+1) = XS(C)/3
320 YS(C+1) = YS(C)/3
330 XL(C+1) = XL(C) : YT(C+1) = YT(C) + YS(C+1)*2 : C = C+1 : HCOLOR=7 : GOSUB 100
340 C = C - 1
350 XS(C+1) = XS(C)/3
360 YS(C+1) = YS(C)/3
370 XL(C+1) = XL(C) + XS(C+1)*2 : YT(C+1) = YT(C) + YS(C+1)*2 : C = C+1 : HCOLOR=14 : GOSUB  100
380 C = C - 1
390 XS(C+1) = XS(C)/3
400 YS(C+1) = YS(C)/3
410 XL(C+1) = XL(C) + XS(C+1) : YT(C+1) = YT(C) : C = C+1 : HCOLOR=15 : GOSUB 100
420 C = C - 1
430 XS(C+1) = XS(C)/3
440 YS(C+1) = YS(C)/3
450 XL(C+1) = XL(C) : YT(C+1) = YT(C) + YS(C+1) : C = C+1 : HCOLOR=15 : GOSUB 100
460 C = C - 1
470 XS(C+1) = XS(C)/3
480 YS(C+1) = YS(C)/3
490 XL(C+1) = XL(C) + XS(C+1)*2 : YT(C+1) = YT(C) + YS(C+1) : C = C+1 : HCOLOR=15 : GOSUB 100
500 C = C - 1
510 XS(C+1) = XS(C)/3
520 YS(C+1) = YS(C)/3
530 XL(C+1) = XL(C) + XS(C+1) : YT(C+1) = YT(C) + YS(C+1)*2 : C = C+1 : HCOLOR=15 : GOSUB 100
540 C = C - 1
600 RETURN
620 FOR K=1 TO 7500:NEXT K
620 FOR K=1 TO 7500:NEXT K

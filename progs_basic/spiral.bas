5 REM drawing a spiral with COS and SIN
10 HGR 
20 R = 5: A = 0
30 HCOLOR = 3 : REM set color to white
40 LET SX = INT (256/2)-0.5
50 LET SY = INT (192/2)-0.5
60 LET PI = 3.14159
70 HPLOT SX + R, SY
80 A = A + (PI/60)
90 LET Y = SIN(A) * R
100 LET X = COS(A) * R 
110 HPLOT TO SX + X,  SY + Y
120 R = R + 0.1
130 IF R < 96 THEN GOTO 80
140 FOR K=1 TO 3000:NEXT K

5 REM DRAW FIBONACCI
10 HGR : CLEAR : HCOLOR=5
20 HPLOT 0,96 TO 255,96 
30 HPLOT 128,0 TO 128,191
40 HCOLOR=7
50 X = 0 : Y = -1
60 D = 0
70 M = 3
80 CX = INT(256/2)-0.5 : CY = INT(192/2)-0.5
100 A = 0 : B = 1
110 C = B : GOSUB 300
120 A = A + B
130 IF E > 0 THEN GOTO 10
140 C = A : GOSUB 300
150 B = A + B
160 IF E > 0 THEN GOTO 10
170 GOTO 110
180 END
300 REM draw square size C by C with starting corner at (X,Y)
310 IF D = 0 THEN DX = 1  : DY = 1 
320 IF D = 1 THEN DX = -1 : DY = 1 
330 IF D = 2 THEN DX = -1 : DY = -1 
340 IF D = 3 THEN DX = 1  : DY = -1
350 DX = DX * C : DY = DY * C
360 GOSUB 400
370 R = C
371 IF D=0 THEN X1 = X : Y1 = Y + DY
372 IF D=1 THEN X1 = X + DX : Y1 = Y
373 IF D=2 THEN X1 = X : Y1 = Y + DY
374 IF D=3 THEN X1 = X + DX : Y1 = Y
375 GOSUB 600
380 X = X + DX : Y = Y + DY
385 D = D + 1
386 IF D = 4 THEN D = 0
390 RETURN
400 ONERR GOTO 470
410 HPLOT X * M + CX, Y * M + CY
420 HPLOT TO (X + DX) * M + CX, Y * M + CY
430 HPLOT TO (X + DX) * M + CX, (Y + DY) * M + CY
440 HPLOT TO X * M + CX,  (Y + DY) * M + CY
450 HPLOT TO X * M + CX,  Y * M + CY
460 GOTO 500
470 E = 1
500 RETURN
600 REM draw arc, centered at X1,Y1, radius R, quarter of circle.
610 ONERR GOTO 710
620 PI = 3.14159
630 RD = PI * (D - 1)/2
640 HCOLOR=15
650 HPLOT CX + M * (X1 + COS(RD) * R), CY + M * (Y1 + SIN(RD) * R)
660 FOR G = RD to (RD + PI/2) STEP 0.05
670 HPLOT TO CX + M * (X1 + COS(G) * R), CY + M * (Y1 + SIN(G) * R)
680 NEXT G
690 HCOLOR=15
700 GOTO 720
710 E = 1
720 RETURN


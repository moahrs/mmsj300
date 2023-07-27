5 REM Snake game prototype in text mode
10 HOME
20 LET X = 20 : Y = 12: DX = 1: DY = 0
30 GET K$
35 GOSUB 230
40 GOTO 30
50 END
230 IF K$="s" THEN DY = 1 : DX = 0
240 IF K$="w" THEN DY = -1 : DX = 0
250 IF K$="a" THEN DX = -1 : DY = 0
260 IF K$="d" THEN DX = 1 : DY = 0 
261 HTAB X : VTAB Y
262 PRINT " "; 
265 X = X + DX : Y = Y + DY
266 IF Y > 23 THEN Y = 23 : DX = 1 : DY = 0 : IF X = 39 THEN DX = -1
267 IF Y < 1 THEN Y = 1 : DX = 1 : DY = 0 : IF X = 39 THEN DX = -1
268 IF X < 1 THEN X = 1 : DX = 0 : DY = 1 : IF Y = 23 THEN DY = -1
269 IF X > 39 THEN X = 39 : DX = 0 : DY = 1 : IF Y = 23 THEN DY = -1
270 HTAB X : VTAB Y
280 PRINT "#"; 
300 RETURN



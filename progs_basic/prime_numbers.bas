05 REM prime sieve - remembering the found PR in array for quick lookup
10 TEXT : HOME : CP = 340
20 DIM PR(CP)
30 LET PR(1) = 2 : MX = 1
40 PRINT "Prime numbers:"
50 FOR X = 3 to 1000
60 IF (MX = CP) then GOTO 100
70 GOSUB 1000
80 IF P = 1 THEN PRINT INT(X);" ";
90 NEXT X
100 END
1000 REM check if X is prime
1020 REM PR(D) is used as possible divisors
1030 REM P is used for return value, if X is prime, P will be 1
1040 LET P = 0
1050 FOR D = 1 TO MX
1065 IF (PR(D) * PR(D)) > X THEN GOTO 1100
1070 LET Q = X/PR(D)
1080 IF (Q = int(Q)) THEN GOTO 1130
1090 NEXT D
1100 LET P = 1
1110 LET PR(MX + 1) = X
1120 LET MX = MX + 1
1130 RETURN

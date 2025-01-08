20 LET F = 99999
30 HOME : GR
40 LET M% = 18: LET N% = 4
60 LET D% = 0:tp%=0
1000 FOR I% = 0 TO 15
1010 READ A$
1020 FOR J% = 1 TO 19
1025 IF MID$ (A$,J%,1) = "A" THEN COLOR= 3: PLOT 2 * J%,2 * I%: PLOT 2 * J% + 1,2 * I%: PLOT 2 * j%, 2 * I% + 1: PLOT 2 * J% + 1,2 * I% + 1
1030 IF MID$ (A$,J%,1) = "." THEN COLOR= 12: PLOT 2 * J%,2 *I%: COLOR= 1: PLOT 2 * j% + 1, 2 * i%:tp%=tp%+1
1050 NEXT J%
1060 NEXT I%
1070 LET TI% = 0
1075 tp% = tp% - 1:D% = tp%
1077 htab 5:vtab 35:print d%
1080 COLOR= 6: PLOT 2 * M%,2 * N%: PLOT 2* m% + 1,2 * N%: PLOT 2 * M%,2 * N% + 1: PLOT 2* M% + 1,2 * N% + 1
1090 LET PM% = M%: LET PN% = N%
1095 get x$
1097 tc%=asc(x$)
1100 IF tc% = 18 AND SCRN( 2 * (M% - 1),2 * N%) <> 3 THEN LET M% = M% - 1
1110 IF tc% = 20 AND SCRN( 2 * (M% + 1),2 * N%) <> 3 THEN LET M% = M% + 1
1120 IF tc% = 17 AND SCRN( 2 * M%,2 * (N% - 1)) <> 3 THEN LET N% = N% - 1
1130 IF tc% = 19 AND SCRN( 2 * M%,2 * (N% + 1)) <> 3 THEN LET N% = N% + 1
1140 IF SCRN( 2 * M%,2 * N%) = 12 THEN LET D% = D% - 1:htab 5:vtab 35:print str$(d%)+"  "
1150 COLOR= 1: PLOT 2 * PM%,2 * PN%: PLOT 2 * PM% + 1,2 * PN%: PLOT 2 * PM%,2 * PN% + 1: PLOT 2 * PM% + 1,2 * PN% + 1
1160 IF D% = 0 THEN GoTo 1180
1170 GoTo 1080
1180 COLOR= 1: PLOT 2 * M%,2 *N%: PLOT 2 * M%+ 1,2 * N%: PLOT 2*M%,2* N%+1: PLOT 2 *M% +1,2*N%+1
1190 text:home
1200 HTAB 4: VTAB 10:  PRINT "Seu tempo foi de ";int(TI% / 5);" sec"
1210 IF F > TI% THEN LET F = TI%
1220 HTAB 4: VTAB 12: PRINT "Melhor tempo: ";int(F / 5);" sec"
1230 HTAB 7: VTAB 18: PRINT "Pressione RETURN para continuar"
1240 GET TS$
1242 IF ASC(TS$) <> 13 THEN GOTO 1240
1245 RESTORE
1250 GoTo 30
2000 DATA "AAAAAAAAAAAAAAAAAAA"
2010 DATA "A.......AAA.......A"
2020 DATA "A.AA.AA.....AA.AA.A"
2030 DATA "A.A.....AAA.....A.A"
2040 DATA "A...A.A.....A.A...A"
2050 DATA "A.AAA.AA.A.AA.AAA.A"
2060 DATA "A...A.........A...A"
2070 DATA "AAA...A.AAA.A...AAA"
2080 DATA "AAA...A.AAA.A...AAA"
2090 DATA "A...A.........A...A"
2100 DATA "A.AAA.AA.A.AA.AAA.A"
2110 DATA "A...A.A.....A.A...A"
2120 DATA "A.A.....AAA.....A.A"
2130 DATA "A.AA.AA.....AA.AA.A"
2140 DATA "A.......AAA.......A"
2150 DATA "AAAAAAAAAAAAAAAAAAA"

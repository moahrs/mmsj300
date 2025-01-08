10 text : home
20 PY%=8 : x%=10 : y%=10 : xv%=0 : yv%=0 :
25 gosub 9000
30 gosub 1000
40 gosub 2000
50 gosub 3000
55 gosub 4000
60 rem for l=0 to 200 : next l
70 goto 40
1000 gr : color=3
1100 hlin 0,39 at 0
1200 hlin 0,39 at 39
1300 vlin 0,39 at 39
1999 return
2000 get k$
2010 ky%=asc(k$)
2020 if k$="q" then text : home :end
2030 if k$="a" then PY%=PY%-1
2040 if k$="z" then PY%=PY%+1
2050 if k$="q" then text : home : end
2050 if KY%=32 then if (xv%=0 and yv%=0) then gosub 8000
2060 if PY%<1 then PY%=1
2070 if PY%>32 then PY%=32
2080 if (xv%=0 and yv%=0) then x%=2 : y%=PY%+3
2999 return
3000 color=1
3010 if PY%>1 then plot 1,PY%-1
3020 if PY%<32 then plot 1,PY%+7
3030 color=5
3040 vlin PY%,PY%+6 at 1
3050 color=1
3999 return
4000 color=1
4010 if x%>=0 then plot x%,y%
4020 if xv%<>0 then x%=x%+xv%
4025 if yv%<>0 then y%=y%+yv%
4026 if (xv%=0 and yv%=0) then gosub 7000
4030 color=15
4040 if x%>=0 then plot x%,y%
4050 if x%>37 then xv%=-1
4060 if (x%=2 and xv%<>0) then gosub 5000
4070 if x%<0 then gosub 6000
4080 if y%<2 then yv%=1
4090 if y%>37 then yv%=-1
4999 return
5000 if y%>=PY% and y%<=PY%+6 then xv%=1
5999 return
6000 xv%=0 : yv%=0
6999 return
7000 color=1
7010 plot x%,y%-1 : plot x%,y%+1
7999 return
8000 if y%<=20 then yv%=1
8010 if y%>20 then yv%=-1
8020 xv%=1
8999 return
9000 htab(17) : vtab(5): print chr$(162)+"PONG"+chr$(162)
9005 htab(4) : vtab(7) : print "Programmed By Alexander G. Tozzi"
9006 htab(4) : vtab(8) : print "Adapted to MMSJ-300 by Moacir Jr"
9007 htab(4) : vtab(10) : print "www.wumpustales.com"
9009 htab(15) : vtab(12) : print "A=UP"
9010 htab(15) : vtab(13) : print "Z=DOWN"
9015 htab(15) : vtab(14) : print "Q=QUIT"
9020 htab(15) : vtab(15) : print "SPACE=LAUNCH BALL"
9030 htab(10) : vtab(20) : print "PRESS ANY KEY TO START"
9035 get k$
9040 get k$
9042 ky%=asc(k$)
9045 if ky% = 0 then goto 9040
9050 normal : home : gr
9999 return

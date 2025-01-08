20 HOME
40 LET x%=15: LET y%=21
50 LET B$=" o#o "
60 LET a%=INT(RND(1)*36)+2
70 htab a%:vtab 2: PRINT "*"
80 LET p%=x%
90 htab x%:vtab y%:print b$
95 get k$
97 j=asc(k$)
100 IF j=97 THEN LET x%=x%-1
110 IF j=100 then LET x%=x%+1
120 IF x%<0 OR x%>38 THEN LET x%=p% 
140 IF j<>102 THEN GOTO 80
145 LET m%=y%-1
150 htab x%+2:vtab m%: print "|"
160 htab x%+2:vtab (m% + 1): print " "
170 LET m%=m%-1
180 IF m%=2 AND (x%+2)=a THEN GOTO 20
190 IF m%<>1 THEN GOTO 150 
195 htab (x%+2):vtab (m%+1):print " "
200 GOTO 80

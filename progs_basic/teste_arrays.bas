10 home
20 print "Tamanho: ";:input a
30 dim cu(a):dim nm$(a)
40 for j=1 to a
50 print int(j);"o. Nome: ";:input nm$(j)
60 print int(j);"o. Valor:  ";:input cu(j)
70 next j
80 for j=1 to a
90 print int(j);"o. Nome: ";nm$(j)
100 print int(j);"o. Valor:  ";cu(j)
110 next j

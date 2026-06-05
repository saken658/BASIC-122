   10 WRITE "Enter filename> ": INPUT file$
   20 OPEN file$ TO WRITE AS #4
   30 PRINT "Enter text": PRINT "----------"
   40 INPUT line$
   50 WHILE line$<>"XXX"
   60     PRINT #4,line$
   70     INPUT line$
   80 WEND
   90 CLOSE #4

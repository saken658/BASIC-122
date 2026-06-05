   10 ECHO OFF
   20 IF HASDATA(#1) THEN
   30     CINPUT a$
   40     PRINT "Read char: ",ASC(a$)
   50 ELSE
   60     PRINT "no data": PAUSE 100
   70 FI
   80 GOTO 20

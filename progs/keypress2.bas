   10 ECHO OFF
   20 REM *** Wait for 1 second then continue ***
   30 IF WAITDATA(#1,1000) THEN
   40     CINPUT a$
   50     PRINT "Read char: ",ASC(a$)
   60 ELSE
   70     PRINT "no data": PAUSE 100
   80 FI
   90 GOTO 30

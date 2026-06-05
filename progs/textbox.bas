   10 REM *** An example of a simple textbox ***
   20 REM
   30 box_length = 30
   40 box_x = 20
   50 box_y = 2
   60 REM
   70 REM *** Draw box ***
   80 REM
   90 CLS: PEN 7: PAPER 2
  100 LOCATE box_x,box_y: PRINT PAD$(" ",box_length)
  110 LOCATE box_x,box_y+1: PRINT " "
  120 LOCATE box_x,box_y+2: PRINT PAD$(" ",box_length)
  130 LOCATE box_x+box_length-1,box_y+1: PRINT " "
  140 LOCATE box_x+1,box_y+1
  150 REM
  160 REM *** Setup for typing ***
  170 REM
  180 PAPER 0
  190 ECHO OFF
  200 pos = box_x+1
  210 line$ = ""
  220 REM
  230 REM *** Main loop ***
  240 REM
  250 WHILE 1
  260      REM
  270      REM *** Wait here until a key is pressed ***
  280      REM
  290      IF NOT WAITDATA(#1) THEN GOTO 290 FI
  300      CINPUT a$
  305      CHOOSE ASC(a$)
  306           CASE 127
  310                REM
  320                REM *** Check for delete ***
  330                REM
  350                IF pos > box_x+1 THEN
  360                     pos = pos-1
  370                     LOCATE pos,box_y+1: WRITE " "
  380                     LOCATE pos,box_y+1
  390                     line$ = LEFT$(line$,LENGTH(line$)-1)
  400                FI
  405                GOTO 590
  410           CASE 10
  420                REM
  430                REM *** Check for newline pressed ***
  440                REM
  460                LOCATE box_x,10: PRINT "You entered: ",line$,PAD$(" ",box_length)
  470                line$ = ""
  480                pos = box_x+1
  490                LOCATE box_x+1,box_y+1: PRINT PAD$(" ",box_length-2)
  500                LOCATE box_x+1,box_y+1
  505                GOTO 590
  510           DEFAULT
  520                IF pos < box_x+box_length-1 THEN
  530                     LOCATE pos,box_y+1: WRITE a$
  540                     pos = pos+1
  550                     line$ = line$&a$
  560                FI
  570      CHOSEN
  590 WEND

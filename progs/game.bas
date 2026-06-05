   10 REM
   20 REM *** Demonstrates part of a game - just moving the ship and firing ***
   30 REM
   40 CLS
   50 ECHO OFF
   60 CURSOR OFF
   70 ship_x = screen_width/2
   80 ship_y = screen_height-5
   90 fire = 0
  100 REM
  110 REM *** Main loop ***
  120 REM
  130 WHILE 1
  140      GOSUB 390
  150      IF fire = 1 THEN GOSUB 470 FI
  160      IF HASDATA(#1) THEN
  170           CINPUT a$
  180           CHOOSE UPPER$(a$)
  190                CASE "Z"
  200                     IF ship_x > 2 THEN ship_x = ship_x-1 FI
  210                     GOTO 330
  220                CASE "X"
  230                     IF ship_x < screen_width-7 THEN ship_x = ship_x+1 FI
  240                     GOTO 330
  250                CASE CHR$(10)
  260                     IF fire = 0 THEN
  270                          fire = 1
  280                          bullet_x = ship_x+2
  290                          bullet_y = ship_y-3
  300                          prev_bullet_x = 0
  310                          prev_bullet_y = 0
  320                     FI
  330           CHOSEN
  340      FI
  350 WEND
  360 REM
  370 REM *** Draw the ship ***
  380 REM
  390 PEN 6
  400 LOCATE ship_x,ship_y-2: PRINT "  XX "
  410 LOCATE ship_x,ship_y-1: PRINT "  XX "
  420 LOCATE ship_x-1,ship_y: PRINT " XXXXXX "
  430 RETURN
  440 REM
  450 REM *** Draw the bullet ***
  460 REM
  470 PEN 3
  480 IF prev_bullet_x THEN
  490      LOCATE prev_bullet_x,prev_bullet_y: PRINT "  "
  500 FI
  510 IF bullet_y < 0 THEN
  520      fire = 0
  530      RETURN
  540 FI
  550 LOCATE bullet_x,bullet_y: PRINT "||"
  560 prev_bullet_x = bullet_x
  570 prev_bullet_y = bullet_y
  580 bullet_y = bullet_y-0.02
  590 RETURN

   10 CURSOR OFF
   20 SRAND TIME()
   30 IF screen_width=0 THEN
   40     PRINT "Unable to get screen size": EXIT 1
   50 FI
   60 x=ROUND(RAND()*screen_width)
   70 y=ROUND(RAND()*screen_height)
   80 xadd=1: yadd=1
   90 CLS
  100 WHILE 1
  110     PEN RAND()*num_pens
  120     PAPER RAND()*num_papers
  130     LOCATE x,y: PRINT "O"
  140     PAUSE 20
  150     PAPER 0
  160     LOCATE x,y: PRINT " "
  170     IF x=1 OR x=screen_width THEN
  180         xadd=-xadd
  190     FI
  200     IF y=1 OR y=screen_height-1 THEN
  210         yadd=-yadd
  220     FI
  230     x=x+xadd: y=y+yadd
  240 WEND

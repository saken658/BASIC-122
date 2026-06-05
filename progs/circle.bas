   10 CLS
   20 midx=screen_width/2
   30 midy=screen_height/2
   40 len=40
   50 col=1
   60 FOR a=0 TO 359
   70     x=midx+SIN(a)*len
   80     y=midY+COS(a)*len
   90     IF x>screen_width THEN x=screen_width FI
  100     IF y>screen_height-1 THEN y=screen_height-1 FI
  110     LOCATE x,y: PEN col: PRINT "O"
  120     col=(col+1)%num_pens
  130 NEXT

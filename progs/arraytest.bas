   10 DIM a(10,10,10,10)
   20 i=0
   30 FOR w=1 TO 10
   40     FOR x=1 TO 10
   50         FOR y=1 TO 10
   60             FOR z=1 TO 10
   70                 a(w,x,z,y)=i
   80                 i=i+1
   90             NEXT
  100         NEXT
  110     NEXT
  120 NEXT
  130 FOR w=1 TO 10
  140     FOR x=1 TO 10
  150         FOR y=1 TO 10
  160             FOR z=1 TO 10
  170                 PRINT "Array ",w,",",x,",",y,",",z," = ",a(w,x,y,z)
  180             NEXT
  190         NEXT
  200     NEXT
  210 NEXT

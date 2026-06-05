   10 WRITE "Enter numeric array size> ": INPUT size
   20 DIM a(size)
   30 FOR i = 1 TO size
   40      WRITE "Enter element ",i,"> ": INPUT e
   50      a(i) = e
   60 NEXT
   70 SORT a
   80 PRINT "*** SORTED NUMBERS ***"
   90 FOR i = 1 TO size
  100      PRINT a(i)
  110 NEXT
  120 WRITE "Enter string array size> ": INPUT size
  130 DIM a$(size)
  140 FOR i = 1 TO size
  150      WRITE "Enter element ",i,"> ": INPUT e$
  160      a$(i) = e$
  170 NEXT
  180 SORT a$
  190 PRINT "*** SORTED STRINGS ***"
  200 FOR i = 1 TO size
  210      PRINT a$(i)
  220 NEXT

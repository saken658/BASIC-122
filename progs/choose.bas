   10 WRITE "STRING>": INPUT a$
   20 CHOOSE a$
   30      CASE "abc": PRINT "ABC!"
   40      CASE "hello","wo"&"rld"
   50      WRITE "NUM>": INPUT a
   60      CHOOSE a
   70           CASE 1: PRINT "ONE": GOTO 100
   80           CASE 1+1: PRINT "TWO": GOTO 100
   90           DEFAULT: PRINT "NO MATCH"
  100      CHOSEN
  110      GOTO 140
  120      DEFAULT: PRINT "default"
  130      CASE "wibble": PRINT 3
  140 CHOSEN
  150 GOTO 10

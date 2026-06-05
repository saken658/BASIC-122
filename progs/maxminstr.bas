   10 DIM warr$(5)
   20 FOR f=1 TO 5
   30     WRITE "Enter word ",f,"> ": INPUT warr$(f)
   40 NEXT
   50 PRINT "Max is: ",MAX$(warr$(1),warr$(2),warr$(3),warr$(4),warr$(5))
   60 PRINT "Min is: ",MIN$(warr$(1),warr$(2),warr$(3),warr$(4),warr$(5))

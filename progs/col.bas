   10 OPEN "wibble" TO WRITE AS #4
   20 FOR f=1 TO 8
   30     FOR g=1 TO 8
   40         FOR h=1 TO 4
   50             PEN f: PAPER g: STYLE h
   60             PRINT "TEXT"
   70             PRINT #4,"XXXX"
   80         NEXT: NEXT: NEXT
   90 CLOSE #4

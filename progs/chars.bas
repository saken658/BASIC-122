   10 WHILE 1
   20     WRITE "String> ": INPUT a$
   30     FOR f=1 TO LENGTH(a$)
   40         PRINT MID$(a$,f,1)," = ",ASC(a$,f)
   50     NEXT
   60 WEND

   10 REM *** An example of how to use DATA ***
   20 DATA a+1,"one "&a$,a+3,"two "&a$
   30 DATA a+5,"three "&a$
   40 RESTORE 20
   50 WRITE "Value for A> ": INPUT a
   60 WRITE "Value for A$> ": INPUT a$
   70 FOR f=1 TO 3
   80     READ b,c$: PRINT b,",",c$
   90 NEXT

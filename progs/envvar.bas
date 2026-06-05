   10 WRITE "Enter variable name>"
   20 INPUT var$
   30 IF ENVEXISTS(var$) THEN
   40      a$ = GETENV$(var$)
   50      IF a$ = "" THEN PRINT "Variable exists but is unset"
   60      ELSE PRINT "Variable has value: ",a$ FI
   70 ELSE PRINT "Variable does not exist"
   80 FI

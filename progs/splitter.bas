   10 REM
   20 REM *** Demonstrates splitting a line on a given seperator ***
   30 REM
   40 WHILE 1
   50     REM
   60     REM *** Get input ***
   70     REM
   80     WRITE "String>": INPUT str$
   90     WRITE "Seperator>": INPUT sep$
  100     pos=SEARCH(str$,sep$)
  110     from=1
  120     len=pos-1
  130     REM
  140     REM *** LOOP ***
  150     REM
  160     WHILE pos>0
  170         IF len>0 THEN PRINT MID$(str$,from,len) FI
  180         from=pos+LENGTH(sep$)
  190         pos=SEARCH(str$,sep$,from)
  200         len=pos-from
  210     WEND
  220     REM
  230     REM *** Print remainder ***
  240     REM
  250     PRINT MID$(str$,from,LENGTH(str$))
  260 WEND

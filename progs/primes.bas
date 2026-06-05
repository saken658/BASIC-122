   10 REM *** This works out primes ***
   20 WRITE "Max value>": INPUT maxval
   30 num=3
   40 WHILE num<maxval
   50     REM Loop up to square root
   60     FOR d=3 TO SQRT(num) STEP 2
   70         IF num%d=0 THEN GOTO 100 FI
   80     NEXT
   90     PRINT num
  100     num=num+2
  110 WEND

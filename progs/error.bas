   10 ON ERROR GOSUB 90
   20 PRINT "Handler will be called"
   30 kjkkj
   40 PRINT "Back from handler"
   50 END
   60 REM
   70 REM *** Error handler subroutine ***
   80 REM
   90 PRINT "ERROR ",last_error," (",ERROR$(last_error),")"
  100 RETURN

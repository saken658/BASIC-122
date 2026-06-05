   10 WHILE 1
   20     WRITE "DIR> ": INPUT dir$
   30     OPENDIR dir$ AS #4
   40     INPUT #4,a$
   50     WHILE NOT EOF(#4)
   60         plen=15-LENGTH(a$)
   70         IF plen<0 THEN plen=0 FI
   80         PRINT a$,PAD$(" ",plen),": ",LSTAT$(dir$&"/"&a$)
   90         INPUT #4,a$
  100     WEND
  110     CLOSE #4
  120 WEND

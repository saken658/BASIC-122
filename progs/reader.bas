   10 WRITE "Enter filename> ": INPUT file$
   20 OPEN file$ TO READ AS #4
   30 INPUT #4,a$
   40 WHILE NOT EOF(#4)
   50     PRINT a$: INPUT #4,a$
   60 WEND
   70 CLOSE #4

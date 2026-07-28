###############################################################################
# FILE: Makefile
# LVU : 1.2.2
###############################################################################


OPTS=-g -Wall -pedantic
CC ?= cc
FILTER=2>&1 | egrep -v "not handled in switch" | cat

basic: \
	build_date.h \
	main.o \
	tokenizer.o \
	lines.o \
	com_inter.o \
	com_io.o \
	com_screen.o \
	com_flow.o \
	com_var.o \
	com_misc.o \
	expressions.o \
	variables.o \
	func_numeric.o \
	func_string.o \
	misc.o
	$(CC) -lm \
	   main.o \
	   tokenizer.o \
	   lines.o \
	   com_inter.o \
	   com_io.o \
	   com_screen.o \
	   com_flow.o \
	   com_var.o \
	   com_misc.o \
	   expressions.o \
	   variables.o func_numeric.o func_string.o misc.o -o basic 

build_date.h:
	echo "#define BUILD_DATE \"`date`\"" > build_date.h

main.o: main.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c main.c $(FILTER)

tokenizer.o: tokenizer.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c tokenizer.c $(FILTER)

lines.o: lines.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c lines.c $(FILTER)

com_inter.o: com_inter.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c com_inter.c $(FILTER)

com_io.o: com_io.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c com_io.c $(FILTER)

com_screen.o: com_screen.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c com_screen.c $(FILTER)

com_flow.o: com_flow.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c com_flow.c $(FILTER)

com_var.o: com_var.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c com_var.c $(FILTER)

com_misc.o: com_misc.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c com_misc.c $(FILTER)

expressions.o: expressions.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c expressions.c $(FILTER)

variables.o: variables.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c variables.c $(FILTER)

func_numeric.o: func_numeric.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c func_numeric.c $(FILTER)

func_string.o: func_string.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c func_string.c $(FILTER)

misc.o: misc.c basic.h version.h
	$(CC) $(OPTS) $(DEFS) -c misc.c $(FILTER)

clean:
	rm -f build_date.h *.o core* basic > /dev/null 2>&1

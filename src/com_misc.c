/****************************************************************************
 FILE: com_misc.c
 LVU : 1.2.0

 DESC:
 Miscellaniouse commands and psuedo commands that don't belong in any of the
 other com_ files.

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"



/*** REM, CASE, CHOSEN:
     This commands do nothing on their own. Just bypass. ***/
enum en_error comPseudo1(enum en_command com, struct st_line *line, int start)
{
return OK;
}




/*** TO,STEP,THEN,AS,APPEND,READWRITE,ERROR,CONTINUE,BREAK,OFF:
     These are dummy commands and should never be used as a real command ***/
enum en_error comPseudo2(enum en_command com, struct st_line *line, int start)
{
return ERR_SYNTAX;
}




/*** PAUSE: Pause for a given number of milliseconds ***/
enum en_error comPause(enum en_command com, struct st_line *line, int start)
{
struct timeval tv;
enum en_error err;
double delay;
int end;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;

/* Get delay */
if ((err = evalNumExpr(line,start+1,&end,&delay)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;
if (delay < 0) return ERR_OUT_OF_BOUNDS;

/* Use select() so we'll fall out on an interrupt */
tv.tv_sec = (int)(delay / 1000);
tv.tv_usec = ((int)delay % 1000) * 1000;

return (select(0,0,0,0,&tv) == -1) ? ERR_INTERRUPT : OK;
}




/*** BREAK: When used as a standalone command it switches ^C on/off ***/
enum en_error comBreak(enum en_command com, struct st_line *line, int start)
{
if (start != line->num_tokens - 2) return ERR_SYNTAX;

switch(line->tokens[start+1]->com) {
	case COM_ON:  break_on = 1;  break;
	case COM_OFF: break_on = 0;  break;

	default: return ERR_SYNTAX;
	}
return OK;
}




/*** SRAND: Set up the random number seed ***/
enum en_error comSrand(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double seed;
int end;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;
if ((err = evalNumExpr(line,start+1,&end,&seed)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;

srand((unsigned int)seed);
return OK;
}

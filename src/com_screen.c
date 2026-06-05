/****************************************************************************
 FILE: com_screen.c
 LVU : 1.0.0

 DESC:
 This contains the functions that do screen stuff (even though some can write
 to any stream). Eg: cls, locate

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"


/*** CLS: Clear the screen ***/
enum en_error comCls(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double stream;
int end,fd;

if (line->num_tokens > start + 1) {
	if (line->num_tokens < start + 3 ||
	    line->tokens[start+1]->type != TYPE_HASH) return ERR_SYNTAX;

	if ((err = getStream(line,start+2,&end,&stream)) != OK) 
		return err;

	if (end < line->num_tokens) return ERR_SYNTAX;

	if (!STREAM_IS_OPEN((int)stream)) return ERR_STREAM_NOT_OPEN;

	if (streams[(int)stream].type == STREAM_READ ||
	    streams[(int)stream].type == STREAM_DIR) 
		return ERR_INVALID_STREAM_TYPE; 

	fd = streams[(int)stream].fd;
	}
else fd = STDIN;

return fdWrite(fd,"\033[2J",4);
}




/*** PEN: Set the current pen colour ***/
enum en_error comPen(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double col;
int end;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;
if ((err = evalNumExpr(line,start+1,&end,&col)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;
if (col < 0 || col > NUM_FGCOLS) return ERR_OUT_OF_BOUNDS;

pen = col;

return OK;
}




/*** PAPER: Set the current paper colour ***/
enum en_error comPaper(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double col;
int end;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;
if ((err = evalNumExpr(line,start+1,&end,&col)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;
if (col < 0 || col > NUM_BGCOLS) return ERR_OUT_OF_BOUNDS;

paper = col;

return OK;
}




/*** STYLE: Set the text style ***/
enum en_error comStyle(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double sty;
int end;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;
if ((err = evalNumExpr(line,start+1,&end,&sty)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;
if (sty < 0 || sty > NUM_STYLES) return ERR_OUT_OF_BOUNDS;

style = sty;

return OK;
}




/*** LOCATE: Set the cursor to a particular point on the screen. Don't
     take a stream argument since anything other than the screen wouldn't
     make sense ***/
enum en_error comLocate(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double stream,x,y;
int end,fd;
char str[20];

if (line->num_tokens < start + 3) return ERR_SYNTAX;

if (line->tokens[start+1]->type == TYPE_HASH) {
	/* We have a stream so get it */
	if ((err = getStream(line,start+2,&end,&stream)) != OK) 
		return err;

	if (!STREAM_IS_OPEN((int)stream)) return ERR_STREAM_NOT_OPEN;

	if (streams[(int)stream].type == STREAM_READ ||
	    streams[(int)stream].type == STREAM_DIR) 
		return ERR_INVALID_STREAM_TYPE; 

	fd = streams[(int)stream].fd;

	if (end >= line->num_tokens ||
	    line->tokens[end]->type != TYPE_COMMA) return ERR_SYNTAX;
	start = end;
	}
else fd = STDOUT;

if ((err = evalNumExpr(line,start+1,&end,&x)) != OK) return err;

if (end >= line->num_tokens || line->tokens[end]->type != TYPE_COMMA) 
	return ERR_SYNTAX;

if ((err = evalNumExpr(line,end+1,&end,&y)) != OK) return err;

if (end < line->num_tokens) return ERR_SYNTAX;

/* Check x,y values are valid */
if (x < 1) x = 1;
if (y < 1) y = 1;

sprintf(str,"\033[%d;%dH",(int)y,(int)x);
return fdWrite(fd,str,strlen(str));
}




/*** SCROLL: Scroll the screen up or down the given number of lines ***/
enum en_error comScroll(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double stream,lcnt;
int end,fd;
char str[20];

if (line->num_tokens <= start + 1) return ERR_SYNTAX;

if (line->tokens[start+1]->type == TYPE_HASH) {
	/* We have a stream so get it */
	if ((err = getStream(line,start+2,&end,&stream)) != OK) 
		return err;

	if (!STREAM_IS_OPEN((int)stream)) return ERR_STREAM_NOT_OPEN;

	if (streams[(int)stream].type == STREAM_READ ||
	    streams[(int)stream].type == STREAM_DIR) 
		return ERR_INVALID_STREAM_TYPE; 

	fd = streams[(int)stream].fd;

	if (end >= line->num_tokens ||
	    line->tokens[end]->type != TYPE_COMMA) return ERR_SYNTAX;
	start = end;
	}
else fd = STDOUT;

if ((err = evalNumExpr(line,start+1,&end,&lcnt)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;

if (lcnt > 0)
	sprintf(str,"\033[%dS",(int)lcnt);
else
	sprintf(str,"\033[%dT",(int)-lcnt);

return fdWrite(fd,str,strlen(str));
}




/*** ECHO: Turns screen echoing on or off ***/
enum en_error comEcho(enum en_command com, struct st_line *line, int start)
{
if (start != line->num_tokens - 2) return ERR_SYNTAX;

switch(line->tokens[start+1]->com) {
	case COM_ON:  echo_on = 1;  break;
	case COM_OFF: echo_on = 0;  break;

	default: return ERR_SYNTAX;
	}
return OK;
}




/*** CURSOR: Turns the terminal cursor on or off ***/
enum en_error comCursor(enum en_command com, struct st_line *line, int start)
{
if (start != line->num_tokens - 2) return ERR_SYNTAX;

switch(line->tokens[start+1]->com) {
	case COM_ON:
	write(STDOUT,"\033[?25h",6);
	cursor_on = 1;
	break;

	case COM_OFF:
	write(STDOUT,"\033[?25l",6);
	cursor_on = 0;
	break;

	default: return ERR_SYNTAX;
	}
return OK;
}

/****************************************************************************
 FILE: lines.c
 LVU : 0.9.0

 DESC:
 This has functions that control handling of lines , whether main BASIC
 lines (ie with a line number) or sub lines within the main lines delimited
 by colons.

 eg: 10 PRINT "hello": GOTO 10
     |              |  |     |
      --------------    -----
            |             |
         sub line      sub line (st_line)
            |             |
             -------------
                   |
               BASIC line (st_bas_line)

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"

void addLineToBasicLine(
	struct st_bas_line *basline,struct st_line *line);


/*************************** PROG LINE FUNCTIONS ***************************/

/*** Add a BASIC line to the program. The lines are stored in a linked list ***/
void addProgramLine(struct st_bas_line *basline)
{
struct st_token *token;
struct st_bas_line *bl;

/* First token is an INT. This is checked in main() */
token = basline->first_line->tokens[0];

if ((basline->line_number = (uint32_t)token->value) < 1) {
	printError(ERR_SYNTAX,0);
	return;
	}

/* Set first line in the list */
if (!first_basline) {
	first_basline = last_basline = basline;
	return;
	}

/* Add onto start of list */
if (first_basline->line_number > basline->line_number) {
	first_basline->prev = basline;
	basline->next = first_basline;
	first_basline = basline;
	goto DONE;
	}

/* Add onto end of list */
if (last_basline->line_number < basline->line_number) {
	last_basline->next = basline;
	basline->prev = last_basline;
	last_basline = basline;
	goto DONE;
	}

/* Find the line numbers to insert us inbetween or if its the same number as 
   an already existing line */
for(bl=first_basline;bl;bl=bl->next) {
	/* Replace a line */
	if (bl->line_number == basline->line_number) {
		basline->prev = bl->prev;
		basline->next = bl->next;

		if (bl == first_basline) first_basline = basline;
		else
		if (bl->prev) bl->prev->next = basline;

		if (bl == last_basline) last_basline = basline;
		else
		if (bl->next) bl->next->prev = basline;

		deleteBasicLine(bl);
		goto DONE;
		}

	/* Insert between lines */
	if (bl->line_number < basline->line_number && 
	    bl->next->line_number > basline->line_number) {
		basline->prev = bl;
		basline->next = bl->next;
		bl->next->prev = basline;
		bl->next = basline;
		goto DONE;
		}
	}
/* Should never get here */
printError(ERR_INTERNAL,0);
return;

DONE:
/* Set the ->next pointers for sublines on the end of the basline */
if (basline->next) basline->last_line->next = basline->next->first_line;
if (basline->prev) basline->prev->last_line->next = basline->first_line;
}




/*** Delete a BASIC line ***/
void deleteProgramLine(struct st_bas_line *basline)
{
/* First reset end subline ->next pointers of previous line */
if (basline->prev) 
	basline->prev->last_line->next = 
		(basline->next ? basline->next->first_line : NULL);

/* Set basline linked list pointers */
if (basline == first_basline) first_basline = first_basline->next;
else
if (basline->prev) basline->prev->next = basline->next;

if (basline == last_basline) last_basline = last_basline->prev;
else
if (basline->next) basline->next->prev = basline->prev;

deleteBasicLine(basline);
}




/*************************** BASIC LINE FUNCTIONS ***************************/

/*** Create a BASIC line ***/
struct st_bas_line *createBasicLine(void)
{
struct st_bas_line *basline;

basline = (struct st_bas_line *)malloc(sizeof(struct st_bas_line));
basline->line_number = 0;
basline->first_line = NULL;
basline->prev = NULL;
basline->next = NULL;
return basline;
}




/*** Delete a BASIC line and all its tokens ***/
void deleteBasicLine(struct st_bas_line *basline)
{
struct st_line *line,*next;

for(line=basline->first_line;line && line->parent == basline;line=next) {
	next = line->next;
	deleteLine(line);
	}
free(basline);
}




/*** Add a sub line to a basic line ***/
void addLineToBasicLine(
	struct st_bas_line *basline,struct st_line *line)
{
if (basline->first_line) 
	basline->last_line->next = line;
else 
	basline->first_line = line;

basline->last_line = line;
}




/**************************** SUB LINE FUNCTIONS *****************************/

/*** Create a line structure and add it to parent basic line ***/
struct st_line *createLine(struct st_bas_line *parent)
{
struct st_line *line;

line = (struct st_line *)malloc(sizeof(struct st_line));
line->parent = parent;
line->num_tokens = 0;
line->num_allocated = 0;
line->tokens = NULL;
line->goto_line_number = 0;
line->goto_line = NULL;
line->fordata = NULL;
line->next = NULL;
line->paired = 0;
line->renum = 0;

addLineToBasicLine(parent,line);

return line;
}




/*** Delete a sub line and all its tokens ***/
void deleteLine(struct st_line *line)
{
int i;
for(i=0;i < line->num_tokens;++i) deleteToken(line->tokens[i]);
FREE(line->tokens);
free(line);
}




/*** Add a token to the line ***/
void addTokenToLine(struct st_line *line, struct st_token *token)
{
if (line->num_tokens >= line->num_allocated) {
	line->num_allocated += ALLOC_BLOCK;
	line->tokens = (struct st_token **)realloc(
		line->tokens,sizeof(struct st_token *) * line->num_allocated);
	}
line->tokens[line->num_tokens++] = token;
}




/*** Find the first line of a BASIC line by the line number ***/
struct st_line *getLine(uint32_t linenum)
{
struct st_bas_line *basline;

for(basline = first_basline;basline;basline = basline->next) 
	if (basline->line_number == linenum) return basline->first_line;
return NULL;
}




/*** Resets line data ***/
void resetLines(struct st_line *line)
{
for(;line;line=line->next) {
	line->goto_line_number = 0;
	line->goto_line = NULL;
	line->paired = 0;
	if (line->fordata) {
		free(line->fordata->varname);
		free(line->fordata);
		line->fordata = NULL;
		}
	}
goto_line = NULL;
data_line = NULL;
error_goto_line = NULL;
data_token_pos = 2;
}


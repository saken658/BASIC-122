/****************************************************************************
 FILE: com_var.c
 LVU : 1.2.0

 DESC:
 BASIC commands that are related to variables and are not in any of the
 other com_ files.

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"



/*** DIM: Dimension one or more arrays ***/
enum en_error comDim(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
int index[MAX_ARRAY_DEPTH];
int depth;
int end;

if (line->num_tokens < start + 2) return ERR_SYNTAX;

for(++start;start < line->num_tokens;start = end+1) {
	switch(line->tokens[start]->type) {
		case TYPE_ARR_VAR:
		case TYPE_ARR_STRVAR:
		break;

		default: return ERR_SYNTAX;
		}

	if ((err = getArrIndex(line,start+2,&end,index,&depth)) != OK)
		return err;

	if (++end < line->num_tokens && line->tokens[end]->type != TYPE_COMMA)
		return ERR_SYNTAX;

	if (getSystemVarNum(line->tokens[start]->text) != NOT_SYSVAR ||
	    getVariable(line->tokens[start]->text))
		return ERR_VAR_ALREADY_EXISTS;

	if ((err = createVariable(
		line->tokens[start]->text,
		line->tokens[start]->type,index,depth,NULL)) != OK) return err;
	}
return OK;
}




/*** LET: Set a variable value ***/
enum en_error comLet(enum en_command com, struct st_line *line, int start)
{
struct st_token *vartoken;
enum en_error err;
enum en_type result_type;
double result;
int index[MAX_ARRAY_DEPTH];
int depth,end;
char *strres;

start += (line->tokens[start]->type == TYPE_COM);
if (line->num_tokens < start + 3) return ERR_SYNTAX;

vartoken = line->tokens[start];

switch(vartoken->type) {
	case TYPE_VAR:
	case TYPE_STRVAR:
	if (line->tokens[start+1]->type != TYPE_EQUALS) return ERR_SYNTAX;
	end = start+2;
	depth = 0;
	break;

	case TYPE_ARR_VAR:
	case TYPE_ARR_STRVAR:
	if ((err = getArrIndex(line,start+2,&end,index,&depth)) != OK) return err;
	if (line->tokens[++end]->type != TYPE_EQUALS) return ERR_SYNTAX;
	++end;
	break;

	default: return ERR_SYNTAX;
	}
if (end >= line->num_tokens) return ERR_SYNTAX;

switch(vartoken->type) {
	case TYPE_VAR:
	case TYPE_ARR_VAR:
	if ((err = evalNumExpr(line,end,&end,&result)) != OK) return err;
	if (end != line->num_tokens) return ERR_SYNTAX;
	err = setVarValue(line->tokens[start]->text,index,depth,result,NULL);
	break;

	case TYPE_STRVAR:
	case TYPE_ARR_STRVAR:
	if ((err = evalStringExpr(
		line,end,&end,&result_type,&result,&strres)) != OK) return err;
	if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;
	err = setVarValue(line->tokens[start]->text,index,depth,0,strres);
	FREE(strres);
	}
return (end+1 >= line->num_tokens) ? err : ERR_SYNTAX;
}




/*** DATA: Do nothing, just stores data ***/
enum en_error comData(enum en_command com, struct st_line *line, int start)
{
if (line->num_tokens <= start + 1) return ERR_SYNTAX;
return OK;
}




/*** READ: Read from current position on DATA line into a variable.
     Currently only one variable is supported for each READ command since
     the code is messy enough already ***/
enum en_error comRead(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
enum en_type result_type;
double result;
char *strres;
int st,end;
int index[MAX_ARRAY_DEPTH];
int depth,data_end;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;

/* See if current position is a DATA line. If not find next one. */
if (!data_line) return ERR_NOT_DATA_LINE;

if (data_line->tokens[IS_START_LINE(data_line)]->com != COM_DATA)
	return ERR_NOT_DATA_LINE;

/* Go through all variables on line */
for(++start;start < line->num_tokens;start = end+1) {
	/* Get array index */
	if (line->tokens[start]->type == TYPE_ARR_VAR ||
	    line->tokens[start]->type == TYPE_ARR_STRVAR) {
		if ((err = getArrIndex(line,start+1,&end,index,&depth)) != OK)
			return err;
		}
	else {
		depth = 0;
		end = start + 1;
		}

	/* Get next data token. If we're at end of line move to next DATA line 
	   which can be anywhere following. This is nominally to allow for REM
	   statements inbetween blocks of DATA lines */
	if (data_token_pos >= data_line->num_tokens) {
		for(data_line=data_line->next;
		    data_line;data_line=data_line->next) {
			st = IS_START_LINE(data_line);
			if (data_line->num_tokens > st + 1 && 
			    data_line->tokens[st]->com == COM_DATA) break;
			}
		if (!data_line || data_line->num_tokens < st + 1)
			return ERR_END_OF_DATA;
		data_token_pos = st + 1;
		}

	switch(line->tokens[start]->type) {
		case TYPE_VAR:
		case TYPE_ARR_VAR:
		if ((err = evalNumExpr(
			data_line,
			data_token_pos,
			&data_end,&result)) != OK) return err;

		err = setVarValue(line->tokens[start]->text,index,depth,result,NULL);
		break;

		case TYPE_STRVAR:
		case TYPE_ARR_STRVAR:
		if ((err = evalStringExpr(
			data_line,
			data_token_pos,
			&data_end,
			&result_type,&result,&strres)) != OK) return err;

		if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

		err = setVarValue(line->tokens[start]->text,index,depth,0,strres);
		FREE(strres);
		break;

		default: return ERR_SYNTAX;
		}
	if (err != OK) return err;

	data_token_pos = data_end;

	/* Make sure command is following on READ line and its not the last
	   thing on the line */
	if (end < line->num_tokens && line->tokens[end]->type != TYPE_COMMA)
		return ERR_SYNTAX;

	/* Ditto above for DATA line */
	if (data_token_pos < data_line->num_tokens) {
		if (data_line->tokens[data_token_pos]->type != TYPE_COMMA)
			return ERR_SYNTAX;
		data_token_pos++;
		}
	}
return OK;
}




/*** RESTORE: Reset READ position to given DATA line ***/
enum en_error comRestore(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double linenum;
int end;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;

/* Get line number */
if ((err = evalNumExpr(line,start+1,&end,&linenum)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;
if (linenum < 1) return ERR_INVALID_LINE_NUMBER;

/* See if line exists */
if (!(data_line = getLine((uint32_t)linenum)))
	return ERR_NO_SUCH_LINE_NUMBER;
data_token_pos = 2; 

/* Make sure its a DATA line */
return (data_line->tokens[1]->com == COM_DATA) ? OK :  ERR_NOT_DATA_LINE;
}




/*** SORT: Sort a one dimensional array into ascending order ***/
enum en_error comSort(enum en_command com, struct st_line *line, int start)
{
if (line->num_tokens != start + 2) return ERR_SYNTAX;
return sortArray(line->tokens[start+1]->text);
}

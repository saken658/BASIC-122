/****************************************************************************
 FILE: com_flow.c
 LVU : 1.2.0

 DESC:
 This contains the BASIC flow of control commands. Eg: FOR/NEXT, IF/THEN,
 GOTO etc.

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"



/*** END: Stop the program ***/
enum en_error comEnd(enum en_command com, struct st_line *line, int start)
{
if (line->num_tokens > start + 1) return ERR_SYNTAX;
stop_program = BREAK_END;
return OK;
}




/*** EXIT: Make the interpreter quit ***/
enum en_error comExit(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double code;
int end;

if (line->num_tokens > start + 1) {
	if ((err = evalNumExpr(line,start+1,&end,&code)) != OK) return err;
	if (end < line->num_tokens) return ERR_SYNTAX;
	}
else code = 0;

printf("*** EXITING with code %d ***\n",(int)code);
doExit((int)code);
return OK; /* Keeps compiler happy */
}




/*** GOTO & GOSUB: Jump to a specified line. For gosub store the return
     position for the RETURN command ***/
enum en_error comGotoGosub(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double linenum;
int end;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;

if (com == COM_GOSUB && gosub_stack_pos >= GOSUB_STACK_SIZE) 
	return ERR_RECURSION_LIMIT_REACHED;

/* Get line number to go to */
if ((err = evalNumExpr(line,start+1,&end,&linenum)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;

/* See if its already been found in another pass */
if (line->goto_line && line->goto_line_number == linenum) {
	goto_line = line->goto_line;
	goto_line_set = 1;
	if (com == COM_GOSUB) goto SET_RETURN;
	return OK;
	}

/* Find it */
if ((goto_line = getLine(linenum))) {
	line->goto_line = goto_line;
	line->goto_line_number = (int)linenum;
	goto_line_set = 1;
	if (com == COM_GOSUB) goto SET_RETURN;
	return OK;
	}
return ERR_NO_SUCH_LINE_NUMBER;

SET_RETURN:
/* The return line must be the NEXT sub line after the current one */
gosub_stack[gosub_stack_pos++] = line->next;
return OK;
}




/*** RETURN: Return from a gosub ***/
enum en_error comReturn(enum en_command com, struct st_line *line, int start)
{
if (line->num_tokens > start+1) return ERR_SYNTAX;

if (!gosub_stack_pos) return ERR_UNEXPECTED_RETURN;

goto_line = gosub_stack[--gosub_stack_pos];
goto_line_set = 1;
return OK;
}




/*** WHILE: Loop while a condition is true ***/
enum en_error comWhile(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
int end;
double result;

if (line->num_tokens < start + 2) return ERR_SYNTAX;

/* Find WEND */
if (!line->goto_line &&
    !setLoopEnd(line,COM_WHILE,COM_WEND)) return ERR_MISSING_WEND;

/* Evaluate condition */
if ((err = evalNumExpr(line,start+1,&end,&result)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;

if (!result) {
	goto_line = line->goto_line;
	goto_line_set = 1;
	}
return OK;
}




/*** WEND: End of a WHILE loop ***/
enum en_error comWend(enum en_command com, struct st_line *line, int start)
{
if (line->num_tokens > start+1) return ERR_SYNTAX;
if (!line->paired) return ERR_UNEXPECTED_WEND;
goto_line = line->goto_line;
goto_line_set = 1;
return OK;
}




/*** DO: Start of a DO - UNTIL loop ***/
enum en_error comDo(enum en_command com, struct st_line *line, int start)
{
if (line->num_tokens > start+1) return ERR_SYNTAX;
return (line->goto_line || setLoopEnd(line,COM_DO,COM_UNTIL)
	? OK : ERR_MISSING_UNTIL);
}




/*** UNTIL: Loop back to DO if condition NOT true ***/
enum en_error comUntil(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double result;
int end;

if (line->num_tokens <  start + 2) return ERR_SYNTAX;
if (!line->paired) return ERR_UNEXPECTED_UNTIL;

/* Evaluate condition */
if ((err = evalNumExpr(line,start+1,&end,&result)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;

if (!result) {
	goto_line = line->goto_line;
	goto_line_set = 1;
	}
return OK;
}




/*** FOR: Loop format: FOR <var> = <start> TO <end> [STEP <step>] ***/
enum en_error comFor(enum en_command com, struct st_line *line, int start)
{
struct st_fordata *fd;
enum en_error err;
char *varname;
double start_val;
double end_val;
double step_val;
double result;
int index[MAX_ARRAY_DEPTH];
int depth;
int end;

if (line->num_tokens < start + 6) return ERR_SYNTAX;

/* Find NEXT */
if (!line->goto_line &&
    !setLoopEnd(line,COM_FOR,COM_NEXT)) return ERR_MISSING_NEXT;

/* If already set up and its not finished AND we jumped here from a NEXT
   (anything else and we reset) just check var value & modify it */
if ((fd = line->fordata) && !fd->finished && next_jump) {
	/* Get current var value */
	if ((err = getVarValue(
		fd->varname,fd->varindex,fd->depth,&result,NULL)) != OK)
		return err;

	/* Alter variable value */
	result += fd->step_val;
	err = setVarValue(fd->varname,fd->varindex,fd->depth,result,NULL);

	/* See if we've reached or passed our target */
	if ((fd->start_val <= fd->end_val && result > fd->end_val) ||
	    (fd->start_val > fd->end_val && result < fd->end_val)) {
		line->fordata->finished = 1;
		goto_line = line->goto_line;
		goto_line_set = 1;
		}
	next_jump = 0;
	return err;
	}

/* Fordata not set up (either pointer not set or finished = 1) so do it */
start++;
varname = line->tokens[start]->text;

/* Get variable */
switch(line->tokens[start]->type) {
	case TYPE_VAR:
	depth = 0;
	end = start;
	break;
	
	case TYPE_ARR_VAR:
	if ((err = getArrIndex(line,start+2,&end,index,&depth)) != OK)
		return err;
	break;

	default: return ERR_SYNTAX;
	}
if (++end >= line->num_tokens || 
    line->tokens[end]->type != TYPE_EQUALS) return ERR_SYNTAX;

/* Get start value */
if ((err = evalNumExpr(line,++end,&end,&start_val)) != OK) return err;
if (end >= line->num_tokens - 1 || line->tokens[end]->com != COM_TO) 
	return ERR_SYNTAX;

/* Get end value */
if ((err = evalNumExpr(line,++end,&end,&end_val)) != OK) return err;

/* Get optional stepping */
if (end < line->num_tokens) {
	if (end >= line->num_tokens - 1 ||
	    line->tokens[end]->com != COM_STEP) return ERR_SYNTAX;
	if ((err = evalNumExpr(line,++end,&end,&step_val)) != OK)
		return err;
	if (end < line->num_tokens) return ERR_SYNTAX;
	}
else step_val = 1;

/* Always set variable to the start value even if we don't execute the loop */
if ((err = setVarValue(varname,index,depth,start_val,NULL)) != OK)
	return err;

/* If the step value does not match the direction implied by the start and
   end values then don't execute the loop. This is standard behaviour in
   most languages. */
if ((end_val > start_val && step_val < 0) ||
    (end_val < start_val && step_val > 0)) {
	goto_line = line->goto_line;
	goto_line_set = 1;
	return OK;
	}

/* Create structure if it doesn't yet exist */
if (!line->fordata) {
	line->fordata = (struct st_fordata *)malloc(sizeof(struct st_fordata));
	line->fordata->varname = strdup(varname);
	memcpy(line->fordata->varindex,index,MAX_ARRAY_DEPTH * sizeof(int));
	line->fordata->depth = depth;
	}
line->fordata->start_val = start_val;
line->fordata->end_val = end_val;
line->fordata->step_val = step_val;
line->fordata->finished = 0;

/* Set initial value */
return setVarValue(varname,index,depth,start_val,NULL);
}




/*** NEXT: End of FOR loop ***/
enum en_error comNext(enum en_command com, struct st_line *line, int start)
{
if (line->num_tokens > start+1) return ERR_SYNTAX;
if (!line->paired) return ERR_UNEXPECTED_NEXT;
goto_line = line->goto_line;
goto_line_set = 1;
next_jump = 1;
return OK;
}




/*** IF: Evaluate a conditional but also set up goto lines and command pairing
     flags at ELSE and FI lines. ***/
enum en_error comIf(enum en_command com, struct st_line *line, int start)
{
struct st_line *line2,*line3;
enum en_error err;
double result;
int nesting,end;

/* If goto line not set then check everything */
if (!line->goto_line) {
	/* Find ELSE and/or FI and set goto_line. This is where we
	   jump if condition is false */
	for(line2=line->next,nesting=0;line2;line2=line2->next) {
		/* Use nesting since we may have nested ifs */
		switch(line2->tokens[IS_START_LINE(line2)]->com) {
			case COM_IF:
			nesting++;  break;

			case COM_FI:
			if (!nesting--) {
				line->goto_line = line2->next;
				line2->paired = 1;
				goto DONE;
				}
			break;

			case COM_ELSE:
			if (nesting) continue;

			/* Set goto_line to the line after ELSE */
			line->goto_line = line2->next;
			line2->goto_line = NULL;
			line2->paired = 1;

			/* Got ELSE. Now find FI to set ELSEs goto_line */
			for(line3=line2->next;line3;line3=line3->next) {
				switch(line3->tokens[IS_START_LINE(line3)]->com) {
					case COM_IF:
					nesting++;  break;

					case COM_FI:
					if (!nesting--) {
						line2->goto_line = line3->next;
						line3->paired = 1;
						goto DONE;
						}	
					default: break;
					}
				}
			if (!line3) return ERR_MISSING_ELSE_FI;
			goto DONE;
			}
			default: break;
		}
	DONE:
	if (!line2) return ERR_MISSING_ELSE_FI;
	}

/* Evaluate condition */
if ((err = evalNumExpr(line,start+1,&end,&result)) != OK) return err;
if (end >= line->num_tokens || line->tokens[end]->com != COM_THEN)
	return ERR_SYNTAX;

/* Jump to next line after ELSE/FI if zero result */
if (!result) {
	goto_line = line->goto_line;
	goto_line_set = 1;
	}
return OK;
}




/*** ELSE: Have arrived here following IF so skip next section and jump
     to where FI is ***/
enum en_error comElse(enum en_command com, struct st_line *line, int start)
{
/* If line hasn't been paired with an IF then goto_line will be NULL
   and we'd jump out of the program on return to runProgram() */
if (!line->paired) return ERR_UNEXPECTED_ELSE;
goto_line = line->goto_line;
goto_line_set = 1;
return OK;
}




/*** FI: Just reset if_jump ***/
enum en_error comFi(enum en_command com, struct st_line *line, int start)
{
return (line->paired ? OK : ERR_UNEXPECTED_FI);
}




/*** ON: Set up ON ERROR GOTO/GOSUB error handling ***/
enum en_error comOn(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double linenum;
int end;

if (line->num_tokens <= start + 2 ||
    line->tokens[start+1]->com != COM_ERROR) return ERR_SYNTAX;

switch(line->tokens[start+2]->com) {
	case COM_GOTO:
	case COM_GOSUB:
	break;

	case COM_BREAK:
	case COM_CONTINUE:
	error_action = line->tokens[start+2]->com;
	error_goto_line = NULL;
	return OK;

	default: return ERR_SYNTAX;
	}
if (line->num_tokens <= start + 3) return ERR_SYNTAX;

/* Get line number to go to */
if ((err = evalNumExpr(line,start+3,&end,&linenum)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;

/* See if its already been found in another pass */
if (line->goto_line && line->goto_line_number == linenum) 
	error_goto_line = line->goto_line;
else 
if ((line->goto_line = getLine(linenum)))  
	error_goto_line = line->goto_line;
else
return ERR_NO_SUCH_LINE_NUMBER;

error_action = line->tokens[start+2]->com;

return OK;
}




/*** CHOOSE: Has exactly the same logic as C switch() statement ***/
enum en_error comChoose(enum en_command com, struct st_line *line, int start)
{
struct st_line *line2,*default_line;
enum en_error err;
enum en_type result_type;
enum en_type case_result_type;
double result;
double case_result;
char *strres;
char *case_strres;
int end;
int nesting;

if (line->num_tokens < start + 2) return ERR_SYNTAX;

strres = NULL;
case_strres = NULL;

/* Get value to switch on. Try numeric argument first and if that fails
   try for a string */
if ((err = evalNumExpr(line,start+1,&end,&result)) != OK) {
	if ((err = evalStringExpr(
		line,start+1,&end,&result_type,&result,&strres)) != OK)
		return err;
	}
else result_type = TYPE_FLOAT;

if (end < line->num_tokens) {
	FREE(strres);  return ERR_SYNTAX;
	}

/* Find CASE (or CHOSEN) bearing in mind we could have nested CHOOSEes */
default_line = NULL;
nesting = 0;

for(line2=line->next;line2;line2=line2->next) {
	start = IS_START_LINE(line2);

	switch(line2->tokens[start]->com) {
		case COM_CHOOSE:
		++nesting;  continue;

		case COM_CHOSEN:
		if (nesting--) continue;
		/* No matching CASE. If we found a default line then jump to
		   that else program jumps to following line */
		if (!default_line) goto END;

		goto_line = default_line;
		goto_line_set = 1;
		err = OK;
		goto ERROR;

		case COM_DEFAULT:
		if (!nesting) default_line = line2;
		if (line2->num_tokens > start + 1) {
			err = ERR_SYNTAX;  goto ERROR;
			}
		continue;

		case COM_CASE:
		if (nesting) continue;

		/* Check value(s) in case. Try numeric first */
		case_result_type = TYPE_FLOAT;
		for(start=start+1;;start=end+1) {
			if ((err = evalNumExpr(
				line2,start,&end,&case_result)) != OK) {
				if ((err = evalStringExpr(
					line2,start,&end,
					&case_result_type,
					&case_result,
					&case_strres)) != OK) goto ERROR;
				}

			/* See if result is the same */
			if (case_result_type != result_type)
				return ERR_INVALID_CASE_TYPE;

			if (result_type == TYPE_FLOAT &&
			    result == case_result) goto END;

			if (result_type == TYPE_STRING &&
			    !strcmp(strres,case_strres)) goto END;

			/* Its not so move on */
			if (end >= line2->num_tokens) break;

			if (line2->tokens[end]->type != TYPE_COMMA) {
				err = ERR_SYNTAX;  goto ERROR;
				}
			}
		default: break;
		}	}
if (line2) {
	END:
	goto_line = line2->next;
	goto_line_set = 1;
	err = OK;
	}
else err = ERR_MISSING_CHOSEN;

ERROR:
FREE(strres);
FREE(case_strres);
return err;
}

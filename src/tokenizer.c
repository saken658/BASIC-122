/****************************************************************************
 FILE: tokenizer.c
 LVU : 0.9.0

 DESC:
 This splits up the incoming text lines into tokens the runtime part of
 the BASIC interpreter can use. These tokens are stored as a list array
 of st_token objects in the st_line object of each sub line.

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"

/*** Forward declarations ***/
enum en_error getTokenString(char **ptr, char **end, enum en_type *type);
enum en_type getTokenChar(char c);
struct st_token *createToken(char *st, char *end, enum en_type type);
void deleteToken(struct st_token *token);

/*** Check for invalid end of line tokens ***/
#define INVALID_END_TOKEN_CHECK(T) \
	switch(T->type) { \
		case TYPE_LBRACKET: \
		return ERR_MISSING_BRACKET; \
 \
		case TYPE_NUMFUNC: \
		case TYPE_STRFUNC: \
		case TYPE_COMMA: \
		case TYPE_HASH: \
		case TYPE_AMPERSAND: \
		case TYPE_BAR: \
		case TYPE_EMARK: \
		case TYPE_EQUALS: \
		case TYPE_NOT_EQUALS: \
		case TYPE_GRT_EQUALS: \
		case TYPE_LESS_EQUALS: \
		case TYPE_GRT: \
		case TYPE_LESS: \
		case TYPE_ADD: \
		case TYPE_SUB: \
		case TYPE_MUL: \
		case TYPE_DIV: \
		case TYPE_MOD: \
		case TYPE_PWR: \
		case TYPE_SHIFT_LEFT: \
		case TYPE_SHIFT_RIGHT: \
		return ERR_SYNTAX; \
		} 


/****************************** INITIAL PARSE ******************************/

/*** Parse a single text line. Each line may consist of a number of 
     sub lines if they're delimited by semicolons. 
     Eg: 10 for f=1 to 10; print f; next
     The above is 1 st_bas_line containing 3 st_line ***/
enum en_error parseTextLine(char *str, struct st_bas_line **basline)
{
enum en_type type,prev_prev_type;
enum en_error err;
struct st_token *token, *prev_token;
struct st_line *line;
char create_new_subline;
char in_rem;
char *s,*end;
int cnt;

token = NULL;
prev_token = NULL;
prev_prev_type = TYPE_NOTSET;

*basline = createBasicLine();
line = createLine(*basline);

for(s=str,in_rem=0,create_new_subline=0,cnt=0;;) {
	if ((err = getTokenString(&s,&end,&type)) != OK) return err;

	/* End of text line */
	if (end == s) break;

	/* See if we've hit end of the line. Don't create new sub line here
	   since if colon is at end of line we'll end up with an empty subline
	   eg "10 print a;" as we don't add the colon to the token list */
	if (!in_rem && type == TYPE_COLON) {
		/* Can only end a line on certain types */
		if (prev_token) {
			INVALID_END_TOKEN_CHECK(prev_token);
			}
		create_new_subline = 1;
		token = NULL;
		goto DONE;
		}

	/* Previously hit semi so create new line object */
	if (create_new_subline) {
		line = createLine(*basline);
		create_new_subline = 0;
		cnt = 0;
		}

	/* Create new token. If its a REM statement previously then token is
	   whole of the remainder of the line */
	if (in_rem) {
		end = s + strlen(s);
		type = TYPE_REM_COMMENTS;
		}
	token = createToken(s,end,type);

	/* If we've got THEN then make it the last thing on the line. If we've
	   got ELSE or FI put it on a line on its own */
	switch(token->com) {
		case COM_ELSE:
		case COM_FI:
		/* Check stuff has gone before the ELSE or FI and its not
		   just a line number. If true then put on own line. */
		if (cnt > 1 || (cnt == 1 && prev_token->type != TYPE_INT))
			line = createLine(*basline); 
		/* Fall through */

		case COM_THEN:
		addTokenToLine(line,token);
		create_new_subline = 1;
		token = NULL;
		cnt = 0;
		goto DONE;

		case COM_REM:
		in_rem = 1;
		}

	if (prev_token) {
		switch(prev_token->type) {
			case TYPE_VAR:
			if (type == TYPE_LBRACKET)
				prev_token->type = TYPE_ARR_VAR;
			break;

			case TYPE_STRVAR:
			if (type == TYPE_LBRACKET)
				prev_token->type = TYPE_ARR_STRVAR;
			break;

			case TYPE_SUB:
			/* Deal with minuses which can either be an operator in
			   their own right or can be a negation for a number,
			   function or variable */
			switch(prev_prev_type) {
				case TYPE_NOTSET:
				case TYPE_COM:
				case TYPE_HASH:
				case TYPE_AMPERSAND:
				case TYPE_BAR:
				case TYPE_EMARK:
				case TYPE_LBRACKET:
				case TYPE_EQUALS:
				case TYPE_NOT_EQUALS:
				case TYPE_GRT_EQUALS:
				case TYPE_LESS_EQUALS:
				case TYPE_GRT:
				case TYPE_LESS:
				case TYPE_COLON:
				case TYPE_COMMA:
				case TYPE_ADD:
				case TYPE_SUB:
				case TYPE_MUL:
				case TYPE_DIV:
				case TYPE_MOD:
				case TYPE_PWR:
				case TYPE_SHIFT_LEFT:
				case TYPE_SHIFT_RIGHT:
				/* Mark current token as negated and remove
				   previous minus token from list by swapping it
				   with this one */
				token->negated = 1;
				deleteToken(prev_token);
				line->tokens[line->num_tokens-1] = token;
				goto DONE;
				}
			break;

			case TYPE_NUMFUNC:
			case TYPE_STRFUNC:
			/* Check function is always followed by '(' */
			if (type != TYPE_LBRACKET) return ERR_SYNTAX;
			}
		addTokenToLine(line,token);
		}
	else addTokenToLine(line,token);

	DONE:
	if (prev_token) prev_prev_type = prev_token->type;
	prev_token = token;
	s = end;
	++cnt;
	}

/* Do some final end of line type checks */
if (token) {
	INVALID_END_TOKEN_CHECK(token);
	}
return OK;
}



/**************************** TOKEN FUNCTIONS ***************************/

/*** Get the next token from the input string ***/
enum en_error getTokenString(char **ptr, char **end, enum en_type *type)
{
char *p;

/* Skip whitespace */
for(p=*ptr;*p < 33;++p) {
	if (!*p) {
		*end = *ptr = p;
		return OK;
		}
	}
*ptr = p;

/* See if its a single or double character operator */
switch((*type = getTokenChar(*p))) {
	case TYPE_NOTSET:
	break;

	case TYPE_GRT:
	switch(*(p+1)) {
		case '=':
		*type = TYPE_GRT_EQUALS;
		break;

		case '>':
		*type = TYPE_SHIFT_RIGHT;
		break;

		default:
		*end = p+1;
		return OK;
		}
	*end = p+2;
	return OK;

	case TYPE_LESS:
	switch(*(p+1)) {
		case '=':
		*type = TYPE_LESS_EQUALS;
		break;

		case '>':
		*type = TYPE_NOT_EQUALS;
		break;

		case '<':
		*type = TYPE_SHIFT_LEFT;
		break;

		default:
		*end = p+1;
		return OK;
		}
	*end = p+2;
	return OK;

	default: 
	*end = p+1;
	return OK;
	}

/* See if its a string */
if (*p == '"') {
	*type = TYPE_STRING;
	for(p=p+1;*p;++p) {
		if (*p == '"') {
			*end = p+1;
			return OK;
			}
		}
	return ERR_MISSING_QUOTES;
	}

/* Its either var, com or number */
*type = TYPE_NOTSET;
for(p=p+1;*p > 32;++p) {
	/* Stop if hit another op */
	if (getTokenChar(*p) != TYPE_NOTSET) {
		*end = p;
		return OK;
		}
	}
*end = p;
return OK;
}




/*** Get the token type for a character ***/
enum en_type getTokenChar(char c)
{
switch(c) {
	case '&': return TYPE_AMPERSAND;
	case '|': return TYPE_BAR;
	case '!': return TYPE_EMARK;
	case '(': return TYPE_LBRACKET;
	case ')': return TYPE_RBRACKET;
	case ',': return TYPE_COMMA;
	case '+': return TYPE_ADD;
	case '-': return TYPE_SUB;
	case '*': return TYPE_MUL;
	case '/': return TYPE_DIV;
	case '%': return TYPE_MOD;
	case '^': return TYPE_PWR;
	case '=': return TYPE_EQUALS;
	case '>': return TYPE_GRT;
	case '<': return TYPE_LESS;
	case ':': return TYPE_COLON;
	case '#': return TYPE_HASH;
	case '?': return TYPE_QUESTION;
	}
return TYPE_NOTSET;
}




/*** Create a token structure object and assign the type if type is still
     unknown (TYPE_NOTSET) ***/
struct st_token *createToken(char *st, char *end, enum en_type type)
{
struct st_token *token;
char *s;
int i,dot;

/* Create the token */
token = (struct st_token *)malloc(sizeof(struct st_token));
token->func = FUNC_NOTSET;
token->value = 0;
token->negated = 0;

/* Check for '?' which is an alias for the PRINT command */
if (type == TYPE_QUESTION) {
	token->type = TYPE_COM;
	token->com = COM_PRINT;
	token->text = strdup("PRINT");
	token->text_len = 5;
	return token;
	}
token->type = type;
token->com = COM_NOTSET;

/* Set the text */
if (type == TYPE_STRING) {
	st++;
	token->text_len = (int)(end - st - 1);
	}
else
	token->text_len = (int)(end - st);

token->text = (char *)malloc(token->text_len+1);
memcpy(token->text,st,token->text_len);
token->text[token->text_len] = '\0';

if (type != TYPE_NOTSET) return token;

/* Check for a command */
for(i=0;i < NUM_COMMANDS;++i) 
	if (!strcasecmp(command[i],token->text)) break;
if (i < NUM_COMMANDS) {
	/* Set as command and convert to upper case */
	token->type = TYPE_COM;
	token->com = i;
	goto FOUND;
	}

/* See if its a numeric function type */
for(i=0;i < NUM_NUM_FUNCTIONS;++i) 
	if (!strcasecmp(numfunc[i],token->text)) break;

if (i < NUM_NUM_FUNCTIONS) {
	token->type = TYPE_NUMFUNC;
	token->func = i;
	goto FOUND;
	}

/* See if its a string function type */
for(i=0;i < NUM_STR_FUNCTIONS;++i) 
	if (!strcasecmp(strfunc[i],token->text)) break;

if (i < NUM_STR_FUNCTIONS) {
	token->type = TYPE_STRFUNC;
	token->func = FUNC_CHR + i;
	goto FOUND;
	}

/* See if its a word operator. ie AND, OR, XOR */
if (!strcasecmp(token->text,"AND")) {
	token->type = TYPE_AND;  goto FOUND;
	}
else
if (!strcasecmp(token->text,"OR")) {
	token->type = TYPE_OR;  goto FOUND;
	}
else
if (!strcasecmp(token->text,"XOR")) {
	token->type = TYPE_XOR;  goto FOUND;
	}
else
if (!strcasecmp(token->text,"NOT")) {
	token->type = TYPE_NOT;  goto FOUND;
	}


/* See if its a number. If not then its a variable. */
for(s=token->text,dot=0;*s;++s) {
	if (!isdigit(*s)) {
		if (dot || *s != '.') break;
		dot = 1;
		}
	}
if (!*s) {
	token->type = dot ? TYPE_FLOAT : TYPE_INT;
	token->value = atof(token->text);
	}
else
	token->type = (token->text[token->text_len-1] == '$') 
		? TYPE_STRVAR : TYPE_VAR;

return token;

FOUND:
toupperString(token->text);
return token;
}




/*** Delete a token ***/
void deleteToken(struct st_token *token)
{
if (token) {
	if (token->text) FREE(token->text);
	free(token);
	}
}

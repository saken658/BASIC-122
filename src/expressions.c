/****************************************************************************
 FILE: expressions.c
 LVU : 1.0.0

 DESC:
 This contains the functions that do numeric, string and boolean expressions.
 Eg: 2*(1+2) , "hello " & "world" , "abc" < "abd" 

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"

/*** Forward declarations ***/
int getPrecedence(enum en_type op);



/*** This is called by other functions to evaluate a numeric expression. The
     end position is the next position after the maths expression ends. If 
     the expression is in brackets this position will be the right bracket ***/
enum en_error evalNumExpr(
	struct st_line *line, int start, int *end, double *result)
{
enum en_type oplist[EXPR_LIST_LEN];
enum en_type result_type;
enum en_error err;
double vallist[EXPR_LIST_LEN];
double val1,val2;
int vallist_pos;
int oplist_pos;
int i,j,prec,neg;
char *strval;
char do_invert;

vallist_pos = -1;
oplist_pos = -1;
do_invert = 0;

/* Create 2 lists , one containing the values, the other containing
   the ops */
for(i=start;i < line->num_tokens;++i) {
	if (++vallist_pos >= EXPR_LIST_LEN) return ERR_EXPR_TOO_COMPLEX;

	/* Check if we've got a leading NOT operator */
	if (line->tokens[i]->type == TYPE_NOT) {
		do_invert = 1;
		if (++i >= line->num_tokens) return ERR_SYNTAX;	
		}

	neg = line->tokens[i]->negated ? -1 : 1;

	/* Get numeric value */
	switch(line->tokens[i]->type) {
		case TYPE_INT:
		case TYPE_FLOAT:
		vallist[vallist_pos] = line->tokens[i]->value;
		++i;
		break;

		case TYPE_VAR:
		if ((err = getVarValue(
			line->tokens[i]->text,NULL,0,&val1,NULL)) != OK)
			return err;

		vallist[vallist_pos] = val1;
		++i;
		break;

		case TYPE_ARR_VAR:
		if ((err = getArrVarValue(line,i,&i,&val1,NULL)) != OK)
			return err;
		vallist[vallist_pos] = val1;
		break;

		case TYPE_STRING:
		case TYPE_STRFUNC:
		case TYPE_STRVAR:
		case TYPE_ARR_STRVAR:
		if ((err = evalStringExpr(
			line,i,&i,&result_type,&val1,&strval)) != OK) 
			return err;
		if (result_type != TYPE_FLOAT) {
			FREE(strval);
			return ERR_INVALID_ARGUMENT;
			}
		vallist[vallist_pos] = val1;
		break;
		
		case TYPE_NUMFUNC:
		if ((err = evalNumFunction(line,i,&i,&val1)) != OK)
			return err;
		vallist[vallist_pos] = val1;
		break;

		case TYPE_LBRACKET:
		/* Recurse for a sub expression in brackets */
		if ((err = evalNumExpr(line,i+1,end,&val1)) != OK)
			return err;
		if (*end >= line->num_tokens ||
		    line->tokens[*end]->type != TYPE_RBRACKET)
			return ERR_MISSING_BRACKET;

		vallist[vallist_pos] = val1;
		i = *end + 1;
		break;

		default: return ERR_SYNTAX;
		}

	vallist[vallist_pos] *= neg;

	if (i >= line->num_tokens) break;

	/* Get operator */
	switch(line->tokens[i]->type) {
		case TYPE_ADD:
		case TYPE_SUB:
		case TYPE_MUL:
		case TYPE_DIV:
		case TYPE_MOD:
		case TYPE_PWR:
		case TYPE_EQUALS:
		case TYPE_NOT_EQUALS:
		case TYPE_GRT_EQUALS:
		case TYPE_LESS_EQUALS:
		case TYPE_GRT:
		case TYPE_LESS: 
		case TYPE_AND:
		case TYPE_OR:
		case TYPE_XOR:
		case TYPE_SHIFT_LEFT:
		case TYPE_SHIFT_RIGHT:
		case TYPE_AMPERSAND:
		case TYPE_BAR:
		case TYPE_EMARK:
		oplist[++oplist_pos] = line->tokens[i]->type;
		break;

		case TYPE_NOT: return ERR_SYNTAX;

		default: goto EVAL;
		}
	}
if (i == start) return ERR_INTERNAL;

/* Evaluate the lists */
EVAL:
*end = i;

/* Go through op precedence in order */
for(prec=1;prec <= 5;++prec) {
	/* Go through lists */
	for(i=0,j=0;i <= oplist_pos;) {
		if (getPrecedence(oplist[i]) != prec) {
			++i;  continue;
			}

		val1 = vallist[i];
		val2 = vallist[i+1];

		/* Do the calculations. */
		switch(oplist[i]) {
			case TYPE_ADD:
			vallist[i] = val1 + val2;
			break;
	
			case TYPE_SUB:
			vallist[i] = val1 - val2;
			break;

			case TYPE_MUL:
			vallist[i] = val1 * val2;
			break;

			case TYPE_DIV:
			if (!val2) return ERR_DIVISION_BY_ZERO;
			vallist[i] = val1 / val2;
			break;

			case TYPE_MOD:
			if (!val2) return ERR_DIVISION_BY_ZERO;
			vallist[i] = (int)val1 % (int)val2;
			break;

			case TYPE_PWR:
			vallist[i] = pow(val1,val2);
			break;

			case TYPE_EQUALS:
			vallist[i] = (val1 == val2);
			break;

			case TYPE_NOT_EQUALS:
			vallist[i] = (val1 != val2);
			break;

			case TYPE_GRT_EQUALS:
			vallist[i] = (val1 >= val2);
			break;

			case TYPE_LESS_EQUALS:
			vallist[i] = (val1 <= val2);
			break;

			case TYPE_GRT:
			vallist[i] = (val1 > val2);
			break;

			case TYPE_LESS: 
			vallist[i] = (val1 < val2);
			break;

			case TYPE_SHIFT_LEFT:
			vallist[i] = (int)val1 << (int)val2;
			break;

			case TYPE_SHIFT_RIGHT:
			vallist[i] = (int)val1 >> (int)val2;
			break;

			case TYPE_AMPERSAND:
			vallist[i] = (int)val1 & (int)val2;
			break;

			case TYPE_BAR:
			vallist[i] = (int)val1 | (int)val2;
			break;

			case TYPE_EMARK:
			vallist[i] = (int)val1 ^ (int)val2;
			break;

			case TYPE_AND:
			vallist[i] = (val1 && val2);
			break;

			case TYPE_OR:
			vallist[i] = (val1 || val2);
			break;

			case TYPE_XOR:
			vallist[i] = ((val1 !=0) != (val2 != 0));
			break;

			default: return ERR_INTERNAL;
			}

		/* Shift lists down by 1 so we remove 2nd value and current
		   op which we've just used */
		for(j=i;j < oplist_pos;++j)
			oplist[j] = oplist[j+1];

		for(j=i+1;j < vallist_pos;++j)
			vallist[j] = vallist[j+1];

		oplist_pos--;
		vallist_pos--;
		}
	}

*result = (do_invert ? !vallist[0] : vallist[0]);
return OK;
}




/*** Return the precedence for an operator. If you add to these don't
     forget to change the max precedence value in the loop in evalNumExpr() ***/
int getPrecedence(enum en_type op)
{
switch(op) {
	case TYPE_PWR:
	case TYPE_SHIFT_LEFT:
	case TYPE_SHIFT_RIGHT:
	case TYPE_AMPERSAND:
	case TYPE_BAR:
	case TYPE_EMARK:
	return 1;

	case TYPE_MOD: 
	case TYPE_MUL:
	case TYPE_DIV:
	return 2;

	case TYPE_ADD:
	case TYPE_SUB:
	return 3;

	case TYPE_EQUALS:
	case TYPE_NOT_EQUALS:
	case TYPE_GRT_EQUALS:
	case TYPE_LESS_EQUALS:
	case TYPE_GRT:
	case TYPE_LESS:
	return 4;

	case TYPE_AND:
	case TYPE_OR:
	case TYPE_XOR:
	return 5;
	default: return 0;
	}
return 0;
}




/*** Evaluate a string expression. This can either be a concatenation
     eg: a$ + "hello" + "world" + b$ type stuff, or it can be a boolean
     test eg: "hello" = "hello". The calling function is reponsible for
     freeing the strres memory ***/
enum en_error evalStringExpr(
	struct st_line *line,
	int start, int *end,
	enum en_type *result_type, double *result, char **strres)
{
enum en_error err;
enum en_type oper;
char *strval;
char *strresult[2];
char do_free;
int i,flag;

*result = 0;
*strres = NULL;
strresult[0] = NULL;
strresult[1] = NULL;
oper = TYPE_AMPERSAND;

for(i=start,flag=0;i < line->num_tokens;++i) {
	do_free = 0;

	/* Get the string */
	switch(line->tokens[i]->type) {
		case TYPE_STRING:
		strval = line->tokens[i++]->text;
		break;
		
		case TYPE_STRVAR:
		if ((err = getVarValue(
			line->tokens[i++]->text,NULL,0,0,&strval)) != OK) 
			return err;
		break;

		case TYPE_ARR_STRVAR:
		if ((err = getArrVarValue(line,i,&i,NULL,&strval)) != OK)
			return err;
		break;

		case TYPE_STRFUNC:
		if ((err = evalStrFunction(line,i,&i,&strval)) != OK)
			return err;
		do_free = 1;
		break;

		case TYPE_INT:
		case TYPE_FLOAT:
		case TYPE_VAR:
		case TYPE_ARR_VAR:
		case TYPE_NUMFUNC:
		FREE(strresult[0]);
		FREE(strresult[1]);
		return ERR_INVALID_ARGUMENT;

		default:
		goto ERROR;
		}

	/* Append to result so far */
	appendString(&strresult[flag],strval);
	if (do_free) FREE(strval);

	/* If we've reached the end then either set string or set maths
	   return value */
	if (i >= line->num_tokens) goto DONE;

	/* Store the operator type for the end of the expression */
	switch(line->tokens[i]->type) {
		case TYPE_AMPERSAND:
		break;

		case TYPE_EQUALS:
		case TYPE_NOT_EQUALS:
		case TYPE_GRT_EQUALS:
		case TYPE_LESS_EQUALS:
		case TYPE_GRT:
		case TYPE_LESS:
		/* Can only have one of these ops per expression.
		   eg: "hello" & "world" = "helloworld" */
		if (flag) goto ERROR;
		oper = line->tokens[i]->type;
		flag = 1;
		break;

		default: goto DONE;
		}
	}

DONE:
*result_type = TYPE_FLOAT;

switch(oper) {
	case TYPE_AMPERSAND:
	*result_type = TYPE_STRING;
	if (!(*strres = strdup(strresult[0]))) {
		stored_errno = errno;
		FREE(strresult[0]);
		FREE(strresult[1]);
		return ERR_MALLOC;
		}
	break;

	case TYPE_EQUALS:
	*result = !strcmp(strresult[0],strresult[1]);
	break;

	case TYPE_NOT_EQUALS:
	*result = strcmp(strresult[0],strresult[1]) != 0;
	break;

	case TYPE_GRT_EQUALS:
	*result = !strcmp(strresult[0],strresult[1]) ||
	           strcmp(strresult[0],strresult[1]) > 0;
	break;

	case TYPE_LESS_EQUALS:
	*result = !strcmp(strresult[0],strresult[1]) ||
	           strcmp(strresult[0],strresult[1]) < 0;
	break;

	case TYPE_GRT:
	*result = strcmp(strresult[0],strresult[1]) > 0;
	break;

	case TYPE_LESS:
	*result = strcmp(strresult[0],strresult[1]) < 0;
	break;

	default: goto ERROR;
	}
FREE(strresult[0]);
FREE(strresult[1]);
*end = i;
return OK;

ERROR:
FREE(strresult[0]);
FREE(strresult[1]);
return ERR_SYNTAX;
}

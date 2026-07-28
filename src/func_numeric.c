/****************************************************************************
 FILE: func_numeric.c
 LVU : 1.2.2

 DESC:
 This file contains the C functions that implement the BASIC functions that
 return numeric values.

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"


#define GET_NUMBER() \
	if (start == limit) return ERR_MISSING_FUNC_ARGUMENT; \
	if ((err = evalNumExpr(line,start,end,result)) != OK) return err;



/*** Call the appropriate numeric return function ***/
enum en_error evalNumFunction(
        struct st_line *line, int start, int *end, double *result)
{
enum en_error err;
int funcend;

if ((*end = findRightBracket(line,start+1)) == -1) return ERR_MISSING_BRACKET;

if (numfuncptr[line->tokens[start]->func]) {
	funcend = *end;
	if ((err = (*numfuncptr[line->tokens[start]->func])(
		line->tokens[start]->func,
		line,start+2,&funcend,*end,result)) != OK) return err;

	if (funcend < *end) return ERR_UNEXPECTED_FUNC_ARGUMENT;
	(*end)++;
	return err;
	}
return ERR_INTERNAL;
}




/*** SQRT(<num>): Return the square root of a value ***/
enum en_error funcSqrt(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 

GET_NUMBER();

if (*result < 0) return ERR_OUT_OF_BOUNDS;
*result = sqrt(*result);

return OK;
}




/*** ROUND(<num>)
     FLOOR(<num>)
     CEIL(<num>)
     Round gives the nearest integer value, floor gives (int)value and 
     ceil gives (int)value + 1 (unless value == (int)value) ***/
enum en_error funcRoundFloorCeil(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 
double f;
int i;

GET_NUMBER();

i = (int)*result;

switch(func) {
	case FUNC_ROUND:
	f = *result - i;
	*result = (double)i + (f >= 0.5);
	break;

	case FUNC_FLOOR:
	*result = (double)i;
	break;

	case FUNC_CEIL:
	/* Make sure we don't return invalid ceiling of integers 
	   eg: ceil(3) = 3 , NOT 4 */
	*result = (double)i + ((double)i != *result);
	break;

	default: return ERR_INTERNAL;
	}
return OK;
}




/*** SIN(<num>): Return the sine of a value ***/
enum en_error funcSin(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 

GET_NUMBER();

*result = sin(*result / DEGS_PER_RADIAN);

return OK;
}




/*** COS(<num>): Do cosine ***/
enum en_error funcCos(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 

GET_NUMBER();

*result = cos(*result / DEGS_PER_RADIAN);

return OK;
}




/*** TAN(<num>): Returns the tangent ***/
enum en_error funcTan(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 

GET_NUMBER();

*result = tan(*result / DEGS_PER_RADIAN);

return OK;
}




/*** ASIN(<num>): Return the arc sine of a value in degrees ***/
enum en_error funcAsin(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 

GET_NUMBER();

*result = DEGS_PER_RADIAN * asin(*result);

return OK;
}




/*** ACOS(<num>): Return the arc cosine of a value in degrees ***/
enum en_error funcAcos(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 

GET_NUMBER();

*result = DEGS_PER_RADIAN * acos(*result);

return OK;
}




/*** ATAN(<num>): Return the arc tan value ***/
enum en_error funcAtan(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 

GET_NUMBER();

*result = DEGS_PER_RADIAN * atan(*result);

return OK;
}




/*** LOG(<num>): Returns the natural logarithm ***/
enum en_error funcLog(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 

GET_NUMBER();

if (*result <= 0) return ERR_OUT_OF_BOUNDS;
*result = log(*result);

return OK;
}




/*** LOG10(<num>): Returns the base 10 logarithm ***/
enum en_error funcLog10(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 

GET_NUMBER();

if (*result <= 0) return ERR_OUT_OF_BOUNDS;
*result = log10(*result);

return OK;
}




/*** HYPOT(<num>,<num>): Returns the hypotenuse length ***/
enum en_error funcHypot(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
double lenx,leny;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get X length */
if ((err = evalNumExpr(line,start,end,&lenx)) != OK) return err;

if (line->tokens[*end]->type != TYPE_COMMA) return ERR_MISSING_FUNC_ARGUMENT; 

/* Get Y length */
if ((err = evalNumExpr(line,*end+1,end,&leny)) != OK) return err;

*result = hypot(lenx,leny);

return OK;
}




/*** ISNUM(<string>): Return 1 if string is a valid number, else 0 ***/
enum en_error funcIsnum(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
enum en_type result_type;
char *strres;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;
if ((err = evalStringExpr(line,start,end,&result_type,result,&strres)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

*result = isNumeric(strres);
free(strres);
return OK;
}




/*** EOF(#<stream>)
     Return result of 1 if we're at the end of the given stream ***/
enum en_error funcEOF(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
double stream;

if (start >= limit - 1) return ERR_MISSING_FUNC_ARGUMENT;
if (line->tokens[start]->type != TYPE_HASH) return ERR_SYNTAX;

/* Get stream */
if ((err = getStream(line,start+1,end,&stream)) != OK) return err;
if (!STREAM_IS_OPEN((int)stream)) return ERR_STREAM_NOT_OPEN;

*result = streams[(int)stream].eof;
return OK;
}




/*** HASDATA(#<stream>)
     Returns 1 if data is available on the given stream else 0.

     WAITDATA(#<stream>[,<millisecond timeout>])
     Waits for data to appear on the screen or for the optional timeout. If
     the timeout occurs it returns 0 , else 1 for data
 ***/
enum en_error funcHasWaitdata(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
struct timeval tv,*tvp;
fd_set mask;
enum en_error err;
double stream, millisecs = 0.0;
int fd;

if (start >= limit - 1) return ERR_MISSING_FUNC_ARGUMENT;
if (line->tokens[start]->type != TYPE_HASH) return ERR_SYNTAX;

/* Get stream */
if ((err = getStream(line,start+1,end,&stream)) != OK) return err;

if (!STREAM_IS_OPEN((int)stream)) return ERR_STREAM_NOT_OPEN;

if (streams[(int)stream].type == STREAM_DIR)
#ifdef SOLARIS
	/* Lets hope this members name doesn't change! */
	fd = streams[(int)stream].dfp->dd_fd;
#else
	fd = dirfd(streams[(int)stream].dfp);
#endif
else
	fd = streams[(int)stream].fd;

if (func == FUNC_WAITDATA) {
	if (*end < limit) {
		if (line->tokens[*end]->type != TYPE_COMMA)
			return ERR_MISSING_FUNC_ARGUMENT;

		if ((err = evalNumExpr(line,*end+1,end,&millisecs)) != OK)
			return err;

		if (millisecs < 0) return ERR_INVALID_ARGUMENT;
		tvp = &tv;
		}
	else tvp = NULL;
	}
else {
	millisecs = 0;
	tvp = &tv;
	}

FD_ZERO(&mask);
FD_SET(fd,&mask);

tv.tv_sec = (int)millisecs / 1000;
tv.tv_usec = ((int)millisecs % 1000) * 1000;

switch(select(FD_SETSIZE,&mask,0,0,tvp)) {
	case -1:
	if (errno != EINTR) {
		stored_errno = errno;
		return ERR_READ_FAILURE;
		}
	/* Fall through */

	case 0:
	*result = 0;
	return OK;
	}
*result = 1;
return OK;
}




/*** SEARCH(<search string>,<searched for string>[,<start pos>])
     Search for a substring within a string and return position ***/
enum en_error funcSearch(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
enum en_type result_type;
char *strres1,*strres2,*ptr;
double spos;
int len;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

strres2 = NULL;

/* Get search string */
if ((err = evalStringExpr(line,start,end,&result_type,result,&strres1)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (line->tokens[*end]->type != TYPE_COMMA) {
	err = ERR_MISSING_FUNC_ARGUMENT; goto ERROR;
	}

/* Get searched for string */
if ((err = evalStringExpr(line,*end+1,end,&result_type,result,&strres2)) != OK)
	goto ERROR;

if (result_type != TYPE_STRING) {
	err = ERR_INVALID_ARGUMENT;  goto ERROR;
	}

/* Get optional start position */
if (line->tokens[*end]->type == TYPE_COMMA) {
	if ((err = evalNumExpr(line,*end+1,end,&spos)) != OK) return err;
	if (spos < 1) return ERR_OUT_OF_BOUNDS;
	if (--spos > (len = strlen(strres1))) spos = len;
	}
else spos = 0;

ptr = strstr(strres1+(int)spos,strres2);
*result = (ptr ? (double)(ptr - strres1) + 1: 0);
err = OK;

ERROR:
free(strres1);
FREE(strres2);
return err;
}




/*** LENGTH(<string>): Return length of the string passed in ***/
enum en_error funcLength(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
enum en_type result_type;
double unused;
char *tmpstr;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get string to look in */
if ((err = evalStringExpr(line,start,end,&result_type,&unused,&tmpstr)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

*result = strlen(tmpstr);
free(tmpstr);
return OK;
}




/*** WORDCNT(<string>): Return a count of the number of words in the string ***/
enum en_error funcWordcnt(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
enum en_type result_type;
double unused;
char *tmpstr,*s,*s2;
int cnt;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get string to count */
if ((err = evalStringExpr(line,start,end,&result_type,&unused,&tmpstr)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

/* Go through string */
for(cnt=0,s=s2=tmpstr;;++cnt) {
	/* Skip whitespace */
	for(s=s2;*s && isspace(*s);++s);
	if (!*s) break;

	/* Find end of word */
	for(s2=s;*s2 && !isspace(*s2);++s2);
	}
*result = cnt;
free(tmpstr);
return OK;
}




/*** VAL(<string>): Return the value of a string (converts it to a number) ***/
enum en_error funcVal(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
enum en_type result_type;
double num;
char *tmpstr;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

if ((err = evalStringExpr(line,start,end,&result_type,&num,&tmpstr)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

/* Make sure string is a valid number. User can check this first using the
   isnum() function */
if (isNumeric(tmpstr)) {
	*result = atof(tmpstr);  err = OK;
	}
else err = ERR_NON_NUMERIC_DATA;

free(tmpstr);
return err;
}




/*** ASC(<string>[,<start pos>])
     Return the ascii value of the first character in the string ***/
enum en_error funcAsc(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
enum en_type result_type;
double pos;
char *tmpstr;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

if ((err = evalStringExpr(line,start,end,&result_type,&pos,&tmpstr)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

/* Get optional character position */
if (line->tokens[*end]->type == TYPE_COMMA) {
	if ((err = evalNumExpr(line,*end+1,end,&pos)) != OK) return err;
	if (pos < 1) return ERR_OUT_OF_BOUNDS;
	if (--pos > strlen(tmpstr)) {
		*result = 0;  return OK;
		}
	}
else pos = 0;

*result = (double)tmpstr[(int)pos];
return OK;
}




/*** RAND(): Return a random number between 0 and 1 ***/
enum en_error funcRand(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
*end = start;
*result = (double)rand() / RAND_MAX;
return OK;
}




/*** TIME():
     Returns the current number of seconds since the epoch with the 
     milliseconds as the fraction ***/
enum en_error funcTime(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
struct timeval tv;

*end = start;

gettimeofday(&tv,NULL);
*result = (double)tv.tv_sec + (double)tv.tv_usec / 1000000;
return OK;
}




/*** FACT(<num>): Return the factorial of the number ***/
enum en_error funcFact(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 
double i;

GET_NUMBER();

if (*result < 0) return ERR_OUT_OF_BOUNDS;
for(i=*result - 1;i > 1;--i) *result *= i;

return OK;
}




/*** MAX(<num>,<num>[,<num>...])
     MIN(<num>,<num>[,<num>...])
     MEAN(<num>,<num>[,<num>...])
     GMEAN(<num>,<num>[,<num>...])
     Returns the max, min, mean (average) or geometric mean of a list of 
     numbers ***/
enum en_error funcMaxMinMean(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 
double val;
int cnt;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT; 

for(cnt=1;;++cnt) {
	if ((err = evalNumExpr(line,start,end,&val)) != OK) return err;

	if (cnt == 1) {
		/* Need at least 2 values to compare */
		if (*end >= limit) return ERR_MISSING_FUNC_ARGUMENT;
		*result = val;
		}
	else
	switch(func) {
		case FUNC_MAX:
		if (val > *result) *result = val;
		break;

		case FUNC_MIN:
		if (val < *result) *result = val;
		break;

		case FUNC_MEAN:
		*result += val;
		break;

		case FUNC_GMEAN:
		*result *= val;
		break;

		default: return ERR_INTERNAL;
		}

	if (*end >= limit) {
		if (func == FUNC_MEAN) *result /= cnt;
		else
		if (func == FUNC_GMEAN) {
			/* Can't do a root of a negative */
			if (*result < 0) return ERR_OUT_OF_BOUNDS;
			*result = pow(*result,1 / (double)cnt);
			}
		return OK;
		}

	if (line->tokens[*end]->type != TYPE_COMMA)
		return ERR_MISSING_FUNC_ARGUMENT;
	start = *end + 1;
	}
return ERR_INTERNAL;
}




/*** MEDIAN(<num>,<num>[,<num>...])
     Returns the median of a list of numbers ***/
enum en_error funcMedian(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
int alloc,cnt;
double val,*list;

alloc = 0;
list = NULL;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT; 

/* Loop through numeric values */
for(cnt=0;;++cnt) {
	if ((err = evalNumExpr(line,start,end,&val)) != OK) {
		FREE(list);  return err;
		}

	if (cnt >= alloc) {
		alloc += ALLOC_BLOCK;
		if (!(list = (double *)realloc(list,alloc * sizeof(double))))
			return ERR_MALLOC;
		}
	list[cnt] = val;

	if (*end >= limit) break;

	if (line->tokens[*end]->type != TYPE_COMMA) {
		FREE(list);
		return ERR_MISSING_FUNC_ARGUMENT;
		}
	start = *end + 1;
	}
/* Need at least 2 values to compare */
if (!cnt) {
	FREE(list);  return ERR_MISSING_FUNC_ARGUMENT;
	}

/* Sort list */
numericSort(cnt+1,list);

/* If list has an odd number of elements that just pick the middle one
   else take average of the 2 middle ones. 'cnt' starts at zero so below
   is opposite way round to what you'd expect */
if (cnt % 2)
	*result = (list[cnt/2] + list[cnt/2+1]) / 2;
else 
	*result = list[cnt/2];

FREE(list);
return OK;
}




/*** MATCH(<string>,<pattern>)
     Returns 1 if the pattern matches the string else 0. The pattern
     consists of characters plus * and ? ***/
enum en_error funcMatch(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
enum en_type result_type;
char *str;
char *pat;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT; 

/* Get string */
if ((err = evalStringExpr(line,start,end,&result_type,result,&str)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

pat = NULL;

if (line->tokens[*end]->type != TYPE_COMMA) {
	err = ERR_MISSING_FUNC_ARGUMENT;  goto ERROR;
	}

/* Get pattern */
start = *end + 1;
if ((err = evalStringExpr(line,start,end,&result_type,result,&pat)) != OK) 
	goto ERROR;

if (result_type != TYPE_STRING) {
	err = ERR_INVALID_ARGUMENT;  goto ERROR;
	}

*result = (double)wildmatch(str,pat);
err = OK;

ERROR:
FREE(str);
FREE(pat);
return err;
}




/*** ARMAX(<numeric array>)
     ARMIN(<numeric array>)
     ARMEAN(<numeric array>)
     ARGMEAN(<numeric array>)
     Returns the max, min , mean or geometric mean of a numeric array ***/
enum en_error funcArMaxMinMean(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
struct st_var *var;
int i;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT; 

/* Check var */
if (getSystemVarNum(line->tokens[start]->text) != NOT_SYSVAR)
	return ERR_VAR_IS_READ_ONLY;

if (!(var = getVariable(line->tokens[start]->text))) return ERR_UNDEFINED_VAR;
if (var->type != TYPE_ARR_VAR) return ERR_INVALID_ARGUMENT;

/* Calc result */
for(i=0,*result=0;i < var->size;++i) {
	if (!i) {
		*result = var->arr_value[0];  continue;
		}
	switch(func) {
		case FUNC_ARMAX:
		if (var->arr_value[i] > *result) *result = var->arr_value[i];
		break;

		case FUNC_ARMIN:
		if (var->arr_value[i] < *result) *result = var->arr_value[i];
		break;

		case FUNC_ARMEAN:
		*result += var->arr_value[i];
		break;

		case FUNC_ARGMEAN:
		*result *= var->arr_value[i];
		break;

		default: return ERR_INTERNAL;
		}
	}
switch(func) {
	case FUNC_ARMEAN:
	*result /= (double)var->size;
	break;

	case FUNC_ARGMEAN:
	/* Can't do a root of a negative */
	if (*result < 0) return ERR_OUT_OF_BOUNDS;
	*result = pow(*result,1 / (double)var->size);
	}
return OK;
}




/*** ARMEDIAN(<numeric array>): Returns the median of a numeric array ***/
enum en_error funcArmedian(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
struct st_var *var;
double *list;
int size;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT; 

/* Check var */
if (getSystemVarNum(line->tokens[start]->text) != NOT_SYSVAR)
	return ERR_VAR_IS_READ_ONLY;

if (!(var = getVariable(line->tokens[start]->text))) return ERR_UNDEFINED_VAR;
if (var->type != TYPE_ARR_VAR) return ERR_INVALID_ARGUMENT;

/* Copy array into temp list */
size = sizeof(double) * var->size;
if (!(list = (double *)malloc(size))) return ERR_MALLOC;
memcpy(list,var->arr_value,size);

/* Sort */
numericSort(var->size,list);

if (var->size % 2)
	*result = list[var->size/2];
else 
	*result = (list[(var->size-1)/2] + list[(var->size-1)/2+1]) / 2;

FREE(list);
return OK;
}




/*** ABS(<num>): Return the absolute value ***/
enum en_error funcAbs(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err; 

GET_NUMBER();

if (*result < 0) *result = -*result;
return OK;
}




/*** ISUPPER(<string>)
     ISLOWER(<string>)
     ISALPHA(<string>)
     ISALNUM(<string>)
     ISDIGIT(<string>)
     ISBLANK(<string>)
     See the documentation for an explanation of what they do ***/
enum en_error funcStringTypeCheck(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
enum en_type result_type;
char *str,*s;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT; 

/* Get string */
if ((err = evalStringExpr(line,start,end,&result_type,result,&str)) != OK)
	return err;

switch(func) {
	case FUNC_ISUPPER:
	for(s=str;*s && isupper(*s);++s);
	break;

	case FUNC_ISLOWER:
	for(s=str;*s && islower(*s);++s);
	break;

	case FUNC_ISALPHA:
	for(s=str;*s && isalpha(*s);++s);
	break;

	case FUNC_ISALPNUM:
	for(s=str;*s && isalnum(*s);++s);
	break;

	case FUNC_ISDIGIT:
	for(s=str;*s && isdigit(*s);++s);
	break;

	case FUNC_ISBLANK:
	if (!*str) {
		/* "" counts as a blank */
		*result = 1;  return OK; 
		}
	for(s=str;*s && isspace(*s);++s);
	break;

	case FUNC_ISPRINT:
	for(s=str;*s && isprint(*s);++s);
	break;

	case FUNC_ISPUNCT:
	for(s=str;*s && isprint(*s);++s);
	break;

	default: return ERR_INTERNAL;
	}
*result = (s != str && *s == '\0');
free(str);
return OK;
}




/*** ENVEXISTS(<env variable name>)
     Returns 1 if the enviroment variable exists, else 0 ***/
enum en_error funcEnvExists(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result)
{
enum en_error err;
enum en_type result_type;
char *str,*ptr;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT; 

/* Get string */
if ((err = evalStringExpr(line,start,end,&result_type,result,&str)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

ptr = getenv(str);
free(str);
*result = ptr ? 1 : 0;
return OK;
}

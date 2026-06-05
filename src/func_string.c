/****************************************************************************
 FILE: func_string.c
 LVU : 1.2.2

 DESC:
 This contains the implementation of the BASIC functions that return string
 data.

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"


/*** Call the appropriate string return function ***/
enum en_error evalStrFunction(
        struct st_line *line, int start, int *end, char **result)
{
enum en_error err;
enum en_function f;
int funcend;

if ((*end = findRightBracket(line,start+1)) == -1) return ERR_MISSING_BRACKET;

f = line->tokens[start]->func - FUNC_CHR;

if (strfuncptr[f]) {
	funcend = *end;

	if ((err = (*strfuncptr[f])(
		line->tokens[start]->func,
		line,start+2,&funcend,*end,result)) != OK) return err;

	if (funcend < *end) {
		free(*result);
		return ERR_UNEXPECTED_FUNC_ARGUMENT;
		}
	(*end)++;
	return err;
	}
return ERR_INTERNAL;
}




/*** CHR$(<num>): Return the character for the given ascii value ***/
enum en_error funcChr(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
double ascii;
char ch[2];

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

if ((err = evalNumExpr(line,start,end,&ascii)) != OK) return err;
if (ascii < 0 || ascii > 255) return ERR_OUT_OF_BOUNDS;

ch[0] = (char)ascii;
ch[1] = '\0';
*result = strdup(ch);
return OK;
}




/*** MID$(<string>,<start pos>,<length>)
     LEFT$(<string>,<length>)
     RIGHT$(<string>,<length>)
     Does mid$(), left$() and right$() functions ***/
enum en_error funcMidLeftRight(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **strres)
{
enum en_error err;
enum en_type result_type;
double tmppos,tmplen,len;
char *tmpstr;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get string to look in */
if ((err = evalStringExpr(line,start,end,&result_type,&tmppos,&tmpstr)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (line->tokens[*end]->type != TYPE_COMMA) {
	err = ERR_MISSING_FUNC_ARGUMENT;  goto ERROR;
	}

/* Get position to start from if this is mid$() */
if (func == FUNC_MID) {
	start = *end + 1;
	if ((err = evalNumExpr(line,start,end,&tmppos)) != OK) goto ERROR;
	if (tmppos < 1) {
		err = ERR_OUT_OF_BOUNDS;  goto ERROR;
		}
	if (line->tokens[*end]->type != TYPE_COMMA) {
		err = ERR_MISSING_FUNC_ARGUMENT;  goto ERROR;
		}
	}
else tmppos = 1;

/* Get length */
start = *end + 1;
if ((err = evalNumExpr(line,start,end,&len)) != OK) goto ERROR;
if (len < 0) {
	err = ERR_OUT_OF_BOUNDS;  goto ERROR;
	}
if (!len) {
	/* If length of zero just return empty string */
	free(tmpstr);
	*strres = strdup("");
	return OK;
	}	
tmplen = strlen(tmpstr);

if (func == FUNC_RIGHT) {
	if (len >= tmplen) {
		if (!(*strres = strdup(tmpstr))) {
			stored_errno = errno;
			err = ERR_MALLOC;
			goto ERROR;
			}
		}
	else {
		if (!(*strres = (char *)malloc((int)len+1))) {
			stored_errno = errno;
			err = ERR_MALLOC;
			goto ERROR;
			}
		strncpy(*strres,tmpstr+(int)(tmplen - len),len);
		(*strres)[(int)len] = '\0';
		}
	}
else {
	/* left$ and mid$ */
	if (tmppos > tmplen) *strres = strdup("");
	else {
		if (len > tmplen) len = tmplen;
		if (!(*strres = (char *)malloc((int)len+1))) {
			stored_errno = errno;
			err = ERR_MALLOC;
			goto ERROR;
			}
		strncpy(*strres,tmpstr+(int)tmppos-1,len);
		(*strres)[(int)len] = '\0';
		}
	}
err = OK;

ERROR:
free(tmpstr);
return err;
}




/*** FORMAT$(<format string>,<num>): Format a number for printing out ***/
enum en_error funcFormat(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **strres)
{
enum en_error err;
enum en_type result_type;
double value;
char valstr[SHORT_STR];
char *format;
char *f,*f2,*v,*v2,prevf;
char output[BUFFSIZE];
char is_neg,flag;
int opos,negpos;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get format string */
if ((err = evalStringExpr(line,start,end,&result_type,&value,&format)) != OK)
	return err;
if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (line->tokens[*end]->type != TYPE_COMMA) {
	err = ERR_MISSING_FUNC_ARGUMENT;  goto ERROR;
	}

/* Get value to format */
start = *end + 1;
if ((err = evalNumExpr(line,start,end,&value)) != OK) goto ERROR;

if (value < 0) {
	sprintf(valstr,"%f",-value);
	is_neg = 1;
	}
else {
	sprintf(valstr,"%f",value);
	is_neg = 0;
	}

/* Parse format and create output string */
if (!(f = strchr(format,'.'))) 
	f = format + strlen(format) - 1;
else 
	f--;
f2 = f + 1;

v2 = valstr + strlen(valstr) - 1;
if (!(v = strchr(valstr,'.')))
	v = v2;
else {
	/* Remove trailing zeros */
	for(;v2 != v;--v2) if (*v2 == '0') *v2 = '\0';
	v--;
	}
v2 = v + 1;

bzero(output,BUFFSIZE);
negpos = -1;

/* Go backwards from decimal point */
for(opos = f - format,prevf = 0,flag=0;f >= format;--f,--opos) {
	switch(*f) {
		case '#':
		/* Flag is to prevent # and X being mixed */
		if (flag == 2) return ERR_INVALID_FORMAT_STRING;
		flag = 1;
		if (v >= valstr)
			output[opos] = *v--;
		else {
			output[opos] = '0';
			if (negpos == -1) negpos = 0;
			}
		break;	
		
		case 'X':
		if (flag == 1) return ERR_INVALID_FORMAT_STRING;
		flag = 2;
		if (v >= valstr)
			output[opos] = *v--;
		else {
			output[opos] = ' ';
			if (negpos == -1) negpos = opos;
			}
		break;

		case ',':
		if (prevf == '#')
			output[opos] = ',';
		else {
			output[opos] = (v > valstr ? ',' : ' ');
			if (negpos == -1) negpos = opos;
			}
		break;


		default:
		return ERR_INVALID_FORMAT_STRING;
		}
	prevf = *f;
	}
if (is_neg) output[negpos] = '-';

/* Go forwards from point */
v = v2 + (*v2 == '.');

for(f=f2,opos = f2 - format,flag=0;*f;++f,++opos) {
	switch(*f) {
		case '#':
		if (flag == 2) return ERR_INVALID_FORMAT_STRING;
		flag = 1;
		output[opos] = (*v ? *v++ : '0');
		break;

		case 'X':
		if (flag == 1) return ERR_INVALID_FORMAT_STRING;
		flag = 2;
		output[opos] = (*v ? *v++ : ' ');
		break;

		case '.':
		output[opos] = '.';  break;

		case ',':
		output[opos] = ',';  break;

		default:
		return ERR_INVALID_FORMAT_STRING;
		}
	}
if (!(*strres = strdup(output))) {
	stored_errno = errno;
	err = ERR_MALLOC;
	}
else err = OK;

ERROR: 
free(format);
return err;
}




/*** PAD$(<string>,<num>)
     Return a line of the passed in string repeated the given number of
     times ***/
enum en_error funcPad(
	enum en_function func,
        struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
enum en_type result_type;
char *tmpstr;
double repeat;
int slen,len,i;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get string to repeat */
if ((err = evalStringExpr(line,start,end,&result_type,&repeat,&tmpstr)) != OK)
	return err;
if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (line->tokens[*end]->type != TYPE_COMMA) {
	err = ERR_MISSING_FUNC_ARGUMENT;  goto ERROR;
	}

/* Get num of times to repeat */
if ((err = evalNumExpr(line,*end+1,end,&repeat)) != OK) goto ERROR;
if (repeat < 0) {
	err = ERR_OUT_OF_BOUNDS; goto ERROR;
	}
len = (int)((slen = strlen(tmpstr)) * repeat);

if (!(*result = (char *)malloc(len + 1))) {
	stored_errno = errno;
	err = ERR_MALLOC;
	goto ERROR;
	}

**result = '\0';

for(i=0;i < len;i += slen) strcat(*result,tmpstr);
err = OK;

ERROR:
free(tmpstr);
return err;
}




/*** WORD$(<string>,<pos>)
     Return the word at the given position in the string ***/
enum en_error funcWord(
	enum en_function func,
        struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
enum en_type result_type;
char *tmpstr,*s,*s2;
int cnt,len;
double wnum;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get string to search */
if ((err = evalStringExpr(line,start,end,&result_type,&wnum,&tmpstr)) != OK)
	return err;
if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (line->tokens[*end]->type != TYPE_COMMA) {
	err = ERR_MISSING_FUNC_ARGUMENT;  goto ERROR;
	}

/* Get word number */
if ((err = evalNumExpr(line,*end+1,end,&wnum)) != OK) goto ERROR;
if (wnum < 1) {
	err = ERR_OUT_OF_BOUNDS;  goto ERROR;
	}

/* Search for word */
for(cnt=0,s=s2=tmpstr;cnt < wnum;++cnt) {
	/* Skip start whitespace */
	for(s=s2;*s && isspace(*s);++s);
	if (!*s) break;

	/* Find end of word */
	for(s2=s;*s2 && !isspace(*s2);++s2);
	if (!*s2) break;
	}
if (!*s) *result = strdup("");
else {
	len = s2 - s;
	if (!(*result = (char *)malloc(len+1))) {
		stored_errno = errno;
		err = ERR_MALLOC;
		goto ERROR;
		}	
	memcpy(*result,s,len);
	(*result)[len] = '\0';
	}
err = OK;

ERROR:
free(tmpstr);
return err;
}




/*** ERROR$(<error num>):
     Return the error string for the given error number ***/
enum en_error funcError(
	enum en_function func,
        struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
double errnum;

if ((err = evalNumExpr(line,start,end,&errnum)) != OK) return err;

if (errnum < 0 || errnum >= NUM_ERRORS) return ERR_OUT_OF_BOUNDS; 

*result = strdup(error[(int)errnum]);
return OK;
}




/*** DATE$(<time>): Returns the date string ***/
enum en_error funcDate(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
double secs;
time_t tsecs;
int pos;
char *tmp;

if ((err = evalNumExpr(line,start,end,&secs)) != OK) return err;
if (secs < 0) return ERR_OUT_OF_BOUNDS; 

tsecs = (time_t)secs;
tmp = strdup(ctime(&tsecs));

/* Get rid of that damn newline at the end */
pos = strlen(tmp) - 1;
if (tmp[pos] == '\n') tmp[pos] = '\0';

*result = tmp;
return OK;
}




/*** STAT(<path>),LSTAT(<path>):
     Returns status information about the given filename or directory ***/
enum en_error funcStatLstat(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result)
{
struct stat fs;
struct passwd *pwd;
struct group *grp;
enum en_error err;
enum en_type result_type;
char udata[SHORT_STR];
char gdata[SHORT_STR];
char statdata[SHORT_STR];
char *filename;
char *ftype;
double unused;
int ret;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get filename */
if ((err = evalStringExpr(line,start,end,&result_type,&unused,&filename)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (func == FUNC_STAT) 
	ret = stat(filename,&fs);
else
	ret = lstat(filename,&fs);

free(filename);

if (ret == -1) {
	stored_errno = errno;
	return ERR_CANT_STAT_FILE;
	}

/* Set filetype */
switch(fs.st_mode & S_IFMT) {
	case S_IFREG : ftype = "FILE";  break;
	case S_IFDIR : ftype = "DIR";   break;
	case S_IFLNK : ftype = "LINK";  break;
	case S_IFIFO : ftype = "FIFO";  break;
	case S_IFBLK : ftype = "BLOCK";  break;
	case S_IFCHR : ftype = "CHAR";   break;
	case S_IFSOCK: ftype = "SOCKET";  break;

	default: ftype = "?"; 
	}

/* Get ownder and group info */
if ((pwd = getpwuid(fs.st_uid))) 
	strcpy(udata,pwd->pw_name);
else
	sprintf(udata,"%u",(u_int)fs.st_uid);

if ((grp = getgrgid(fs.st_gid)))
	strcpy(gdata,grp->gr_name);
else
	sprintf(gdata,"%u",(u_int)fs.st_gid);

/* Order is file type, permissions, size, last access, last mod, last
   status change, number of hard links */
snprintf(statdata,SHORT_STR-1,"%s %04o %s %s %u %u %u %u %u",
	ftype,
	fs.st_mode & 0xFFF,
	udata,gdata,
	(u_int)fs.st_size,
	(u_int)fs.st_atime,
	(u_int)fs.st_mtime,
	(u_int)fs.st_ctime,
	(u_int)fs.st_nlink);

*result = strdup(statdata);

return OK;
}




/*** MAX$(<string>,<string>[,<string>...])
     MIN$(<string>,<string>[,<string>...])
     Returns the max or min values of a list of strings ***/
enum en_error funcMaxMinStr(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
enum en_type result_type;
double unused;
char *val;
int cnt;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;
val = NULL;
*result = NULL;

for(cnt=1;;++cnt) {
	if ((err = evalStringExpr(
		line,start,end,&result_type,&unused,&val)) != OK) break;

	if (result_type != TYPE_STRING) {
		err = ERR_INVALID_ARGUMENT;  break;
		}
	if (cnt == 1) {
		if (*end >= limit) {
			err = ERR_MISSING_FUNC_ARGUMENT;  break;
			}
		*result = val;
		val = NULL;
		}
	else 
	if (func == FUNC_MAXSTR) {
		if (strcmp(val,*result) > 0) {
			*result = val;  val = NULL;
			}
		}
	else
	if (strcmp(val,*result) < 0) {
		*result = val;  val = NULL;
		}
	FREE(val);

	if (*end >= limit) return OK;

	if (line->tokens[*end]->type != TYPE_COMMA) {
		err = ERR_SYNTAX;  break;
		}
	start = *end + 1;
	}
FREE(*result);
FREE(val);
return err;
}




/*** ARMAX$(<string array>)
     ARMIN$(<string array>): Returns the max or min values in an array ***/
enum en_error funcArMaxMinStr(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result)
{
struct st_var *var;
char *val,*res;
int i;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Check variable */
if (getSystemVarNum(line->tokens[start]->text) != NOT_SYSVAR)
	return ERR_VAR_IS_READ_ONLY;

if (!(var = getVariable(line->tokens[start]->text))) return ERR_UNDEFINED_VAR;
if (var->type != TYPE_ARR_STRVAR) return ERR_INVALID_ARGUMENT;

/* Get result */
for(i=0,val="",res="";i < var->size;++i) {
	if (!i) {
		res = var->arr_str_value[0] ? var->arr_str_value[0] : "";
		continue;
		}
	val = var->arr_str_value[i] ? var->arr_str_value[i] : "";

	if (func == FUNC_ARMAXSTR) {
		if (strcmp(val,res) > 0) res = var->arr_str_value[i];
		}
	else {
		if (strcmp(val,res) < 0) res = var->arr_str_value[i];
		}
	}
*end = start + 1;
return (*result = strdup(res ? res : "")) ? OK : ERR_MALLOC;
}




/*** TOSTR$(<num>): Converts a number to a string like FORMAT$() except without
     you having to worry about a formatting string ***/
enum en_error funcTostr(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
char valstr[SHORT_STR];
double value;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get value to format */
if ((err = evalNumExpr(line,start,end,&value)) != OK) return err;
sprintf(valstr,"%f",value);
*result = strdup(valstr);
return OK;
}




/*** UPPER$(<string>)
     LOWER$(<string>): Converts a string to upper or lower case ***/
enum en_error funcUpperLower(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
enum en_type result_type;
double unused;
char *s;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get string */
if ((err = evalStringExpr(line,start,end,&result_type,&unused,result)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (func == FUNC_UPPER)
	for(s=*result;*s;*s=toupper(*s),++s);
else
	for(s=*result;*s;*s=tolower(*s),++s);
return OK;
}




/*** REVERSE$(<string>): Reverses the string ***/
enum en_error funcReverse(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
enum en_type result_type;
double unused;
char *s1,*s2,*tmpstr;
int len;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get string */
if ((err = evalStringExpr(line,start,end,&result_type,&unused,&tmpstr)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

len = strlen(tmpstr);
if (!(*result = (char *)malloc(len+1))) {
	free(tmpstr);  return ERR_MALLOC;
	}
for(s1=tmpstr+len-1,s2=*result;s1 >= tmpstr;--s1,++s2) *s2 = *s1;
*s2 = '\0';
free(tmpstr);
return OK;
}




/*** SWAPCASE$(<string>): Reverses the cases of the letters in the string ***/
enum en_error funcSwapcase(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
enum en_type result_type;
double unused;
char *s;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get string */
if ((err = evalStringExpr(line,start,end,&result_type,&unused,result)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

for(s=*result;*s;++s) {
	if (isupper(*s)) *s = tolower(*s);
	else
	if (islower(*s)) *s = toupper(*s);
	}
return OK;
}




/*** GETENV$(<env variable name>)
     Returns the value of the given enviroment (not BASIC) variable. If it 
     doesn't exist it returns empty string ***/
enum en_error funcGetenv(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result)
{
enum en_error err;
enum en_type result_type;
double unused;
char *tmpstr,*ptr;

if (start == limit) return ERR_MISSING_FUNC_ARGUMENT;

/* Get string */
if ((err = evalStringExpr(line,start,end,&result_type,&unused,&tmpstr)) != OK)
	return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

ptr = getenv(tmpstr);
*result = strdup(ptr ? ptr : "");
free(tmpstr);

return OK;
}

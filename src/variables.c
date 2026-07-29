/****************************************************************************
 FILE: variables.c
 LVU : 1.2.0

 DESC: 
 Code to create/delete/get/set & sort BASIC variables.

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"


/*** Create a variable and enter it into the lists. This assumes it doesn't
     aready exist. ***/
enum en_error createVariable(
	char *name,
	enum en_type type, int *index, int depth, struct st_var **newvar)
{
struct st_var *var,*var2;
u_int size;
int c,i,j;

if (!(var = (struct st_var *)malloc(sizeof(struct st_var)))) {
	stored_errno = errno;  return ERR_MALLOC;
	}
var->name = strdup(name);
toupperString(var->name);
var->type = type;
var->size = 0;
var->depth = 0;
var->str_value = NULL;
var->value = 0;
var->arr_str_value = NULL;
var->arr_value = NULL;
var->next = NULL;

/* If we have an array then set it up */
if (depth) {
	for(i=1,size=index[0];i < depth;++i) size *= index[i];

	if (size > MAX_ARRAY_SIZE) {
		free(var);  return ERR_ARRAY_TOO_BIG;
		}

	var->size = size;
	var->depth = depth;
	memcpy(var->index,index,MAX_ARRAY_DEPTH * sizeof(int));

	/* Prodindex is used in calculation of linear array location for
	   passed in indexes */
	bzero(var->prodindex,MAX_ARRAY_DEPTH * sizeof(int));
	for(i=0;i < depth-1;++i) {
		var->prodindex[i] = var->index[i];
		for(j=i+1;j < depth;++j) var->prodindex[i] *= var->index[j];
		}
	var->prodindex[i] = var->index[i];

	/* Allocate array */
	switch(type) {
		case TYPE_ARR_VAR:
		/* Could be allocating a lot of mem here so check malloc()
		   return */
		var->arr_value = (double *)malloc(sizeof(double) * size);
		if (!var->arr_value) {
			stored_errno = errno;
			free(var);
			return ERR_MALLOC;
			}
		bzero(var->arr_value,sizeof(double) * size);
		break;

		case TYPE_ARR_STRVAR:
		var->arr_str_value = (char **)malloc(sizeof(char *) * size);
		if (!var->arr_str_value) {
			stored_errno = errno;
			free(var);
			return ERR_MALLOC;
			}
		bzero(var->arr_str_value,sizeof(char *) * size);
		break;

		default:
		free(var);
		return ERR_INTERNAL;
		}
	}

/* Add to var list */
c = (int)toupper(name[0]);
if (varlist[c]) {
	for(var2=varlist[c];var2->next;var2=var2->next);
	var2->next = var;
	}
else 
	varlist[c] = var;

if (newvar) *newvar = var;

return OK;
}




/*** Delete a variable ***/
void deleteVariable(struct st_var *var)
{
int i;

free(var->name);

switch(var->type) {
	case TYPE_VAR: return;

	case TYPE_STRVAR:
	FREE(var->str_value);
	return;

	case TYPE_ARR_VAR:
	FREE(var->arr_value);
	return;

	case TYPE_ARR_STRVAR:
	for(i=-0;i < var->size;++i) FREE(var->arr_str_value[i]);
	FREE(var->arr_str_value);
	}
}




/*** Delete all variables ***/
void deleteAllVariables(void)
{
struct st_var *var,*next;
int i;

/* Go through var list and wipe it */
for(i=0;i < 256;++i) {
	if (varlist[i]) {
		for(var = varlist[i];var;) {
			next = var->next;
			deleteVariable(var);
			var = next;
			}
		varlist[i] = NULL;
		}
	}
last_error = OK;
}




/*** Find a variable based on its name ***/
struct st_var *getVariable(char *name)
{
struct st_var *var;
int c;

c = (int)toupper(name[0]);
if ((var = varlist[c])) {
	for(var=varlist[c];var;var=var->next) 
		if (!strcasecmp(var->name,name)) return var;
	}
return var;
}




/*** Set a variable value. If its not an array then create it automatically
     if it doesn't already exist ***/
enum en_error setVarValue(
	char *name, int *index, int depth, double value, char *strval)
{
struct st_var *var;
enum en_type type;
enum en_error err;
int idx;

if (getSystemVarNum(name) != NOT_SYSVAR) return ERR_VAR_IS_READ_ONLY;

/* Try to find the variable. If not found then create */
if (!(var = getVariable(name))) {
	/* Don't create array variables. This is done by the "dim" command */
	if (depth) return ERR_UNDEFINED_ARRAY;

	/* Do sanity checks */
	if (name[strlen(name)-1] == '$') {
		if (!strval) return ERR_INVALID_ARGUMENT;
		type = TYPE_STRVAR;
		}
	else {
		if (strval) return ERR_INVALID_ARGUMENT;
		type = TYPE_VAR;
		}
	if ((err = createVariable(name,type,NULL,0,&var)) != OK) return err;
	idx = 0;
	}
else {
	/* Get index */
	if (depth) {
		if ((err = calcArrIndex(var,index,depth,&idx)) != OK)
			return err;
		}
	else idx = 0;

	/* Do sanity checks */
	if (var->size && idx < 1) return ERR_MISSING_ARRAY_INDEX;
	if (var->type == TYPE_VAR && strval) return ERR_INVALID_ARGUMENT;
	if (var->type == TYPE_STRVAR && !strval) return ERR_INVALID_ARGUMENT;
	}

/* Set the value */
switch(var->type) {
	case TYPE_VAR:
	case TYPE_ARR_VAR:
	if (idx) {
		if (idx > var->size) return ERR_INDEX_OUT_OF_BOUNDS;
		var->arr_value[(int)idx-1] = value;
		}
	else
		var->value = value;
	break;

	case TYPE_STRVAR:
	case TYPE_ARR_STRVAR:
	if (idx) {
		if (idx > var->size) return ERR_INDEX_OUT_OF_BOUNDS;
		--idx;
		FREE(var->arr_str_value[(int)idx]);
		if (!(var->arr_str_value[(int)idx] = strdup(strval))) {
			stored_errno = errno;
			return ERR_MALLOC;
			}
		}
	else {
		FREE(var->str_value);
		if (!(var->str_value = strdup(strval))) {
			stored_errno = errno;
			return ERR_MALLOC;
			}
		}
	break;

	default: return ERR_INTERNAL;
	}
return OK;
}




/*** Set a variable value. We know if we're getting a string value because
     the pointer will be not null. strval return is a direct pointer to the
     variable data or an empty string, it is NOT a copy. ***/
enum en_error getVarValue(
	char *name, int *index, int depth, double *value, char **strval)
{
struct st_var *var;
enum en_error err;
int sv,idx;

/* If they're system vars just bypass normal code */
if ((sv = getSystemVarNum(name)) != NOT_SYSVAR) 
	return getSystemVarValue(sv,index,depth,value,strval);

if (!(var = getVariable(name))) return ERR_UNDEFINED_VAR;

if (depth) {
	if ((err = calcArrIndex(var,index,depth,&idx)) != OK)
		return err;
	}
else idx = 0;

switch(var->type) {
	case TYPE_VAR:
	if (strval) return ERR_INTERNAL;
	*value = var->value;
	break;

	case TYPE_STRVAR:
	if (!strval) return ERR_INTERNAL;
	*strval = (var->str_value ? var->str_value : "");
	break;

	case TYPE_ARR_VAR:
	if (idx > var->size) return ERR_INDEX_OUT_OF_BOUNDS;
	if (!idx) return ERR_VAR_IS_ARRAY;
	*value = var->arr_value[idx-1];
	break;

	case TYPE_ARR_STRVAR:
	if (idx > var->size) return ERR_INDEX_OUT_OF_BOUNDS;
	if (!idx) return ERR_VAR_IS_ARRAY;
	*strval = (var->arr_str_value[idx-1] ? var->arr_str_value[idx-1] : "");
	}
return OK;
}




/*** Checks the given array index against the variables defined dimentional
     size , then works out the actual position in the linear array where
     the given multidimensional index actually is ***/
enum en_error calcArrIndex(struct st_var *var, int *index, int depth, int *idx)
{
int i;

if (!var->depth) return ERR_VAR_NOT_ARRAY;

if (depth != var->depth) return ERR_INVALID_ARRAY_INDEX;

/* Check we've not gone over */
for(i=0;i < depth;++i) 
	if (index[i] > var->index[i]) return ERR_INDEX_OUT_OF_BOUNDS;

/* Calculate index. For an index dimensioned A,B,C,D a given location
   W,X,Y,Z is found at position (W-1)*B*C*D + (X-1)*B*C + (Y-1)*C + (Z-1)
   in the linear array. prodindex has pre-calculated B*C*D, B*C etc */
for(i=0,*idx=0;i < depth-1;++i) *idx += (index[i] - 1) * var->prodindex[i+1];
*idx += (index[depth-1] - 1);
++*idx; /* Add 1 since index starts at 1 , not 0 for calling functions */

if (*idx < 1) return ERR_INVALID_ARRAY_INDEX;
return OK;
}




/*** Get the position of the system variable in the list ***/
int getSystemVarNum(char *name)
{
int i;

for(i=0;i < NUM_SYSTEM_VARS;++i)
	if (!strcasecmp(name,system_vars[i])) return i;
return NOT_SYSVAR;
}




/*** Return the value of a system variable ***/
enum en_error getSystemVarValue(
	int sv, int *index, int depth, double *value, char **strval)
{
struct winsize *ws;
char dir[LONG_STR];

if (sv != SYSVAR_ARGV && index) return ERR_VAR_NOT_ARRAY;

switch(sv) {
	case SYSVAR_ARGC:
	*value = basic_argc;
	return OK;

	case SYSVAR_ARGV:
	if (!depth) return ERR_VAR_IS_ARRAY;
	if (depth > 1) return ERR_INVALID_ARRAY_INDEX;
	if (index[0] > basic_argc) return ERR_INDEX_OUT_OF_BOUNDS;
	*strval = basic_argv[index[0]-1];
	return OK;

	case SYSVAR_VERSION:
	*strval = VERSION;
	return OK;

	case SYSVAR_CREDITS:
	*strval = "Written by and copyright Neil Robertson 2006";
	return OK;

	case SYSVAR_BUILD_DATE:
	*strval = BUILD_DATE;
	return OK;

	case SYSVAR_CURR_DIR:
	getcwd(dir,LONG_STR);
	*strval = dir;
	return OK;

	case SYSVAR_USERNAME:
	*strval = username;
	return OK;

	case SYSVAR_USERGROUP:
	*strval = usergroup;
	return OK;

	case SYSVAR_LAST_ERROR:
	*value = (double)last_error;
	return OK;

	case SYSVAR_NUM_ERRORS:
	*value = NUM_ERRORS;
	return OK;

	case SYSVAR_NUM_PENS:
	*value = NUM_FGCOLS;
	return OK;

	case SYSVAR_NUM_PAPERS:
	*value = NUM_BGCOLS;
	return OK;

	case SYSVAR_NUM_STYLES:
	*value = NUM_STYLES;
	return OK;

	case SYSVAR_SCR_WIDTH:
	case SYSVAR_SCR_HEIGHT:
	ws = getWinSize();
	*value = (sv == SYSVAR_SCR_WIDTH ? ws->ws_col : ws->ws_row);
	return OK;

	case SYSVAR_PI:
	*value = 3.1415926535;
	return OK;

	case SYSVAR_DEG_PER_RAD:
	*value = DEGS_PER_RADIAN;
	return OK;
	}
return ERR_INTERNAL;
}




/*** Get a value for an array variable based on line position ***/
enum en_error getArrVarValue(
        struct st_line *line, int start, int *end, double *value, char **strval)
{
enum en_error err;
int index[MAX_ARRAY_DEPTH];
int depth;

if ((err = getArrIndex(line,start+2,end,index,&depth)) != OK) return err;
if (*end >= line->num_tokens || line->tokens[*end]->type != TYPE_RBRACKET)
	return ERR_MISSING_BRACKET;
(*end)++;

return getVarValue(line->tokens[start]->text,index,depth,value,strval);
}




/*** Get an index for an array and check if its valud ***/
enum en_error getArrIndex(
	struct st_line *line, int start, int *end, int *index, int *depth)
{
enum en_error err;
double idx;
int i;

for(i=0;i < MAX_ARRAY_DEPTH;++i) {
	if ((err = evalNumExpr(line,start,end,&idx)) != OK) return err;
	if (idx < 1) return ERR_INVALID_ARRAY_INDEX;
	index[i] = (int)idx;

	if (*end >= line->num_tokens) return ERR_SYNTAX;

	switch(line->tokens[*end]->type) {
		case TYPE_RBRACKET:
		*depth = i+1;  return OK;

		case TYPE_COMMA:
		start = *end + 1;  break;

		default: return ERR_SYNTAX;
		}
	}
return ERR_MAX_DIM_EXCEEDED;
}




/*** Sort a 1 dimensional array into ascending order ***/
enum en_error sortArray(char *name)
{
struct st_var *var;
int i,j;
char *strval;

/* Check var */
if (getSystemVarNum(name) != NOT_SYSVAR) return ERR_VAR_IS_READ_ONLY;

if (!(var = getVariable(name))) return ERR_UNDEFINED_VAR;

switch(var->type) {
	case TYPE_ARR_VAR:
	/* Do sort */
	numericSort(var->size,var->arr_value);
	return OK;

	case TYPE_ARR_STRVAR:
	for(i=1;i < var->size;++i) {
		strval = var->arr_str_value[i];
		for(j=i;j && strcmp(
			var->arr_str_value[j-1] ? var->arr_str_value[j-1] : "",
			strval ? strval : "") > 0;--j)
			var->arr_str_value[j] = var->arr_str_value[j-1];
		var->arr_str_value[j] = strval;
		}
	return OK;
	}
return ERR_VAR_NOT_ARRAY;
}

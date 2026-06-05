/****************************************************************************
 FILE: com_io.c
 LVU : 1.0.0

 DESC:
 All the functions for the BASIC interpreter commands that deal with I/O
 and file operations. eg: OPEN 

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"



/*** PRINT,WRITE: 
     Print something lovely to screen or file. We have to send a terminal
     reset if we've set the colours BEFORE we print a newline else some
     terminals/xterms go a bit funny. Which makes the code messy. ***/
enum en_error comPrintWrite(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
enum en_type result_type;
double result,stream;
int st,end,fd;
char *strres;
char numtext[SHORT_STR];
char done_cols;

fd = STDOUT;
strres = NULL;
done_cols = 0;

/* Go through arguments to print out */
for(st=start+1;st < line->num_tokens;) {
	switch(line->tokens[st]->type) {
		case TYPE_HASH:
		/* See if we've been passed a stream */
		if ((err = getStream(line,st+1,&end,&stream)) != OK) 
			goto COLRESET;

		if (streams[(int)stream].type == STREAM_READ ||
		    streams[(int)stream].type == STREAM_DIR) {
			err = ERR_INVALID_STREAM_TYPE;  goto COLRESET;
			}

		if (streams[(int)stream].fd == -1) {
			err = ERR_STREAM_NOT_OPEN;  goto COLRESET;
			}

		fd = streams[(int)stream].fd;
		st = end + 1;
		done_cols = 0;
		continue;

		case TYPE_STRING:
		case TYPE_STRVAR:
		case TYPE_ARR_STRVAR:
		case TYPE_STRFUNC:
		/* Try numeric first , if that fails try string */
		result_type = TYPE_FLOAT;
		if (evalNumExpr(line,st,&end,&result) != OK) {
			if ((err = evalStringExpr(
				line,
				st,&end,&result_type,&result,&strres)) != OK) 
				goto COLRESET;
			}
		if (!done_cols) {
			done_cols = 1;
			if ((err = setColours(fd)) != OK) goto COLRESET;
			}
		if (result_type == TYPE_STRING) {
			err = fdWrite(fd,strres,strlen(strres));
			free(strres);
			if (err != OK) goto COLRESET;
			}
		else {
			if (result == (int)result) {
				sprintf(numtext,"%d",(int)result);
				if ((err = fdWrite(fd,numtext,strlen(numtext))) != OK) 
					goto COLRESET;
				}
			else {
				sprintf(numtext,"%f",result);
				if ((err = fdWrite(fd,numtext,strlen(numtext))) != OK) 
					goto COLRESET;
				}
			}
		break;

		default:
		if ((err = evalNumExpr(line,st,&end,&result)) != OK) 
			goto COLRESET;

		if (!done_cols) {
			done_cols = 1;
			if ((err = setColours(fd)) != OK) goto COLRESET;
			}
		if (result == (int)result) {
			sprintf(numtext,"%d",(int)result);
			if ((err = fdWrite(fd,numtext,strlen(numtext))) != OK)
				goto COLRESET;
			}
		else {
			sprintf(numtext,"%f",result);
			if ((err = fdWrite(fd,numtext,strlen(numtext))) != OK)
				goto COLRESET;
			}
		}

	if (end < line->num_tokens && line->tokens[end]->type != TYPE_COMMA) {
		err = ERR_SYNTAX;  
		goto COLRESET;
		}

	st = end + 1;
	}

/* PRINT puts a newline at the end. WRITE doesn't */
if (done_cols && (pen || paper || style)) fdWrite(fd,"\033[0m",4);
if (com == COM_PRINT) return fdWrite(fd,"\n",1);
return OK;

COLRESET:
if (done_cols && (pen || paper || style)) fdWrite(fd,"\033[0m",4);
if (st != start+1) fdWrite(fd,"\n",1);
return err;
}




/*** INPUT/CINPUT: Read a value into one or more variables ***/
enum en_error comInput(enum en_command com, struct st_line *line, int start)
{
struct dirent *ds;
enum en_error err;
double stream;
char one,input[BUFFSIZE+1];
int index[MAX_ARRAY_DEPTH];
int depth,fd,end;

if (line->num_tokens < start + 2) return ERR_SYNTAX;

one = (com == COM_CINPUT);
fd = STDIN;
stream = STDIN;

for(++start;start < line->num_tokens;start = end+1) {
	/* Check for specific stream then variable */
	switch(line->tokens[start]->type) {
		case TYPE_HASH:
		/* See if we've been passed a stream */
		if ((err = getStream(line,start+1,&end,&stream)) != OK) 
			return err;

		if (!STREAM_IS_OPEN((int)stream)) return ERR_STREAM_NOT_OPEN;

		if (streams[(int)stream].type == STREAM_WRITE ||
		    (one && streams[(int)stream].type == STREAM_DIR))
			return ERR_INVALID_STREAM_TYPE;

		if (streams[(int)stream].eof) return ERR_EOF;

		fd = streams[(int)stream].fd;
		continue;

		case TYPE_VAR:
		case TYPE_STRVAR:
		depth = 0;
		end = start + 1;
		break;

		case TYPE_ARR_VAR:
		case TYPE_ARR_STRVAR:
		if ((err = getArrIndex(line,start+2,&end,index,&depth)) != OK)
			return err;
		++end;
		break;

		default: return ERR_SYNTAX;
		}
	if (end < line->num_tokens && line->tokens[end]->type != TYPE_COMMA)
		return ERR_SYNTAX;

	/* Read data off stream */
	if (streams[(int)stream].type == STREAM_DIR) {
		if ((ds = readdir(streams[(int)stream].dfp)))
			strcpy(input,ds->d_name);	
		else {
			streams[(int)stream].eof = 1;  return OK;
			}
		}
	else
	if ((err = getInput(fd,input,one)) != OK) {
		if (err == ERR_EOF) {
			streams[(int)stream].eof = 1;  return OK;
			}
		return err;
		}

	/* Put data in variable */
	switch(line->tokens[start]->type) {
		case TYPE_VAR:
		case TYPE_ARR_VAR:
		/* Check its numeric */
		if (!isNumeric(input)) return ERR_NON_NUMERIC_DATA;
		err = setVarValue(
			line->tokens[start]->text,index,depth,atof(input),NULL);
		break;

		case TYPE_STRVAR:
		case TYPE_ARR_STRVAR:
		err = setVarValue(line->tokens[start]->text,index,depth,0,input);
		}
	if (err != OK) return err;
	}
return OK;
}




/*** OPEN: Open a disk file to read/write/append ***/
enum en_error comOpen(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
enum en_type result_type;
enum en_stream_type st;
double result,stream;
char *filename;
int fd,flags;
int end;
mode_t perm;

if (line->num_tokens <= start + 5) return ERR_SYNTAX;

/* Get filename */
if ((err = evalStringExpr(
	line,start+1,&end,&result_type,&result,&filename)) != OK) return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (end >= line->num_tokens - 4 ||
    line->tokens[end++]->com != COM_TO) goto SYNERR;

/* Check type of open */
switch(line->tokens[end]->com) {
	case COM_READ:
	st = STREAM_READ;
	flags = O_RDONLY;	
	break;

	case COM_WRITE:
	st = STREAM_WRITE;
	flags = O_WRONLY | O_CREAT | O_TRUNC;
	break;

	case COM_APPEND: 
	st = STREAM_WRITE;
	flags = O_WRONLY | O_APPEND | O_CREAT;
	break;

	case COM_READWRITE:
	st = STREAM_READWRITE;
	flags = O_RDWR;
	break;

	default: goto SYNERR;
	}

if (line->tokens[++end]->com != COM_AS ||
    line->tokens[++end]->type != TYPE_HASH) goto SYNERR;

/* Get stream number */
if ((err = getStream(line,end+1,&end,&stream)) != OK) {
	free(filename);  return err;
	}

/* Check for optional file permissions */
if (end < line->num_tokens) {
	if (line->tokens[end]->type != TYPE_COMMA) goto SYNERR;

	/* Get permissions */
	if ((err = evalNumExpr(line,end+1,&end,&result)) != OK) {
		free(filename);  return err;
		}
	if (end < line->num_tokens) goto SYNERR;

	if ((err = calcPermission(result,&perm)) != OK) {
		free(filename);  return err;
		}
	}
else perm = 0644;

if (STREAM_IS_OPEN((int)stream)) {
	free(filename);  return ERR_STREAM_ALREADY_OPEN;
	}

/* Everything ok so far , lets try and open the file */
if ((fd = open(filename,flags,perm)) == -1) {
	stored_errno = errno;
	err= ERR_CANT_OPEN_FILE;
	}
else {
	/* File opened. Set up stream */
	streams[(int)stream].type = st;
	streams[(int)stream].fd = fd;
	streams[(int)stream].eof = 0;
	err = OK;
	}
free(filename);
return err;

SYNERR:
free(filename);
return ERR_SYNTAX;
}




/*** CLOSE: Close a disk file ***/
enum en_error comClose(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double stream;
int end,strm;

if (line->num_tokens <= start + 2 ||
    line->tokens[start+1]->type != TYPE_HASH) return ERR_SYNTAX;

/* Get stream */
if ((err = getStream(line,start+2,&end,&stream)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;

strm = (int)stream;

if (!STREAM_IS_OPEN(strm)) return ERR_STREAM_NOT_OPEN;

if (streams[strm].type == STREAM_DIR) {
	closedir(streams[strm].dfp);
	streams[strm].dfp = NULL;
	return OK;
	}

/* Can't close stdin or stdout */
if (stream <= STDERR) return ERR_CANT_CLOSE_STREAM;

close(streams[strm].fd);
streams[strm].fd = -1;

return OK;
}




/*** SEEK/SEEKSTART: Seek from the current position on an open stream ***/
enum en_error comSeek(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double stream;
double offset;
int end,strm;

if (line->num_tokens <= start + 4 ||
    line->tokens[start+1]->type != TYPE_HASH) return ERR_SYNTAX;

/* Get stream */
if ((err = getStream(line,start+2,&end,&stream)) != OK) return err;
if (end >= line->num_tokens) return ERR_SYNTAX;

strm = (int)stream;
if (!STREAM_IS_OPEN(strm)) return ERR_STREAM_NOT_OPEN;

/* Get offset to seek */
if (line->tokens[end]->type != TYPE_COMMA) return ERR_SYNTAX;
if ((err = evalNumExpr(line,end+1,&end,&offset)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;

if (streams[strm].type == STREAM_DIR) {
	/* Because of the hopeless limitations of seekdir() (it can only
	   use values from telldir() so we'd have to store these values for
	   every position!) all we can realistically do is just rewind to
	   the beginning */
	if (com == COM_SEEK || offset) return ERR_CANT_SEEK_ON_STREAM;
	rewinddir(streams[strm].dfp);
	return OK;
	}
if (lseek(
	streams[strm].fd,
	(int)offset, com == COM_SEEK ? SEEK_CUR : SEEK_SET) == -1) {
	stored_errno = errno;
	return ERR_CANT_SEEK_ON_STREAM;
	}
return OK;
}




/*** REMOVE: Remove/delete a file or directory ***/
enum en_error comRemove(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
enum en_type result_type;
double result;
char *filename;
int end;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;

if ((err = evalStringExpr(
	line,start+1,&end,&result_type,&result,&filename)) != OK) return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (end < line->num_tokens) {
	free(filename);  return ERR_SYNTAX;
	}
if (remove(filename) == -1) {
	stored_errno = errno;  
	err = ERR_CANT_DELETE_FILE;
	}
else err = OK;

free(filename);
return err;
}




/*** RENAME: Rename a file or directory ***/
enum en_error comRename(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
enum en_type result_type;
double result;
char *fromname,*toname;
int end;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;

toname = NULL;

if ((err = evalStringExpr(
	line,start+1,&end,&result_type,&result,&fromname)) != OK) return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (end > line->num_tokens - 2 || line->tokens[end]->type != TYPE_COMMA) {
	err = ERR_SYNTAX;  goto ERROR;
	}

if ((err = evalStringExpr(
	line,end+1,&end,&result_type,&result,&toname)) != OK) goto ERROR;

if (result_type != TYPE_STRING) {
	err = ERR_INVALID_ARGUMENT;  goto ERROR;
	}
if (end < line->num_tokens) {
	err = ERR_SYNTAX;  goto ERROR;
	}

if (rename(fromname,toname) == -1) {
	stored_errno = errno;
	err = ERR_CANT_RENAME_FILE;
	}
else err = OK;

ERROR:
free(fromname);
FREE(toname);
return err;
}




/*** MKDIR/CHMOD/CD: Create a directory or change the permissions on a
     file or change directory ***/
enum en_error comMkdirChmodCd(
	enum en_command com, struct st_line *line, int start)
{
enum en_error err;
enum en_type result_type;
double result;
char *filename;
int end;
mode_t perm;

if (line->num_tokens <= start + 1) return ERR_SYNTAX;

/* Get filename */
if ((err = evalStringExpr(
	line,start+1,&end,&result_type,&result,&filename)) != OK) return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (end == line->num_tokens) {
	if (com == COM_CHMOD) goto SYNERR;
	perm = 0755;
	}
else {
	if (com == COM_CD) goto SYNERR;

	if (end > line->num_tokens - 2 ||
	    line->tokens[end]->type != TYPE_COMMA) goto SYNERR;

	/* Get permissions */
	if ((err = evalNumExpr(line,end+1,&end,&result)) != OK) {
		free(filename);  return err;
		}
	if (end < line->num_tokens) goto SYNERR;

	if ((err = calcPermission(result,&perm)) != OK) {
		free(filename);  return err;
		}
	}

err = OK;

switch(com) {
	case COM_MKDIR:
	if (mkdir(filename,perm) == -1) {
		stored_errno = errno;  
		err = ERR_CANT_CREATE_DIR;
		}
	break;

	case COM_CHMOD:
	if (chmod(filename,perm) == -1) {
		stored_errno = errno;
		err = ERR_CANT_CHANGE_PERM;
		}
	break;

	case COM_CD:
	if (chdir(filename) == -1) {
		stored_errno = errno;
		err = ERR_CANT_CHANGE_DIR;
		}
	break;

	default:
	err = ERR_INTERNAL;
	}

free(filename);
return err;

SYNERR:
free(filename);
return ERR_SYNTAX;
}




/*** OPENDIR: Open a directory to read a list if its contents ***/
enum en_error comOpendir(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
enum en_type result_type;
double result,stream;
char *dirname;
int end;

if (line->num_tokens <= start + 3) return ERR_SYNTAX;

/* Get dirname */
if ((err = evalStringExpr(
	line,start+1,&end,&result_type,&result,&dirname)) != OK) return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (end >= line->num_tokens - 2 ||
    line->tokens[end++]->com != COM_AS ||
    line->tokens[end++]->type != TYPE_HASH) {
	free(dirname);  return ERR_SYNTAX;
	}

/* Get stream number */
if ((err = getStream(line,end,&end,&stream)) != OK) {
	free(dirname);  return err;
	}
if (STREAM_IS_OPEN((int)stream)) {
	free(dirname);  return ERR_STREAM_ALREADY_OPEN;
	}

/* Open the dir */
if (!(streams[(int)stream].dfp = opendir(dirname))) {
	stored_errno = errno;
	err = ERR_CANT_OPEN_FILE;
	}
else {
	streams[(int)stream].type = STREAM_DIR;
	streams[(int)stream].eof = 0;
	err = OK;
	}

free(dirname);
return err;
}

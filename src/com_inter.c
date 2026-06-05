/****************************************************************************
 FILE: com_inter.c
 LVU : 1.2.1

 DESC:
 This contains the BASIC commands that are generally used interactively as
 opposed to being in a program. Eg: RUN

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"



/*** LIST: List out the program to the screen ***/
enum en_error comList(enum en_command com, struct st_line *listline, int start)
{
enum en_error err;
int start_line;
int end_line;

/* Get from and to line numbers if any */
if ((err = getLineNumbers(listline,start,&start_line,&end_line)) != OK)
	return err;

listProgram(stdout,start_line,end_line);
return OK;
}




/*** RUN: Run the program ***/
enum en_error comRun(enum en_command com, struct st_line *line, int start)
{
enum en_error err;

if (program_running) return ERR_INTERACTIVE_COMMAND;

if (line->num_tokens > 1) return ERR_SYNTAX;
if (!first_basline) return OK;

/* Delete all variables and also reset fordata */
deleteAllVariables();
resetLines(first_basline->first_line);
closeStreams();

error_action = COM_BREAK;
program_running = 1;
pen = 0;
paper = 0;
style = 0;

last_error = err = runProgram(first_basline->first_line);

program_running = 0;
return err;
}




/*** CLEAR: Just calls deleteAllVariables() and resets pen/paper etc ***/
enum en_error comClear(enum en_command com, struct st_line *line, int start)
{
if (program_running) return ERR_INTERACTIVE_COMMAND;
if (line->num_tokens > start + 1) return ERR_SYNTAX;

deleteAllVariables();

pen = 0;
paper = 0;
style = 0;
echo_on = 1;

puts(PROMPT2);

return OK;
}




/*** NEW: Clear out everything including all the tokens ***/
enum en_error comNew(enum en_command com, struct st_line *line, int start)
{
if (program_running) return ERR_INTERACTIVE_COMMAND;
if (line->num_tokens > start + 1) return ERR_SYNTAX;
reset();
puts(PROMPT1);
return OK;
}




/*** DELETE: Delete the selected lines from the program ***/
enum en_error comDelete(enum en_command com, struct st_line *line, int start)
{
struct st_bas_line *basline;
enum en_error err;
int start_line;
int end_line;
int cnt;

if (program_running) return ERR_INTERACTIVE_COMMAND;

if (line->num_tokens < start + 2) return ERR_SYNTAX;

/* Get from and to line numbers if any */
if ((err = getLineNumbers(line,start,&start_line,&end_line)) != OK)
	return err;

/* Go through the program lines and delete the appropriate ones */
for(basline=first_basline,cnt=0;basline;basline=basline->next) {
	if (end_line) {
		/* Delete range */
		if (basline->line_number >= start_line &&
		    basline->line_number <= end_line)
		deleteProgramLine(basline);
		++cnt;
		}
	else 
	if (basline->line_number == start_line) {
		/* If only start line given the its a specific line so just
		   delete this and quit */
		deleteProgramLine(basline);
		++cnt;
		break;
		}
	}
if (!cnt) return ERR_NO_SUCH_LINE_NUMBER;

/* Go through all lines and reset goto and data read lines in case they pointed
   to any lines we just deleted */
if (first_basline) resetLines(first_basline->first_line);

puts(PROMPT2);

return OK;
}




/*** LOAD: Clear everything and load a program ***/
enum en_error comLoad(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
enum en_type result_type;
double result;
char *filename;
int end;

if (program_running) return ERR_INTERACTIVE_COMMAND;
if (line->num_tokens <= start + 1) return ERR_SYNTAX;

/* Get file to save to */
if ((err = evalStringExpr(
	line,start+1,&end,&result_type,&result,&filename)) != OK) return err;

if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

if (end < line->num_tokens) {
	free(filename);  return ERR_SYNTAX;
	}

err = loadProgram(filename);
free(filename);
if (err == OK) puts(PROMPT1);
return err;
}




/*** SAVE: Write program out to text file ***/
enum en_error comSave(enum en_command com, struct st_line *line, int start)
{
FILE *fp;
enum en_error err;
enum en_type result_type;
double result;
char *filename;
int end;

if (program_running) return ERR_INTERACTIVE_COMMAND;
if (line->num_tokens <= start + 1) return ERR_SYNTAX;

if (!first_basline) return ERR_NO_PROGRAM;

/* Get file to save to */
if ((err = evalStringExpr(
	line,start+1,&end,&result_type,&result,&filename)) != OK) return err;
if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;
if (end < line->num_tokens) {
	free(filename);  return ERR_SYNTAX;
	}

/* Open it and save */
if (!(fp = fopen(filename,"w"))) {
	free(filename);
	stored_errno = errno;
	return ERR_CANT_OPEN_FILE;
	}

listProgram(fp,0,0);

fclose(fp);
free(filename);
puts("SAVED");
return OK;
}




/*** RENUM: Renumber the whole program in increments of 10s ***/
enum en_error comRenum(enum en_command com, struct st_line *line, int start)
{
struct st_bas_line *basline;
struct st_line *line2;
struct st_token *token1,*token2;
enum en_error err;
uint32_t linenum,prev_linenum;
double lineinc;
int end,lpos;
char newline[SHORT_STR];
char oldline[SHORT_STR];

if (program_running) return ERR_INTERACTIVE_COMMAND;

if (line->num_tokens > start + 1) {
	if ((err = evalNumExpr(line,start+1,&end,&lineinc)) != OK) return err;
	if (end < line->num_tokens) return ERR_SYNTAX;
	if (lineinc < 1) return ERR_OUT_OF_BOUNDS;
	}
else lineinc = 10;

if (!first_basline) {
	puts(PROMPT2);  return OK;
	}

for(line2=first_basline->first_line;line2;line2=line2->next) line2->renum = 0;

/* Go through main BASIC lines */
for(linenum=lineinc,basline=first_basline,prev_linenum=0;basline;
    prev_linenum=linenum,linenum += lineinc,basline=basline->next) {
	if (linenum < prev_linenum) return ERR_INVALID_LINE_NUMBER;
	basline->line_number = linenum;	

	token1 = basline->first_line->tokens[0];
	strcpy(oldline,token1->text);
	free(token1->text);
	sprintf(newline,"%u",linenum);
	token1->text = strdup(newline);

	/* Now find gotos/gosubs etc that have the old line number hard
	   coded (can't do anything if its an expression) and set to the new
	   one. Eg: can change "GOTO 10" but not "GOTO a+10" */
	for(line2=first_basline->first_line;line2;line2=line2->next) {
		if (line2->renum) continue;
		start = IS_START_LINE(line2);

		token1 = line2->tokens[start];
		switch(token1->com) {
			case COM_GOTO:
			case COM_GOSUB:
			case COM_RESTORE:
			lpos = start + 1;
			break;

			case COM_ON:
			if (line2->num_tokens < start + 4) continue;
			lpos = start + 3;
			break;

			default: continue;
			}

		/* Check to see we have a simple int following GOTO etc, not an 
		   expression */
		if (line2->num_tokens != lpos + 1 ||
		    line2->tokens[lpos]->type != TYPE_INT) {
			printf("Cannot renumber '%s' argument on line %u\n",
				line2->tokens[lpos]->text,
				line2->parent->line_number);
			line2->renum = 1;
			continue;
			}

		token2 = line2->tokens[lpos];

		/* Do the renumber. Involves changing token data */
		if (token2->type == TYPE_INT && !strcmp(token2->text,oldline)) {
			free(token2->text);
			token2->text = strdup(newline);
			token2->text_len = strlen(newline);
			token2->value = (double)linenum;
			line2->renum = 1;
			if (token1->com != COM_RESTORE) 
				line2->goto_line_number = linenum;
			}
		}
	}
puts(PROMPT2);
return OK;
}




/*** DIR: The interactive dir command ***/
enum en_error comDir(enum en_command com, struct st_line *line, int start)
{
DIR *dir;
struct dirent *ds;
struct stat fs;
struct passwd *pwd;
struct group *grp;
enum en_error err;
enum en_type result_type;
double result;
char *dirname,*ftype;
char path[LONG_STR];
char udata[SHORT_STR];
char gdata[SHORT_STR];
int end,total;

if (line->num_tokens > start + 1) {
	/* Get dirname */
	if ((err = evalStringExpr(
		line,
		start+1,&end,&result_type,&result,&dirname)) != OK) return err;

	if (result_type != TYPE_STRING) return ERR_INVALID_ARGUMENT;

	if (end < line->num_tokens) {
		free(dirname);  return ERR_SYNTAX;
		}
	}
else dirname = strdup(".");

if (!(dir = opendir(dirname))) {
	stored_errno = errno;
	free(dirname);
	return ERR_CANT_OPEN_FILE;
	}

puts("\nType    Perm  User      Group         Bytes  Name");
puts("------  ----  --------  --------  ---------  ----");
for(total=0;(ds = readdir(dir));++total) {
	snprintf(path,LONG_STR-1,"%s/%s",dirname,ds->d_name);
	lstat(path,&fs);

	/* Using the same names as in the stat() functions */
	switch(fs.st_mode & S_IFMT) {
		case S_IFREG : ftype = "FILE  ";  break;
		case S_IFDIR : ftype = "DIR   ";  break;
		case S_IFLNK : ftype = "LINK  ";  break;
		case S_IFIFO : ftype = "FIFO  ";  break;
		case S_IFBLK : ftype = "BLOCK ";  break;
		case S_IFCHR : ftype = "CHAR  ";  break;
		case S_IFSOCK: ftype = "SOCKET";  break;

		default: ftype = "?     ";
		}

	/* Get user & group */
	if ((pwd = getpwuid(fs.st_uid)))
		strcpy(udata,pwd->pw_name);
	else
		sprintf(udata,"%u",(u_int)fs.st_uid);

	if ((grp = getgrgid(fs.st_gid)))
		strcpy(gdata,grp->gr_name);
	else
		sprintf(gdata,"%u",(u_int)fs.st_gid);


	printf("%s  %04o  %-8s  %-8s  %9u  %s\n",
		ftype,
		fs.st_mode & 0xFFF,
		udata,gdata,
		(u_int)fs.st_size,
		ds->d_name);
	}
closedir(dir);
free(dirname);

printf("\nTotal of %d objects.\n\n",total);
return OK;
}




/*** INDENT: Sets the amount of indenting ***/
enum en_error comIndent(enum en_command com, struct st_line *line, int start)
{
enum en_error err;
double idc;
int end;

if (start + 2 > line->num_tokens) return ERR_SYNTAX;

if ((err = evalNumExpr(line,start+1,&end,&idc)) != OK) return err;
if (end < line->num_tokens) return ERR_SYNTAX;

if (idc < 0) return ERR_INVALID_ARGUMENT;
indent_chars = (int)idc;
return OK;
}




/*** PAGING: Sets paging during a LIST , on or off ***/
enum en_error comPaging(enum en_command com, struct st_line *line, int start)
{
if (line->num_tokens != start + 2) return ERR_SYNTAX;

switch(line->tokens[start+1]->com) {
	case COM_ON:  paging_on = 1; break;
	case COM_OFF: paging_on = 0; break;

	default: return ERR_SYNTAX;
	}
return OK;
}


/****************************************************************************
 FILE: misc.c
 LVU : 1.2.1

 DESC:
 For generic functions or stuff that doesn't really fit anywhere else.

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#include "basic.h"


/*** Get input from stdin ***/
enum en_error getInput(int fd, char *input, char one)
{
fd_set mask;
char c;
int i,cnt;

for(i=0,c=0,cnt=0;i <= BUFFSIZE;) {
	FD_ZERO(&mask);
	FD_SET(fd,&mask);

	/* Have to use select() since getchar() won't exit on ^C if SIGINT is
	   handled so we won't be able to exit on user interrupt */
	switch(select(FD_SETSIZE,&mask,0,0,0)) {
		case -1:
		case 0:
		if (errno == EINTR) return ERR_INTERRUPT;
		continue;
		}
	switch(read(fd,&c,1)) {
		case -1:
		stored_errno = errno;
		return ERR_READ_FAILURE;

		case 0: return ERR_EOF;
		}

	/* Canonical mode & echo is off on stdin so we have to process
	   everything ourself */
	if (fd == STDIN) {
		/* Deal with delete keys */
		if (!one && (c == DEL1 || c == DEL2)) {
			if (i) {
				if (echo_on) {
					printf("\b \b");
					fflush(stdout);
					}
				--i;
				--cnt;
				}
			continue;
			}
		if (echo_on) {
			putchar(c);
			fflush(stdout);
			}
		}
		
	if (!one && c == '\n') break;
	input[i] = c;
	++i;
	if (one) break; 
	}
if (i <= BUFFSIZE) {
	input[i] = '\0';  return OK;
	}

/* Too long. Clear out reset of stdin unless last char was \n */
if (c != '\n') {
	while (read(fd,&c,1) > 0 && c != '\n');
	}
return ERR_LINE_TOO_LONG;
}




/*** Convert a string to upper case ***/
char *toupperString(char *s)
{
for(;*s;++s) *s = toupper(*s);
return s;
}




/*** Append one string to another ***/
void appendString(char **result, char *str)
{
if (!str) return;

if (!*result) {
	*result = (char *)malloc(strlen(str)+1);
	strcpy(*result,str);
	return;
	}
*result = (char *)realloc(*result,strlen(*result) + strlen(str) + 1);
strcat(*result,str);
}




/*** Matches a right bracket with a the left one we're starting at ***/
int findRightBracket(struct st_line *line, int start)
{
int i,cnt;

if (line->tokens[start]->type != TYPE_LBRACKET) return -1;
for(i=start,cnt=0;i < line->num_tokens;++i) {
	cnt = cnt + (line->tokens[i]->type == TYPE_LBRACKET) -
		    (line->tokens[i]->type == TYPE_RBRACKET);
	if (!cnt) return i;
	}
return -1;
}




/*** Used by comList and comDelete. Parses "<line> TO <line>" ***/
enum en_error getLineNumbers(
	struct st_line *line, int start, int *start_line, int *end_line)
{
enum en_error err;
double result;
int end;

*start_line = 0;
*end_line = 0;
start++;

/* Get first line number */
if (start < line->num_tokens) {
	if ((err = evalNumExpr(line,start,&end,&result)) != OK) return err;
	if (result > (int)result) return ERR_INVALID_LINE_NUMBER;
	*start_line = (int)result;

	/* Get second line number */
	if (end < line->num_tokens) {
		if (line->tokens[end]->com != COM_TO) return ERR_SYNTAX;
		if (++end >= line->num_tokens) return ERR_SYNTAX;
		if ((err = evalNumExpr(line,end,&end,&result)) != OK)
			return err;
		if (result > (int)result) return ERR_INVALID_LINE_NUMBER;
		*end_line = (int)result;

		if (*start_line > *end_line) return ERR_INVALID_ARGUMENT;
		}
	}
return OK;
}




/*** Used by loop header commands to find and set the end of the loop ***/
char setLoopEnd(
	struct st_line *line,
	enum en_command start_com, 
	enum en_command end_com)
{
struct st_line *line2;
int nesting,st;

for(line2=line->next,nesting=0;line2;line2=line2->next) {
	st = IS_START_LINE(line2);
	if (line2->tokens[st]->com == start_com) nesting++;
	else
	if (line2->tokens[st]->com == end_com) {
		if (!nesting--) {
			line->goto_line = line2->next;
			line->paired = 1;
			break;
			}
		}
	}
if (!line2) return 0;
line2->goto_line = line;
line2->paired = 1;
return 1;
}




/*** Returns 1 if its a valid number ***/
int isNumeric(char *str)
{
char dot;
char *s;

if (*str == '-') ++str;
if (!*str) return 0;

for(dot=0,s=str;*s;++s) {
	if (*s == '.') {
		if (dot) return 0;
		dot = 1;
		}
	else 
	if (!isdigit(*s)) return 0;
	}
/* Check it wasn't just a dot on its own */
return !(dot && s - str < 2);
}




/*** Print the error messages ***/
void printError(enum en_error err, int line)
{
if (stored_errno) {
	if (line) 
		printf("ERROR %d: %s (%s) on line %d\n",
			err,error[err],strerror(stored_errno),line);
	else
		printf("ERROR %d: %s (%s)\n",err,error[err],strerror(stored_errno));
	return;
	}
if (line) 
	printf("ERROR %d: %s on line %d\n",err,error[err],line);
else
	printf("ERROR %d: %s\n",err,error[err]);

last_error = err;
}




/*** Do a complete reset ***/
void reset()
{
struct st_bas_line *basline,*next;

/* Clear variables */
deleteAllVariables();

/* Clear program */
for(basline = first_basline;basline;) {
	next = basline->next;
	deleteBasicLine(basline);
	basline = next;
	}
closeStreams();
init();
}




/*** Close an open streams and reset file pointers ***/
void closeStreams()
{
int i;

for(i=STDERR+1;i < NUM_STREAMS;++i){
	if (streams[i].fd != -1) {
		close(streams[i].fd);
		streams[i].fd = -1;
		}
	else
	if (streams[i].dfp) {
		closedir(streams[i].dfp);
		streams[i].dfp = NULL;
		}
	}
}




/*** Get a stream value and check if its valid ***/
enum en_error getStream(
	struct st_line *line, int start, int *end, double *stream)
{
enum en_error err;

if ((err = evalNumExpr(line,start,end,stream)) != OK)
	return err;
if (*stream < 1 || *stream > NUM_STREAMS) return ERR_INVALID_STREAM;
--*stream;
return OK;
}




/*** Set tthe text foreground and background colours & text style ***/
enum en_error setColours(int fd)
{
enum en_error err;

if (pen && (err = fdWrite(fd,fgcols[pen-1],5)) != OK) return err;
if (paper && (err = fdWrite(fd,bgcols[paper-1],5)) != OK) return err;
if (style) return fdWrite(fd,text_styles[style-1],4);
return OK;
}




/*** List the program so whereever the file pointer points ***/
void listProgram(FILE *fp, int start_line, int end_line)
{
struct winsize *ws;
struct st_bas_line *basline;
struct st_line *line;
struct st_token *token;
enum en_type prev_type;
int i,j,k;
int start,indent;
int linecnt,pagelines;
char then_else,c;

indent = 0;

/* Get screen height for paging. If we can't get it default to 24 */
ws = getWinSize();
pagelines = (ws->ws_row ? ws->ws_row - 1 : 24);

/* Go through the program lines */
for(basline=first_basline,linecnt=0;basline;basline=basline->next) {
	if (basline->line_number < start_line) continue;
	if (end_line && basline->line_number > end_line) break;

	/* Print the line number */
	fprintf(fp,"%5u ",basline->line_number);

	/* Go through the sub lines */
	for(line=basline->first_line;
	    line && line->parent == basline;line=line->next) {

		/* Go through tokens */
		for(i=IS_START_LINE(line),start=1,then_else=0,prev_type=TYPE_NOTSET;
		    i < line->num_tokens;++i,start=0) {
			token = line->tokens[i];

			/* Count indents */
			if (indent_chars) {
				switch(token->com) {
					case COM_UNTIL:
					case COM_NEXT:
					case COM_WEND:
					case COM_FI:
					case COM_ELSE:
					case COM_CASE:
					case COM_DEFAULT:
					if (indent) --indent;
					break;

					case COM_CHOSEN:
					if (indent > 1) indent -= 2;
					}

				if (start && line == basline->first_line) {
					for(j=0;j < indent;++j) {
						for(k=0;k < indent_chars;++k)
							fputc(' ',fp);
						}
					}

				switch(token->com) {
					case COM_DO:
					case COM_FOR:
					case COM_WHILE:
					case COM_IF:
					case COM_ELSE:
					case COM_CASE:
					case COM_DEFAULT:
					indent++;  break;

					case COM_CHOOSE:
					indent += 2;
					}
				}

			if (token->negated) fputc('-',fp);

			switch(token->type) {
				case TYPE_STRING:
				fprintf(fp,"\"%s\"",token->text);
				break;

				case TYPE_COM:
				if (i > 1 && prev_type != TYPE_COM)
					fputc(' ',fp);
				if (i == line->num_tokens - 1)
					/* Don't follow with space if we're
					   at the end of a line */
					fputs(token->text,fp);
				else
					fprintf(fp,"%s ",token->text);
				break;

				case TYPE_AND:
				case TYPE_OR:
				case TYPE_XOR:
				case TYPE_NOT:
				if (i > 1 && 
				    prev_type != TYPE_COM &&
				    (prev_type < TYPE_EQUALS ||
				     prev_type > TYPE_LESS)) fputc(' ',fp);

				if (i == line->num_tokens - 1)
					fputs(token->text,fp);
				else
					fprintf(fp,"%s ",token->text);
				break;

				case TYPE_GRT:
				case TYPE_LESS:
				case TYPE_EQUALS:
				case TYPE_NOT_EQUALS:
				case TYPE_GRT_EQUALS:
				case TYPE_LESS_EQUALS:
				fprintf(fp," %s ",token->text);
				break;

				default:
				fputs(token->text,fp);
				}

			then_else = (token->com == COM_THEN ||
			             token->com == COM_ELSE);

			prev_type = token->type;
			}
		if (line->next &&
		    line->next->parent == line->parent) {
			/* Don't follow THEN or ELSE with a colon nor put one
			   before an ELSE or FI. ie pretend to user they're
			   not seperate command types but part of IF. */
			if (then_else) fputc(' ',fp);
			else {
				if (line->next->tokens[0]->com == COM_ELSE ||
				    line->next->tokens[0]->com == COM_FI)
					fputc(' ',fp);
				else
					fprintf(fp,": ");
				}
			}
		}
	fputc('\n',fp);
	if (!end_line && basline->line_number == start_line) return;
	if (paging_on && fp == stdout && ++linecnt >= pagelines) {
		write(STDOUT,"--- MORE ---",12);
		c = getchar();
		/* Erase MORE line above */
		write(STDOUT,"\r            \r",14);

		if (c == 'q' || c == 'Q') return;
		linecnt = 0;
		}
	}
}




/*** Write out to the file description ***/
enum en_error fdWrite(int fd, char *str, int len)
{
if (write(fd,str,len) == -1) {
	stored_errno = errno;
	return ERR_WRITE_FAILURE;
	}
return OK;
}




/*** Return a mode_t file permission given a decimal ***/
enum en_error calcPermission(double result, mode_t *perm)
{
int u,g,w,s;

if (result < 0 || result > 7777) return ERR_INVALID_ARGUMENT;

/* Convert to octal */
u = (int)result % 10;
g = ((int)result % 100) / 10;
w = ((int)result % 1000) / 100;
s = ((int)result % 10000) / 1000;

if (u > 7 || g > 7 || w > 7 || s > 7) return ERR_INVALID_ARGUMENT;

*perm = (mode_t)(u | g << 3 | w << 6 | s << 9);
return OK;
}




/*** Get terminal/window size ***/
struct winsize *getWinSize()
{
static struct winsize ws;

bzero(&ws,sizeof(ws));
#ifdef TIOCGWINSZ
ioctl(1,TIOCGWINSZ,&ws);
#endif
return &ws;
}




/*** Does wildcard pattern match (not regular expressions!) ***/
int wildmatch(char *str, char *pat)
{
char *s,*p,*s2;

/* Loop through string and pattern */
for(s=str,p=pat;*s && *p;++s,++p) {
	switch(*p) {
		case '?': continue;

		case '*':
		/* If star is last character in pattern then it'll match
		   everything left in the string */
		if (!*(p+1)) return 1;

		/* Attempt to match from next positions of s and p onwards */
		for(s2=s;*s2;++s2) if (wildmatch(s2,p+1)) return 1;
		return 0;
		}
	if (*s != *p) return 0;
	}

/* Only match If have reached the ends at the same time */
return (!*s && !*p);
}




/*** Do a sort of a numeric list of values using an insertion sort ***/
void numericSort(int num, double *data)
{
double val;
int i,j;

for(i=1;i < num;++i) {
	val = data[i];
	for(j=i;j && data[j-1] > val;--j) data[j] = data[j-1];
	data[j] = val;
	}
}

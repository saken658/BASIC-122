/****************************************************************************
 FILE: main.c
 LVU : 1.2.2

 DESC:
 All the magic starts here! Contains the main loop function as well as
 the init code and signal handler.

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/

#define MAINFILE
#include "basic.h"

void startup(void);
void parseCmdLine(int argc, char **argv);
void runFileAndExit(void);
void interactive(void);
void sighandler(int sig);


/*** Start here ***/
int main(int argc, char **argv)
{
startup();
init();
parseCmdLine(argc,argv);

/* If a filename is specified then run it and exit */
if (run_filename) runFileAndExit();

/* Go into interactive mode */
interactive();
return 0;
}




/*** Do startup stuff ***/
void startup(void)
{
struct termios tio;
struct passwd *pwd;
struct group *grp;

/* Once set these are never changed even by NEW */
basic_argc = 0;
basic_argv = NULL;

/* Disable canonical mode on stdin & set buffer size to 1 char */
tcgetattr(0,&stored_tio);
tio = stored_tio;
tio.c_lflag &= ~ICANON;
tio.c_lflag &= ~ECHO;
tio.c_cc[VTIME] = 0;
tio.c_cc[VMIN] = 1;
tcsetattr(0,TCSANOW,&tio);

/* Get username & group */
if ((pwd = getpwuid(getuid()))) {
	username = pwd->pw_name;
	if ((grp = getgrgid(pwd->pw_gid))) 
		usergroup = grp->gr_name;
	else
		usergroup = "<unknown>";
	}
else {
	username = "<unknown>";
	usergroup = "<unknown>";
	}

/* Set up signal handler */
signal(SIGINT,sighandler);
signal(SIGQUIT,sighandler);
}




/*** Initialise a few things ***/
void init(void)
{
int i;

bzero(varlist,sizeof(varlist));

first_basline = NULL;
last_basline = NULL;
goto_line = NULL;
data_line = NULL;
data_token_pos = 2;
stored_errno = 0;
last_error = OK;
error_action = COM_BREAK;
error_goto_line = NULL;


streams[0].type = STREAM_READ;
streams[0].fd = STDIN;
streams[0].dfp = NULL;
streams[0].eof = 0;
streams[1].type = STREAM_WRITE;
streams[1].fd = STDOUT;
streams[1].dfp = NULL;
streams[1].eof = 0;
streams[2].type = STREAM_WRITE;
streams[2].fd = STDERR;
streams[2].dfp = NULL;
streams[2].eof = 0;

for(i=STDERR+1;i < NUM_STREAMS;++i) {
	streams[i].type = STREAM_READ;
	streams[i].fd = -1;
	streams[i].dfp = NULL;
	streams[i].eof = 1;
	}

pen = 0;
paper = 0;
style = 0;

echo_on = 1;
cursor_on = 1;
paging_on = 1;
indent_chars = INDENT_CHARS;
}




/*** Load any command line options ***/
void parseCmdLine(int argc, char **argv)
{
const char *opt[] = {
	"nobreak",
	"run",
	"vers",
	"-"
	};
enum {
	OPT_NOBREAK,
	OPT_RUN,
	OPT_VERS,
	OPT_ARGS_FOLLOW,

	OPT_END
	};
int i,j,o;

break_on = 1;
run_filename = NULL;

for(i=1;i < argc;++i) {
	if (argv[i][0] != '-') goto USAGE;

	for(o=0;o != OPT_END;++o)
		if (!strcasecmp(opt[o],argv[i]+1)) break;

	switch(o) {
		case OPT_NOBREAK:
		break_on = 0;  break;

		case OPT_RUN:
		if (i == argc-1) goto USAGE;
		run_filename = argv[++i];
		break;

		case OPT_VERS:
		printf("Boltar BASIC version %s\n",VERSION);
		doExit(0);

		case OPT_ARGS_FOLLOW:
		/* Anything from now on is an argv argument */
		if (!(basic_argc = argc - i - 1)) break;
		basic_argv = (char **)malloc(basic_argc * sizeof(char *));
		for(j=i+1;j < argc;++j) 
			basic_argv[j-(i+1)] = strdup(argv[j]);
		return;

		default: goto USAGE;
		}
	}
return;

USAGE:
printf("Usage: %s [-run <filename>] [-nobreak] [-vers] [-- <arguments>]\n",
	argv[0]);
doExit(1);
}




/*** Run a program file given on the command line ***/
void runFileAndExit(void)
{
enum en_error err;

if ((err = loadProgram(run_filename)) != OK) {
	printError(err,0);  doExit(err);
	}
program_running = 1;
if ((err = runProgram(first_basline->first_line)) != OK) 
	printError(err,0); 
doExit(last_error);
}




/*** Main loop of the interactive part of the interpreter ***/
void interactive(void)
{
struct st_bas_line *basline;
struct st_line *line;
enum en_error err;
char input[BUFFSIZE+1];

printf("\nBoltar BASIC\n"
         "Copyright (C) Neil Robertson 2006\n\n"
         "Version: %s\n"
         "Built  : %s\n\n%s\n",VERSION,BUILD_DATE,PROMPT1);

while(1) {
	stored_errno = 0;

	/* Always have echo and cursor on at the interactive prompt */
	echo_on = 1;
	if (!cursor_on) {
		write(STDOUT,"\033[?25h",6);
		cursor_on = 1;
		}

	/* Read in the text off interpreter command line */
	write(1,">",1);
	switch ((err = getInput(STDIN,input,0))) {
		case OK: break;

		case ERR_INTERRUPT:
		if (break_on) puts("*** BREAK ***");
		continue;

		case ERR_EOF:
		puts("*** EXITING with code 0 ***");
		doExit(0);

		default:
		printError(err,0);
		continue;
		}

	/* Parse the line */
	if ((err = parseTextLine(input,&basline)) != OK) {
		deleteBasicLine(basline);
		printError(err,0);
		continue;
		}

	/* Ignore empty lines */
	if (!basline->first_line || !basline->first_line->num_tokens) {
		deleteBasicLine(basline);
		continue;
		}

	line = basline->first_line;

	/* Either run the line or put it into the program */
	switch(line->tokens[0]->type) {
		case TYPE_INT:
		if (line->num_tokens < 2) {
			printError(ERR_SYNTAX,0);
			break;
			}
		if (line->tokens[0]->negated) 
			printError(ERR_INVALID_LINE_NUMBER,0);
		else
			addProgramLine(basline);
		continue;

		case TYPE_FLOAT:
		printError(ERR_INVALID_LINE_NUMBER,0);
		continue;

		case TYPE_COM:
		case TYPE_VAR:
		case TYPE_STRVAR:
		case TYPE_ARR_VAR:
		case TYPE_ARR_STRVAR:
		stop_program = DONT_STOP;
		program_running = 0;
		gosub_stack_pos = 0;
		next_jump = 0;

		if ((err = runProgram(basline->first_line)) != OK)
			printError(err,0);
		break;

		default:
		printError(ERR_SYNTAX,0);
		}
	deleteBasicLine(basline);
	}
}




/*** Load a program from a file into memory ***/
enum en_error loadProgram(char *filename)
{
FILE *fp;
struct st_bas_line *basline;
enum en_error err;
char input[BUFFSIZE+1];
int len;

reset();

/* Open file */
if (!(fp = fopen(filename,"r"))){
	stored_errno = errno;
	return ERR_CANT_OPEN_FILE;
	}

fgets(input,BUFFSIZE,fp);
while(!feof(fp)) {
	if (!(len = strlen(input))) goto READ;
	if (input[len-1] == '\n') input[len-1] = '\0';

	/* Parse line read in */
	if ((err = parseTextLine(input,&basline)) != OK) {
		deleteBasicLine(basline);
		return ERR_SYNTAX_IN_FILE;
		}
	/* If its empty then get rid of it */
	if (!basline->first_line || !basline->first_line->num_tokens) {
		deleteBasicLine(basline);
		goto READ;
		}

	/* Check it has a line number */
	if (basline->first_line->tokens[0]->type != TYPE_INT ||
	    basline->first_line->tokens[0]->negated) {
		deleteBasicLine(basline);
		return ERR_INVALID_LINE_NUMBER;
		}
	
	/* Add line to the program */
	addProgramLine(basline);

	READ:
	fgets(input,BUFFSIZE,fp);
	}
fclose(fp);
return OK;
}




/*** Run the program starting at the given subline until we reach the end of
     the program. Remember the last subline on a basic line has its ->next
     pointer pointing to the first subline of the next basline ***/
enum en_error runProgram(struct st_line *line)
{
struct st_token *token;
enum en_error err;
int start;

goto_line_set = 0;
printed_break = 0;

for(;line;line = (goto_line_set ? goto_line : line->next)) {
	/* We want command functions to skip the line number at the start */
	start = IS_START_LINE(line);
	token = line->tokens[start];
	goto_line_set = 0;
	stored_errno = 0;

	switch(token->type) {
		case TYPE_COM:
		if (!comfunc[token->com]) return ERR_INTERNAL;

		err = (*comfunc[token->com])(token->com,line,start);
		/* Set program_running in case goto called from command line */
		if (goto_line_set) program_running = 1;
		break;

		case TYPE_VAR:
		case TYPE_STRVAR:
		case TYPE_ARR_VAR:
		case TYPE_ARR_STRVAR:
		/* If its a variable at the start of a line it must be an
		   assignment */
		err = (*comfunc[COM_LET])(COM_LET,line,start);
		break;

		default: err = ERR_SYNTAX;
		}

	switch(err) {
		case OK:
		break;

		case ERR_INTERRUPT:
		if (!break_on) break;
		last_error = err;
		goto PROG_BREAK;

		default:
		last_error = err;

		if (program_running) {
			/* See what we should do now theres an error */
			switch(error_action) {
				case COM_BREAK:
				printError(err,line->parent->line_number);
				return OK;

				case COM_CONTINUE: goto CONT;

				case COM_GOSUB:
				if (gosub_stack_pos >= GOSUB_STACK_SIZE) {
					printError(
						ERR_RECURSION_LIMIT_REACHED,
						line->parent->line_number);
					return OK;
					}
				gosub_stack[gosub_stack_pos++] = line->next;
				/* Fall through */
				
				case COM_GOTO:
				goto_line = error_goto_line;
				goto_line_set = 1;
				goto CONT;
				default: break;
				}
			}
		return err;
		}

	CONT:

	/* See if someone has breaked the program */
	switch(stop_program) {
		case BREAK_STOP: goto PROG_BREAK;

		case BREAK_END:
		if (!printed_break) {
			printf("*** END on line %d ***\n",
				line->parent->line_number);
			printed_break = 1;
			}
		return OK;
		}

	}
goto_line_set = 0;
return OK;

PROG_BREAK:
/* Could be printed twice because of recursion */
if (!printed_break) {
	if (line->parent->line_number)
		printf("*** BREAK in line %d ***\n",line->parent->line_number);
	else
		puts("*** BREAK ***");
	printed_break = 1;
	}
goto_line_set = 0;
return OK;
}




/*** Currently just sets stop_program on ***/
void sighandler(int sig)
{
if (break_on && program_running) stop_program = BREAK_STOP;
/* Reset for some unixes */
signal(sig,sighandler);
}




/*** Reset terminal and exit ***/
void doExit(int err)
{
tcsetattr(0,TCSANOW,&stored_tio);
exit(err);
}


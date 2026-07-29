/****************************************************************************
 FILE: basic.h:
 LVU : 1.2.2

 DESC:
 Main header files. Contains all the structures, types etc.

 Copyright (C) Neil Robertson 2006
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#ifndef SOLARIS
#include <stdint.h>
#endif
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "version.h"
#include "build_date.h"

#define BUFFSIZE 2000
#define LONG_STR BUFFSIZE 
#define SHORT_STR 500
#define NUM_COMMANDS 71
#define NUM_NUM_FUNCTIONS 46
#define NUM_STR_FUNCTIONS 21
#define NUM_ERRORS 65
#define NUM_SYSTEM_VARS 17
#define ALLOC_BLOCK 5
#define MAX_ARRAY_DEPTH 10 /* 10D should be enough for anyone */
#define MAX_ARRAY_SIZE 1000000000
#define EXPR_LIST_LEN 100
#define GOSUB_STACK_SIZE 100
#define NUM_STREAMS 10
#define NUM_FGCOLS 8
#define NUM_BGCOLS 8
#define NUM_STYLES 6
#define INDENT_CHARS 5

#define PROMPT1 "READY"
#define PROMPT2 "OK"

#define DEL1 8
#define DEL2 127

#define FREE(X) if (X) free(X)
#define IS_START_LINE(X) (X->tokens[0]->type == TYPE_INT)
#define STREAM_IS_OPEN(X) (streams[X].fd != -1 || streams[X].dfp)

#define DEGS_PER_RADIAN 57.29578

/*** Enums and arrays ***/

enum {
	STDIN,
	STDOUT,
	STDERR
	};

/* Token types. Operators are all seperate types for efficiency reasons,
   not logical reasons! Really there should just be a TYPE_OP then a 
   sub type with the op type but that would just make the tokenizer and
   expression parser code more complex for no gain. */
enum en_type {
	TYPE_COM,
	TYPE_PAD,
	TYPE_NUMFUNC,
	TYPE_STRFUNC,

	TYPE_VAR,
	TYPE_STRVAR,
	TYPE_ARR_VAR,
	TYPE_ARR_STRVAR,

	TYPE_REM_COMMENTS,

	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING,

	/* Char operators */
	TYPE_LBRACKET,
	TYPE_RBRACKET,
	TYPE_COMMA,
	TYPE_HASH,
	TYPE_AMPERSAND,
	TYPE_BAR,
	TYPE_EMARK,
	TYPE_COLON,
	TYPE_EQUALS,
	TYPE_NOT_EQUALS,
	TYPE_GRT_EQUALS,
	TYPE_LESS_EQUALS,
	TYPE_GRT,
	TYPE_LESS,
	TYPE_ADD,
	TYPE_SUB,
	TYPE_MUL,
	TYPE_DIV,
	TYPE_MOD,
	TYPE_PWR,
	TYPE_SHIFT_LEFT,
	TYPE_SHIFT_RIGHT,
	TYPE_QUESTION,

	/* Word operators */
	TYPE_AND,
	TYPE_OR,
	TYPE_XOR,
	TYPE_NOT,

	TYPE_NOTSET
	};

enum en_error {
	OK,
	ERR_INTERNAL,
	ERR_MALLOC,
	ERR_LINE_TOO_LONG,
	ERR_SYNTAX,
	ERR_SYNTAX_IN_FILE,
	ERR_MISSING_QUOTES,
	ERR_INVALID_ARGUMENT,
	ERR_INVALID_LINE_NUMBER,
	ERR_MISSING_BRACKET,
	ERR_VAR_IS_READ_ONLY,
	ERR_VAR_NOT_ARRAY,
	ERR_VAR_IS_ARRAY,
	ERR_UNDEFINED_VAR,
	ERR_UNDEFINED_ARRAY,
	ERR_MISSING_ARRAY_INDEX,
	ERR_INVALID_ARRAY_INDEX,
	ERR_INVALID_ARRAY_SIZE,
	ERR_INDEX_OUT_OF_BOUNDS,
	ERR_MAX_DIM_EXCEEDED,
	ERR_ARRAY_TOO_BIG,
	ERR_MISSING_FUNC_ARGUMENT,
	ERR_UNEXPECTED_FUNC_ARGUMENT,
	ERR_EXPR_TOO_COMPLEX,
	ERR_VAR_ALREADY_EXISTS,
	ERR_DIVISION_BY_ZERO,
	ERR_INTERACTIVE_COMMAND,
	ERR_NO_SUCH_LINE_NUMBER,
	ERR_RECURSION_LIMIT_REACHED,
	ERR_UNEXPECTED_RETURN,
	ERR_MISSING_ELSE_FI,
	ERR_UNEXPECTED_ELSE,
	ERR_UNEXPECTED_FI,
	ERR_MISSING_WEND,
	ERR_UNEXPECTED_WEND,
	ERR_MISSING_UNTIL,
	ERR_UNEXPECTED_UNTIL,
	ERR_MISSING_NEXT,
	ERR_UNEXPECTED_NEXT,
	ERR_NON_NUMERIC_DATA,
	ERR_NOT_DATA_LINE,
	ERR_END_OF_DATA,
	ERR_CANT_OPEN_FILE,
	ERR_NO_PROGRAM,
	ERR_STREAM_NOT_OPEN,
	ERR_STREAM_ALREADY_OPEN,
	ERR_INVALID_STREAM,
	ERR_INVALID_STREAM_TYPE,
	ERR_CANT_CLOSE_STREAM,
	ERR_CANT_SEEK_ON_STREAM,
	ERR_EOF,
	ERR_INVALID_FORMAT_STRING,
	ERR_OUT_OF_BOUNDS,
	ERR_READ_FAILURE,
	ERR_WRITE_FAILURE,
	ERR_CANT_DELETE_FILE,
	ERR_CANT_RENAME_FILE,
	ERR_CANT_CREATE_DIR,
	ERR_CANT_CHANGE_PERM,
	ERR_CANT_STAT_FILE,
	ERR_CANT_CHANGE_DIR,
	ERR_MISSING_CHOSEN,
	ERR_UNEXPECTED_CHOSEN,
	ERR_INVALID_CASE_TYPE,

	ERR_INTERRUPT
	};

enum en_command {
	/* 0 */
	COM_REM,
	COM_LIST,
	COM_RUN,
	COM_END,
	COM_EXIT,

	/* 5 */
	COM_CLEAR,
	COM_NEW,
	COM_DELETE,
	COM_DIM,
	COM_LET,

	/* 10 */
	COM_PRINT,
	COM_WRITE,
	COM_GOTO,
	COM_GOSUB,
	COM_RETURN,
	
	/* 15 */
	COM_WHILE,
	COM_WEND,
	COM_DO,
	COM_UNTIL,
	COM_FOR,

	/* 20 */
	COM_NEXT,
	COM_TO,
	COM_STEP,
	COM_THEN,
	COM_IF,

	/* 25 */
	COM_ELSE,
	COM_FI,
	COM_DATA,
	COM_READ,
	COM_RESTORE,

	/* 30 */
	COM_INPUT,
	COM_CINPUT,
	COM_LOAD,
	COM_SAVE,
	COM_RENUM,

	/* 35 */
	COM_PAUSE,
	COM_OPEN,
	COM_CLOSE,
	COM_SEEK,
	COM_SEEKSTART,

	/* 40 */
	COM_AS,
	COM_APPEND,
	COM_READWRITE,
	COM_ON,
	COM_ERROR,

	/* 45 */
	COM_CONTINUE,
	COM_BREAK,
	COM_CLS,
	COM_PEN,
	COM_PAPER,

	/* 50 */
	COM_STYLE,
	COM_LOCATE,
	COM_SCROLL,
	COM_SRAND,
	COM_ECHO,

	/* 55 */
	COM_OFF,
	COM_REMOVE,
	COM_MKDIR,
	COM_RENAME,
	COM_CHMOD,

	/* 60 */
	COM_CD,
	COM_OPENDIR,
	COM_DIR,
	COM_CURSOR,
	COM_INDENT,

	/* 65 */
	COM_PAGING,
	COM_CHOOSE,
	COM_CASE,
	COM_DEFAULT,
	COM_CHOSEN,

	/* 70 */
	COM_SORT,

	COM_NOTSET
	};

enum en_function {
	/* 0 */
	FUNC_SQRT,
	FUNC_ROUND,
	FUNC_FLOOR,
	FUNC_CEIL,
	FUNC_SIN,

	/* 5 */
	FUNC_COS,
	FUNC_TAN,
	FUNC_ASIN,
	FUNC_ACOS,
	FUNC_ATAN,

	/* 10 */
	FUNC_LOG,
	FUNC_LOG10,
	FUNC_HYPOT,
	FUNC_ISNUM,
	FUNC_EOF,

	/* 15 */
	FUNC_HASDATA,
	FUNC_WAITDATA,
	FUNC_SEARCH,
	FUNC_LENGTH,
	FUNC_WORDCNT,

	/* 20 */
	FUNC_VAL,
	FUNC_ASC,
	FUNC_RAND,
	FUNC_TIME,
	FUNC_FACT,

	/* 25 */
	FUNC_MAX,
	FUNC_MIN,
	FUNC_MEAN,
	FUNC_GMEAN,
	FUNC_MEDIAN,

	/* 30 */
	FUNC_MATCH,
	FUNC_ARMAX,
	FUNC_ARMIN,
	FUNC_ARMEAN,
	FUNC_ARGMEAN,

	/* 35 */
	FUNC_ARMEDIAN,
	FUNC_ABS,
	FUNC_ISUPPER,
	FUNC_ISLOWER,
	FUNC_ISALPHA,

	/* 40 */
	FUNC_ISALPNUM,
	FUNC_ISDIGIT,
	FUNC_ISBLANK,
	FUNC_ISPRINT,
	FUNC_ISPUNCT,

	/* 45 */
	FUNC_ENVEXISTS,
	
	/* String functions. If FUNC_CHR is moved then update createToken()
	   & evalStrFunction() */
	/* 0 (46) */
	FUNC_CHR, 
	FUNC_MID,
	FUNC_LEFT,
	FUNC_RIGHT,
	FUNC_FORMAT,

	/* 5 */
	FUNC_PAD,
	FUNC_WORD,
	FUNC_ERROR,
	FUNC_DATE,
	FUNC_STAT,

	/* 10 */
	FUNC_LSTAT,
	FUNC_MAXSTR,
	FUNC_MINSTR,
	FUNC_ARMAXSTR,
	FUNC_ARMINSTR,

	/* 15 */
	FUNC_TOSTR,
	FUNC_UPPER,
	FUNC_LOWER,
	FUNC_REVERSE,
	FUNC_SWAPCASE,

	/* 20 */
	FUNC_GETENV,

	FUNC_NOTSET
	};

enum en_stop {
	DONT_STOP,
	BREAK_STOP,
	BREAK_END
	};

enum en_stream_type {
	STREAM_READ,
	STREAM_WRITE,
	STREAM_READWRITE,
	STREAM_DIR
	};

enum en_sysvar {
	NOT_SYSVAR = -1,
	SYSVAR_ARGC,
	SYSVAR_ARGV,
	SYSVAR_VERSION,
	SYSVAR_CREDITS,
	SYSVAR_BUILD_DATE,
	SYSVAR_CURR_DIR,
	SYSVAR_USERNAME,
	SYSVAR_USERGROUP,
	SYSVAR_LAST_ERROR,
	SYSVAR_NUM_ERRORS,
	SYSVAR_NUM_PENS,
	SYSVAR_NUM_PAPERS,
	SYSVAR_NUM_STYLES,
	SYSVAR_SCR_WIDTH,
	SYSVAR_SCR_HEIGHT,
	SYSVAR_PI,
	SYSVAR_DEG_PER_RAD
	};

#ifdef MAINFILE
char *error[NUM_ERRORS] = {
	"OK",
	"INTERNAL ERROR",
	"Memory allocation error",
	"Line too long",
	"Syntax error",
	"Syntax error in file",
	"Missing quotes",
	"Invalid argument",
	"Invalid line number",
	"Missing bracket(s)",
	"Variable is read only",
	"Variable is not an array",
	"Variable is an array",
	"Undefined variable",
	"Undefined array",
	"Missing array index",
	"Invalid array index",
	"Invalid array size",
	"Array index out of bounds",
	"Maximum array dimensionality exceeded",
	"Array too big",
	"Missing argument(s) in function",
	"Unexpected argument(s) in function",
	"Expression too complex",
	"Variable already exists",
	"Division by zero",
	"Interactive command",
	"No such line number",
	"Recursion limit reached",
	"Unexpected RETURN",
	"Missing ELSE or FI",
	"Unexpected ELSE",
	"Unexpected FI",
	"Missing WEND",
	"Unexpected WEND",
	"Missing UNTIL",
	"Unexpected UNTIL",
	"Missing NEXT",
	"Unexpected NEXT",
	"Non numeric data",
	"READ or RESTORE on non DATA line",
	"End of DATA reached",
	"Can't open file or directory",
	"No program to save",
	"Stream not open",
	"Stream already open",
	"Invalid stream",
	"Invalid stream type",
	"Can't close stream",
	"Can't seek on stream",
	"End of file",
	"Invalid format string",
	"Number out of bounds",
	"Read failure",
	"Write failure",
	"Can't delete file or directory",
	"Can't rename file or directory",
	"Can't create directory",
	"Can't change permissions",
	"Can't stat file or directory",
	"Can't change directory",
	"Missing CHOSEN",
	"Unexpected CHOSEN",
	"Invalid CASE argument type in CHOOSE",

	/* Not a true error */
	"Interrupt" 
	};

char *command[NUM_COMMANDS] = {
	/* 0 */
	"REM",
	"LIST",
	"RUN",
	"END",
	"EXIT",

	/* 5 */
	"CLEAR",
	"NEW",
	"DELETE",
	"DIM",
	"LET",

	/* 10 */
	"PRINT",
	"WRITE",
	"GOTO",
	"GOSUB",
	"RETURN",

	/* 15 */
	"WHILE",
	"WEND",
	"DO",
	"UNTIL",
	"FOR",

	/* 20 */
	"NEXT",
	"TO",
	"STEP",
	"THEN",
	"IF",

	/* 25 */
	"ELSE",
	"FI",
	"DATA",
	"READ",
	"RESTORE",

	/* 30 */
	"INPUT",
	"CINPUT",
	"LOAD",
	"SAVE",
	"RENUM",

	/* 35 */
	"PAUSE",
	"OPEN",
	"CLOSE",
	"SEEK",
	"SEEKSTART",

	/* 40 */
	"AS",
	"APPEND",
	"READWRITE",
	"ON",
	"ERROR",
	
	/* 45 */
	"CONTINUE",
	"BREAK",
	"CLS",
	"PEN",
	"PAPER",

	/* 50 */
	"STYLE",
	"LOCATE",
	"SCROLL",
	"SRAND",
	"ECHO",

	/* 55 */
	"OFF",
	"REMOVE",
	"MKDIR",
	"RENAME",
	"CHMOD",

	/* 60 */
	"CD",
	"OPENDIR",
	"DIR",
	"CURSOR",
	"INDENT",

	/* 65 */
	"PAGING",
	"CHOOSE",
	"CASE",
	"DEFAULT",
	"CHOSEN",

	/* 70 */
	"SORT"
	};

/* These are functions which return a number. They may well take
   string argumenst however */
char *numfunc[NUM_NUM_FUNCTIONS] = {
	/* 0 */
	"SQRT",
	"ROUND",
	"FLOOR",
	"CEIL",
	"SIN",

	/* 5 */
	"COS",
	"TAN",
	"ASIN",
	"ACOS",
	"ATAN",

	/* 10 */
	"LOG",
	"LOG10",
	"HYPOT",
	"ISNUM",
	"EOF",

	/* 15 */
	"HASDATA",
	"WAITDATA",
	"SEARCH",
	"LENGTH",
	"WORDCNT",

	/* 20 */
	"VAL",
	"ASC",
	"RAND",
	"TIME",
	"FACT",

	/* 25 */
	"MAX",
	"MIN",
	"MEAN",
	"GMEAN",
	"MEDIAN",

	/* 30 */
	"MATCH",
	"ARMAX",
	"ARMIN",
	"ARMEAN",
	"ARGMEAN",

	/* 35 */
	"ARMEDIAN",
	"ABS",
	"ISUPPER",
	"ISLOWER",
	"ISALPHA",

	/* 40 */
	"ISALNUM",
	"ISDIGIT",
	"ISBLANK",
	"ISPRINT",
	"ISPUNCT",

	/* 45 */
	"ENVEXISTS"
	};

/* These are functions which return a string */
char *strfunc[NUM_STR_FUNCTIONS] = {
	/* 0 */
	"CHR$",
	"MID$",
	"LEFT$",
	"RIGHT$",
	"FORMAT$",

	/* 5 */
	"PAD$",
	"WORD$",
	"ERROR$",
	"DATE$",
	"STAT$",

	/* 10 */
	"LSTAT$",
	"MAX$",
	"MIN$",
	"ARMAX$",
	"ARMIN$",

	/* 15 */
	"TOSTR$",
	"UPPER$",
	"LOWER$",
	"REVERSE$",
	"SWAPCASE$",

	/* 20 */
	"GETENV$"
	};

char *system_vars[NUM_SYSTEM_VARS] = {
	"argc",
	"argv$",
	"version$",
	"credits$",
	"build_date$",
	"current_dir$",
	"username$",
	"usergroup$",
	"last_error",
	"num_errors",
	"num_pens",
	"num_papers",
	"num_styles",
	"screen_width",
	"screen_height",
	"pi",
	"degs_per_radian"
	};

char *fgcols[NUM_FGCOLS] = {
	/* Black, red, green, yellow */
	"\033[30m","\033[31m","\033[32m","\033[33m",
	/* Blue, magenta, turquoise, white */
	"\033[34m","\033[35m","\033[36m","\033[37m"
	};
char *bgcols[NUM_BGCOLS] = {
	/* Colours same as above */
	"\033[40m","\033[41m","\033[42m","\033[43m",
	"\033[44m","\033[45m","\033[46m","\033[47m"
	};
char *text_styles[NUM_STYLES] = {
	/* Bold, italic, underline, slow blink, fast blink, reverse */
	"\033[1m","\033[3m","\033[4m","\033[5m","\033[6m","\033[7m"
	};
#else

extern char *error[NUM_ERRORS];
extern char *command[NUM_COMMANDS];
extern char *numfunc[NUM_NUM_FUNCTIONS];
extern char *strfunc[NUM_STR_FUNCTIONS];
char *system_vars[NUM_SYSTEM_VARS];
char *fgcols[NUM_FGCOLS];
char *bgcols[NUM_BGCOLS];
char *text_styles[NUM_STYLES];

#endif


/*** Structures ***/
struct st_bas_line {
	uint32_t line_number;
	struct st_line *first_line;
	struct st_line *last_line;
	struct st_bas_line *prev,*next;
	};

struct st_fordata {
	char *varname;
	char finished;
	int varindex[MAX_ARRAY_DEPTH];
	int depth;
	double start_val;
	double end_val;
	double step_val;
	};

struct st_line {
	struct st_bas_line *parent;
	struct st_line *goto_line;
	struct st_token **tokens;
	struct st_fordata *fordata;
	int num_tokens;
	int num_allocated;
	int goto_line_number;
	char paired;
	char renum;
	struct st_line *next;
	};

struct st_token {
	char *text;
	int text_len;
	enum en_type type;
	enum en_command com;
	enum en_function func;
	double value;
	char negated;
	};

struct st_var {
	int size;
	int index[MAX_ARRAY_DEPTH];
	int prodindex[MAX_ARRAY_DEPTH];
	int depth;
	char *name;
	char *str_value;
	char **arr_str_value;
	double value;
	double *arr_value;
	enum en_type type;
	struct st_var *next;
	};

struct st_stream {
	enum en_stream_type type;
	int fd;
	DIR *dfp;
	char eof;
	} streams[NUM_STREAMS];

/*** Other global variables ***/
struct termios stored_tio;
struct st_bas_line *first_basline, *last_basline;
struct st_line *gosub_stack[GOSUB_STACK_SIZE];
struct st_line *goto_line;
struct st_line *error_goto_line;
struct st_line *data_line;
struct st_var *varlist[256];
int basic_argc;
int gosub_stack_pos;
int data_token_pos;
int stored_errno;
int pen,paper,style;
char **basic_argv;
char *run_filename;
char stop_program;
char program_running;
char goto_line_set;
char printed_break;
char next_jump;
char echo_on;
char cursor_on;
char break_on;
char paging_on;
char indent_chars;
char *username;
char *usergroup;
enum en_error last_error;
enum en_command error_action;

/*** Forward declarations for global functions ***/

/* main.c */
void init(void);
enum en_error loadProgram(char *filename);
enum en_error runProgram(struct st_line *line);
void doExit(int err);

/* tokenizer.c */
enum en_error parseTextLine(char *str, struct st_bas_line **basline);
void deleteToken(struct st_token *token);

/* lines.c */
void addProgramLine(struct st_bas_line *basline);
void deleteProgramLine(struct st_bas_line *basline);
struct st_bas_line *createBasicLine(void);
void deleteBasicLine(struct st_bas_line *basline);
struct st_line *createLine(struct st_bas_line *parent);
void deleteLine(struct st_line *line);
void addTokenToLine(
	struct st_line *line, struct st_token *token);
struct st_line *getLine(uint32_t linenum);
void resetLines(struct st_line *line);

/* com_inter.c */
enum en_error comList(enum en_command com, struct st_line *line, int start);
enum en_error comRun(enum en_command com, struct st_line *line, int start);
enum en_error comClear(enum en_command com, struct st_line *line, int start);
enum en_error comNew(enum en_command com, struct st_line *line, int start);
enum en_error comDelete(enum en_command com, struct st_line *line, int start);
enum en_error comLoad(enum en_command com, struct st_line *line, int start);
enum en_error comSave(enum en_command com, struct st_line *line, int start);
enum en_error comRenum(enum en_command com, struct st_line *line, int start);
enum en_error comDir(enum en_command com, struct st_line *line, int start);
enum en_error comIndent(enum en_command com, struct st_line *line, int start);
enum en_error comPaging(enum en_command com, struct st_line *line, int start);

/* com_io.c */
enum en_error comPrintWrite(enum en_command com, struct st_line *line, int start);
enum en_error comInput(enum en_command com, struct st_line *line, int start);
enum en_error comOpen(enum en_command com, struct st_line *line, int start);
enum en_error comClose(enum en_command com, struct st_line *line, int start);
enum en_error comSeek(enum en_command com, struct st_line *line, int start);
enum en_error comRemove(enum en_command com, struct st_line *line, int start);
enum en_error comRename(enum en_command com, struct st_line *line, int start);
enum en_error comMkdirChmodCd(enum en_command com, struct st_line *line, int start);
enum en_error comOpendir(enum en_command com, struct st_line *line, int start);

/* com_screen.c */
enum en_error comCls(enum en_command com, struct st_line *line, int start);
enum en_error comPen(enum en_command com, struct st_line *line, int start);
enum en_error comPaper(enum en_command com, struct st_line *line, int start);
enum en_error comStyle(enum en_command com, struct st_line *line, int start);
enum en_error comLocate(enum en_command com, struct st_line *line, int start);
enum en_error comScroll(enum en_command com, struct st_line *line, int start);
enum en_error comEcho(enum en_command com, struct st_line *line, int start);
enum en_error comCursor(enum en_command com, struct st_line *line, int start);

/* com_flow.c */
enum en_error comEnd(enum en_command com, struct st_line *line, int start);
enum en_error comExit(enum en_command com, struct st_line *line, int start);
enum en_error comGotoGosub(enum en_command com, struct st_line *line, int start);
enum en_error comReturn(enum en_command com, struct st_line *line, int start);
enum en_error comWhile(enum en_command com, struct st_line *line, int start);
enum en_error comWend(enum en_command com, struct st_line *line, int start);
enum en_error comDo(enum en_command com, struct st_line *line, int start);
enum en_error comUntil(enum en_command com, struct st_line *line, int start);
enum en_error comFor(enum en_command com, struct st_line *line, int start);
enum en_error comNext(enum en_command com, struct st_line *line, int start);
enum en_error comIf(enum en_command com, struct st_line *line, int start);
enum en_error comElse(enum en_command com, struct st_line *line, int start);
enum en_error comFi(enum en_command com, struct st_line *line, int start);
enum en_error comOn(enum en_command com, struct st_line *line, int start);
enum en_error comChoose(enum en_command com, struct st_line *line, int start);

/* com_var.c */
enum en_error comDim(enum en_command com, struct st_line *line, int start);
enum en_error comLet(enum en_command com, struct st_line *line, int start);
enum en_error comData(enum en_command com, struct st_line *line, int start);
enum en_error comRead(enum en_command com, struct st_line *line, int start);
enum en_error comRestore(enum en_command com, struct st_line *line, int start);
enum en_error comSort(enum en_command com, struct st_line *line, int start);

/* com_misc.c */
enum en_error comPseudo1(enum en_command com, struct st_line *line, int start);
enum en_error comPseudo2(enum en_command com, struct st_line *line, int start);
enum en_error comPause(enum en_command com, struct st_line *line, int start);
enum en_error comBreak(enum en_command com, struct st_line *line, int start);
enum en_error comSrand(enum en_command com, struct st_line *line, int start);


/* expressions.c */
enum en_error evalNumExpr(
	struct st_line *line, int start, int *end, double *result);
enum en_error evalStringExpr(
	struct st_line *line,
	int start, int *end,
	enum en_type *result_type, double *result, char **strres);

/* variables.c */
enum en_error createVariable(
	char *name,
	enum en_type type, int *index, int depth, struct st_var **newvar);
void deleteVariable(struct st_var *var);
void deleteAllVariables(void);
struct st_var *getVariable(char *name);
enum en_error setVarValue(
	char *name, int *index, int depth, double value, char *strval);
enum en_error getVarValue(
	char *name, int *index, int depth, double *value, char **strval);
enum en_error calcArrIndex(struct st_var *var, int *index, int depth, int *idx);
int getSystemVarNum(char *name);
enum en_error getSystemVarValue(
	int sv, int *index, int depth, double *value, char **strval);
enum en_error getArrVarValue(
	struct st_line *line,
	int start, int *end, double *value, char **strval);
enum en_error getArrIndex(
	struct st_line *line, int start, int *end, int *index, int *depth);
enum en_error sortArray(char *name);


/* func_numeric.c */
enum en_error evalNumFunction(
	struct st_line *line, int start, int *end, double *result);

enum en_error funcSqrt(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcRoundFloorCeil(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcSin(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcCos(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcTan(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcAsin(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcAcos(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcAtan(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcLog(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcLog10(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcHypot(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcIsnum(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcSearch(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcEOF(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcHasWaitdata(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcLength(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcWordcnt(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcVal(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcAsc(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcRand(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcTime(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcFact(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcMaxMinMean(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcMedian(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcMatch(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcArMaxMinMean(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcArmedian(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcAbs(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcStringTypeCheck(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);
enum en_error funcEnvExists(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, double *result);

/* func_string.c */
enum en_error evalStrFunction(
	struct st_line *line, int start, int *end, char **result);

enum en_error funcChr(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcMidLeftRight(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcFormat(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcPad(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcWord(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcError(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcDate(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcStatLstat(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcMaxMinStr(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcArMaxMinStr(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcTostr(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcUpperLower(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcReverse(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcSwapcase(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);
enum en_error funcGetenv(
	enum en_function func,
	struct st_line *line, int start, int *end, int limit, char **result);


/* misc.c */
enum en_error getInput(int fd, char *input, char one);
char *toupperString(char *s);
void appendString(char **result, char *str);
int findRightBracket(struct st_line *line, int start);
enum en_error getLineNumbers(
	struct st_line *line, int start, int *start_line, int *end_line);
char setLoopEnd(
	struct st_line *line,
	enum en_command start_com, enum en_command end_com);
int isNumeric(char *str);
void printError(enum en_error err, int line);
void reset(void);
void closeStreams(void);
enum en_error getStream(
	struct st_line *line, int start, int *end, double *stream);
enum en_error  setColours(int fd);
void listProgram(FILE *fp, int start_line, int end_line);
enum en_error fdWrite(int fd, char *str, int len);
enum en_error calcPermission(double result, mode_t *perm);
struct winsize *getWinSize(void);
int wildmatch(char *str, char *pat);
void numericSort(int num, double *data);


/*** Command function array ***/
#ifdef MAINFILE
enum en_error (*comfunc[NUM_COMMANDS])(enum en_command, struct st_line *, int) = {
	/* 0 */
	&comPseudo1,
	&comList,
	&comRun,
	&comEnd,
	&comExit,

	/* 5 */
	&comClear,
	&comNew,
	&comDelete,
	&comDim,
	&comLet,

	/* 10 */
	&comPrintWrite,
	&comPrintWrite,
	&comGotoGosub,
	&comGotoGosub,
	&comReturn,

	/* 15 */
	&comWhile,
	&comWend,
	&comDo,
	&comUntil,
	&comFor,

	/* 20 */
	&comNext,
	&comPseudo2,
	&comPseudo2,
	&comPseudo2,
	&comIf,

	/* 25 */
	&comElse,
	&comFi,
	&comData,
	&comRead,
	&comRestore,

	/* 30 */
	&comInput,
	&comInput,
	&comLoad,
	&comSave,
	&comRenum,

	/* 35 */
	&comPause,
	&comOpen,
	&comClose,
	&comSeek,
	&comSeek,

	/* 40 */
	&comPseudo2,
	&comPseudo2,
	&comPseudo2,
	&comOn,
	&comPseudo2,

	/* 45 */
	&comPseudo2,
	&comBreak,
	&comCls,
	&comPen,
	&comPaper,

	/* 50 */
	&comStyle,
	&comLocate,
	&comScroll,
	&comSrand,
	&comEcho,

	/* 55 */
	&comPseudo2,
	&comRemove,
	&comMkdirChmodCd,
	&comRename,
	&comMkdirChmodCd,

	/* 60 */
	&comMkdirChmodCd,
	&comOpendir,
	&comDir,
	&comCursor,
	&comIndent,

	/* 65 */
	&comPaging,
	&comChoose,
	&comPseudo1,
	&comPseudo1,
	&comPseudo1,

	/* 70 */
	&comSort
	};
#else
extern enum en_error (*comfunc[NUM_COMMANDS])(enum en_command, struct st_line *, int);
#endif


/*** Function function pointer array ***/
#ifdef MAINFILE
enum en_error (*numfuncptr[NUM_NUM_FUNCTIONS])(enum en_function, struct st_line *, int, int *, int, double *) = {
	/* 0 */
	&funcSqrt,
	&funcRoundFloorCeil,
	&funcRoundFloorCeil,
	&funcRoundFloorCeil,
	&funcSin,

	/* 5 */
	&funcCos,
	&funcTan,
	&funcAsin,
	&funcAcos,
	&funcAtan,

	/* 10 */
	&funcLog,
	&funcLog10,
	&funcHypot,
	&funcIsnum,
	&funcEOF,

	/* 15 */
	&funcHasWaitdata,
	&funcHasWaitdata,
	&funcSearch,
	&funcLength,
	&funcWordcnt,

	/* 20 */
	&funcVal,
	&funcAsc,
	&funcRand,
	&funcTime,
	&funcFact,

	/* 25 */
	&funcMaxMinMean,
	&funcMaxMinMean,
	&funcMaxMinMean,
	&funcMaxMinMean,
	&funcMedian,	

	/* 30 */
	&funcMatch,
	&funcArMaxMinMean,
	&funcArMaxMinMean,
	&funcArMaxMinMean,
	&funcArMaxMinMean,

	/* 35 */
	&funcArmedian,
	&funcAbs,
	&funcStringTypeCheck,
	&funcStringTypeCheck,
	&funcStringTypeCheck,

	/* 40 */
	&funcStringTypeCheck,
	&funcStringTypeCheck,
	&funcStringTypeCheck,
	&funcStringTypeCheck,
	&funcStringTypeCheck,

	/* 45 */
	&funcEnvExists
	};

enum en_error (*strfuncptr[NUM_STR_FUNCTIONS])(enum en_function, struct st_line *, int, int *, int, char **) = {
	/* 0 */
	&funcChr,
	&funcMidLeftRight,
	&funcMidLeftRight,
	&funcMidLeftRight,
	&funcFormat,

	/* 5 */
	&funcPad,
	&funcWord,
	&funcError,
	&funcDate,
	&funcStatLstat,

	/* 10 */
	&funcStatLstat,
	&funcMaxMinStr,
	&funcMaxMinStr,
	&funcArMaxMinStr,
	&funcArMaxMinStr,

	/* 15 */
	&funcTostr,
	&funcUpperLower,
	&funcUpperLower,
	&funcReverse,
	&funcSwapcase,

	/* 20 */
	&funcGetenv
	};
#else
extern enum en_error (*numfuncptr[NUM_NUM_FUNCTIONS])(enum en_function, struct st_line *, int, int *, int, double *);
extern enum en_error (*strfuncptr[NUM_STR_FUNCTIONS])(enum en_function, struct st_line *, int, int *, int, char **);
#endif

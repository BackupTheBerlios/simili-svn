%union{
  char *id;
  int digit;
  char *datastr;
}

%token <id> ID_TOKEN VARIABLE_TOKEN
%token <digit> DIGIT_TOKEN TIMESTAMP_TOKEN
%token <datastr> DATASTR_TOKEN
%token TRACK_TOKEN HEADER_TOKEN

%type <id> tsid variable

%start midi

%{
#include "text2midi.h"
int line=1;
//#define YYSTYPE char*

int yyerror()
{
  printf("Error line: %d\n", line);
  return 0;
}
#if 1
#define parsedbg(str)
#else
#define parsedbg(str) printf("parsedbg[%d]: %s\n", __LINE__, str)
#endif
%}

%%

tsid: ID_TOKEN {parsedbg($1); T2M_mmsg_new(T2M, $1, 0);}
  | TIMESTAMP_TOKEN ':' ID_TOKEN {parsedbg($3); T2M_mmsg_new(T2M, $3, $1);};

variable: { $$ = 0; } | VARIABLE_TOKEN '=' { $$ = $1; };

param: variable ID_TOKEN {parsedbg("param"); T2M_mmsg_param_id(T2M, $1, $2);}
  | variable DIGIT_TOKEN {parsedbg("param"); T2M_mmsg_param_digit(T2M, $1, $2);}
  | variable DATASTR_TOKEN {parsedbg("param"); T2M_mmsg_param_datastr(T2M, $1, $2);};

param_list: /* Nothing */
  | ':' param {parsedbg("param_list");}
  | param_list ',' param {parsedbg("param_list");};

statement: tsid param_list ';' {parsedbg("statement"); T2M_mmsg_flush(T2M);};
statements: | statements statement {parsedbg("statements");};

header: | HEADER_TOKEN statements {parsedbg("header");T2M_header_flush(T2M);};
tracks: | tracks TRACK_TOKEN statements {parsedbg("tracks");T2M_track_flush(T2M);};

midi: header tracks {parsedbg("midi");};
%%

%{
#include <stdio.h>
#include <malloc.h>

%}

%union {
char sym;
double dval;
}

%token <dval> NUM
%token <sym>  SYMBOL

%token ARROW
%token AXIOM_WORD
%token RULES_WORD
%token ANGLE_WORD
%token TETHA_WORD
%token SIGMA_WORD
%token DEPTH_WORD
%token XYINIT_WORD
%token XYSCALE_WORD

%type <dval> repeat_num

%%

input: definition ;

definition: axiom rules parameters ;

/* expected examples:  "Axiom: F", "Axiom: F[F+]G", etc. */

axiom: AXIOM_WORD ':' pattern_non_empty { printf("Axiom Head Word Parsed :\n"); }
	;

pattern_non_empty: SYMBOL repeat_num
				{ 
					//char newname;
                    printf("%c",$1);
				}
			pattern
	| '|' { printf(" | "); } pattern
	| '$' { printf(" $ "); } pattern
	| '[' { printf(" [ "); } pattern_non_empty ']' { printf( " ] "); } pattern
	| '+' { printf(" + "); } pattern
	| '-' { printf(" - "); } pattern
	| '%' { printf(" % "); } /* empty rule */
	;

pattern: /* empty *//* 仅仅是用来消除左递归*/
	| pattern_non_empty 
	;

repeat_num: /* empty */ { $$ = 0; }
	| NUM { 
        $$ = $1; 
        printf("Number Parsed %g \n",$1);
    }
	;

rules: RULES_WORD ':' rules_decl_non_empty { 
        //char* predecessor = (char*) $3;
        //printf("rule captured %s\n", predecessor);
    }
	;

/* there may be many rules, unlike the axiom */
rules_decl_non_empty: SYMBOL ARROW pattern_non_empty rules_decl 
	| ';' rules_decl_non_empty 
	;

rules_decl: /* empty */
	| rules_decl_non_empty
	;

parameters: /* empty */
	| angle     parameters
	| sigma     parameters
	| tetha     parameters
	| depth     parameters
	| xyinit    parameters
	| xyscale   parameters
	;

angle:      ANGLE_WORD      ':' NUM { printf("parameter"); }
sigma:      SIGMA_WORD      ':' NUM { printf("parameter"); }
tetha:      TETHA_WORD      ':' NUM { printf("parameter"); }
depth:      DEPTH_WORD      ':' NUM { printf("parameter"); }
xyinit:     XYINIT_WORD     ':' NUM NUM { printf("parameter"); }
xyscale:    XYSCALE_WORD    ':' NUM NUM { printf("parameter"); }

%%



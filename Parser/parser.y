%token AUTO BREAK CHAR STRING CONSTANT CONTINUE ELSE

%token FLOAT BOOL FOR IF INT LARGE RETURN

%token SMALL STATIC STRUCT TYPEDEF VOID WHILE FUNCTION

%token GRAPH DIRECTED_GRAPH NODE_SET NODE_SEQUENCE NODE_PROP EDGE_PROP

%token EDGE BFS DFS NODES NODE LEVELS NEIGHBOURS

%token ELLIPSIS APPEND_ASSIGN RIGHT_SHIFT_ASSIGN LEFT_SHIF_ASSIGN 

%token ADD_ASSIGN SUB_ASSIGN EXP_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN

%token AND_ASSIGN XOR_ASSIGN OR_ASSIGN RSHIFT_OP LSHIFT_OP INC_OP

%token DEC_OP PTR_OP AND_OP OR_OP LE_OP GE_OP EQ_OP NE_OP

%token STM_DELIM L_BRACE R_BRACE COLON_OP ASSIGN L_PAREN R_PAREN

%token APPEND_OP EXP_OP L_BRAKET

%token R_BRAKET DOT_OP COMMA_OP ADDRESS_OP NOT_OP

%token BIT_COMPL_OP SUBTRACT_OP ADDITION_OP MULTI_OP

%token DIV_OP MOD_OP LESSER_OP GREATER_OP BIT_XOR_OP

%token BIT_OR_OP COND_OP

%token STRING_LITERAL

%token IDENTIFIER HEXAL_CONST OCTAL_CONST CONST

%token INVALID_TOKEN

%%
  graph_initialisation : GRAPH IDENTIFIER ASSIGN L_BRACE edge_line R_BRACE STM_DELIM
  		       | DIRECTED_GRAPH IDENTIFIER ASSIGN L_BRACE edge_line R_BRACE STM_DELIM
  		       ;
  edgebasic: CONSTANT COLON_OP CONSTANT
  	    ;
  edgeline : edgebasic
 	   | edgebasic COMMA_OP edgeline
 	   ;
  node_declaration : NODE LESSER_OP IDENTIFIER GREATER_OP initializer_list STM_DELIM
  		   ;
  initializer_list : IDENTIFIER
  		   | IDENTIFIER ASSIGN CONSTANT 
  		   | IDENTIFIER COMMA_OP initializer_list
  		   | IDENTIFIER ASSIGN CONSTANT COMMA_OP initializer_list
         ;
%%
#include <stdio.h>

int main()
{
    yyparse();

    return 0;
}

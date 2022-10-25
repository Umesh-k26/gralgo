%skeleton "lalr1.cc" /* -*- c++ -*- */
%require "3.0"
%defines
%define api.parser.class {parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires
{
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "types.h"

class Driver;

}


// The parsing context.
%param { Driver& driver }

// Location tracking
%locations
%initial-action
{
    // Initialize the initial location.
    @$.begin.filename = @$.end.filename = &driver.file;
};

// Enable tracing and verbose errors (which may be wrong!)
%define parse.trace
%define parse.error verbose

// Parser needs to know about the driver:
%code
{
#include "driver.hh"
#define yylex driver.lexer.yylex

#define ctx driver.ctx
#define M(x) std::move(x)
#define C(x) node(x)
}

// Tokens:
%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  ASSIGN  '='
  MINUS   '-'
  PLUS    '+'
  STAR    '*'
  SLASH   '/'
  AMPERSAND '&'
  LPAREN  '('
  RPAREN  ')'
  MOD '%'
  B_OR '|'
  COMMA ','
  SEMI_COLON ';'
  COLON ':'
  LBRACE '{'
  RBRACE '}'
  LSB '['
  RSB ']'
  AND "&&"
  OR "||"
  PP "++"
  MM "--"
  PL_EQ "+="
  MI_EQ "-="
  MU_EQ "*="
  DI_EQ "/="
  LESS "<"
  GREATER ">"
  NE "!="
  EQ "=="
  GEQ ">="
  LEQ "<="

  BREAK "break"
  CONTINUE "continue"
  RETURN "return"
  IF "if"
  ELSE "else"
  WHILE "while"
  FOR "for"
  BFS "BFS"
  DFS "DFS"
;
%token
  VOID "void"
  INT "int"
  BOOL "bool"
  FLOAT "float"
  CHAR "char"
  STRING "string"
  GRAPH "graph"
  DGRAPH "dgraph"
  NODE "node"
  NODE_SET "node_set"
  NODE_SEQ "node_seq"
  NODE_PROP "node_prop"
  EDGE_PROP "edge_prop"
  EDGE_SET "edge_set"
  EDGE_SEQ "edge_seq"
  NODES "nodes"
  LEVELS "levels"
  NEIGHBOURS "neighbours"
;

// Use variant-based semantic values: %type and %token expect genuine types
%token <std::string> IDENTIFIER "identifier" STRING_LITERAL
%token <int> NUMBER "number"
%token <double> DOUBLE_CONST "double_const"
%type<std::string> identifier
%type<node> expr exprs stmt selection_stmt jump_stmt expression_stmt iteration_stmt vardec_stmt empty_stmt compound_stmt p_expr initializer initializer_list vardec1 edge
%type<type_name> typename

/* Operator precedence */
%left  ','
%right '?' ':' '=' "+=" "-="
%left  "||"
%left  "&&"
%left  "==" "!="
%left  '+' '-'
%left  '*' '/' '%'
%right '&' "++" "--"
%left  '(' '['

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

// No %destructors are needed, since memory will be reclaimed by the
// regular destructors.
/* %printer { yyoutput << $$; } <*>; */

// Grammar:
%%
%start program;

program: { ++ctx; } declarations { --ctx; ctx.dump_ast(); };
declarations: declarations declaration { }
|             %empty
;
declaration: vardec_stmt
|            function
;

function: typename identifier { ctx.defun($2); ++ctx; } LPAREN paramdecls RPAREN compound_stmt RBRACE { ctx.add_function(M($2), M($7), $1); --ctx; } 
;
paramdecls: paramdecl
|           %empty
;
paramdecl:  paramdecl COMMA typename identifier { ctx.defparam($4, $3); }
|           typename identifier { ctx.defparam($2, $1); }
;
typename: VOID { $$ = type_name::VOID; }
|         INT { $$ = type_name::INT; }
|         BOOL { $$ = type_name::BOOL; }
|         CHAR { $$ = type_name::CHAR; }
|         FLOAT { $$ = type_name::FLOAT; }
|         STRING { $$ = type_name::STRING; }
|         GRAPH { $$ = type_name::GRAPH; }
|         DGRAPH { $$ = type_name::DGRAPH; }
|         NODE_PROP '<' identifier '>'
|         NODE_SET '<' identifier '>'
|         NODE_SEQ '<' identifier '>'
|         EDGE_PROP '<' identifier '>'
|         EDGE_SET '<' identifier '>'
|         EDGE_SEQ '<' identifier '>'
;

stmt: compound_stmt RBRACE { $$ = M($1); --ctx; }
|     selection_stmt
|     jump_stmt
|     expression_stmt
|     empty_stmt
|     vardec_stmt SEMI_COLON
|     iteration_stmt
;
expression_stmt: exprs SEMI_COLON { $$ = M($1); }
;
jump_stmt: CONTINUE SEMI_COLON { $$ = n_cont(); }
|          BREAK SEMI_COLON  { $$ = n_br(); }
|          RETURN SEMI_COLON  { $$ = n_ret(); }
|          RETURN expr SEMI_COLON                  { $$ = n_ret(M($2));         }
;
empty_stmt: SEMI_COLON
;
vardec_stmt: typename { ctx.temptype = $1; } vardec1 {  $$ = n_comma(M($3)); }
|            vardec_stmt COMMA vardec1 { $$ = M($1); $$.params.push_back(M($3)); }
;
vardec1: identifier ASSIGN initializer { $$ = ctx.def($1) %= M($3); }
|        identifier { $$ = ctx.def($1) %= 0; }
;
initializer: expr
|            edge
|            LBRACE initializer_list RBRACE { $$ = M($2); }
;
initializer_list: initializer { $$ = n_init_list(M($1)); }
|                 initializer_list COMMA initializer { $$ = M($1); $$.params.push_back($3); }
;
edge: NUMBER COLON NUMBER { $$ = n_edge($1, $3); }
;

compound_stmt:  LBRACE { $$ = n_comma(); ++ctx; }
|               compound_stmt stmt { $$ = M($1); $$.params.push_back(M($2)); }
;
selection_stmt: IF p_expr stmt %prec LOWER_THAN_ELSE  { $$ = n_cand(M($2), M($3)); }
|               IF p_expr stmt ELSE stmt   
;
iteration_stmt: WHILE p_expr stmt          { $$ = n_loop(M($2), M($3)); }
|               FOR '(' expr SEMI_COLON expr SEMI_COLON expr ')' stmt { $$ = n_loop(M($3), M($5), M($7), M($9)); }
|               FOR '(' typename identifier COLON identifier ')' stmt { $$ = n_loop(M($8)); }
|               BFS '(' typename identifier COLON identifier ')' stmt { $$ = n_loop(M($8)); }
|               DFS '(' typename identifier COLON identifier ')' stmt { $$ = n_loop(M($8)); }
;
p_expr: '(' expr ')' { $$ = M($2); }
;
exprs: expr                     { $$ = M($1); }
|      exprs COMMA expr         { $$ = M($1); $$.params.push_back(M($3)); }
;

expr: NUMBER                    { $$ = $1;    }
|     STRING_LITERAL            { $$ = M($1); }
|     identifier                { $$ = ctx.use($1);   }
|     '(' exprs ')'             { $$ = M($2); }
|     expr '[' exprs ']'        { $$ = n_deref(n_add(M($1), M($3))); }
|     expr '(' ')'              { $$ = n_fcall(M($1)); }
|     expr '(' exprs ')'
|     expr ASSIGN expr             { $$ = M($1) %= M($3); }
|     expr PLUS expr             { $$ = n_add( M($1), M($3)); }
|     expr MINUS expr %prec '+'   { $$ = n_add( M($1), n_neg(M($3))); }
|     expr STAR expr             { $$ = n_mul( M($1), M($3)); }
|     expr SLASH expr %prec '*'   { $$ = n_div( M($1), M($3)); }
|     expr MOD expr             { $$ = n_mod( M($1), M($3));}
|     expr "+=" expr            //{ if(!$3.is_pure()) { $$ = ctx.temp() %= node_addrof(M($1)); $1 = node_deref($$.params.back()); } $$ = node_comma(M($$), M($1) %= node_add(C($1), M($3))); }
|     expr "-=" expr            //{ if(!$3.is_pure()) { $$ = ctx.temp() %= node_addrof(M($1)); $1 = node_deref($$.params.back()); } $$ = node_comma(M($$), M($1) %= node_add(C($1), node_neg(M($3)))); }
|     "++" expr                 //{ if(!$2.is_pure()) { $$ = ctx.temp() %= node_addrof(M($2)); $2 = node_deref($$.params.back()); } $$ = node_comma(M($$), M($2) %= node_add(C($2),  1l)); }
|     "--" expr %prec "++"      //{ if(!$2.is_pure()) { $$ = ctx.temp() %= node_addrof(M($2)); $2 = node_deref($$.params.back()); } $$ = node_comma(M($$), M($2) %= node_add(C($2), -1l)); }
|     expr "++"                 //{ if(!$1.is_pure()) { $$ = ctx.temp() %= node_addrof(M($1)); $1 = node_deref($$.params.back()); } auto i = ctx.temp(); $$ = node_comma(M($$), C(i) %= C($1), C($1) %= node_add(C($1),  1l), C(i)); }
|     expr "--" %prec "++"      //{ if(!$1.is_pure()) { $$ = ctx.temp() %= node_addrof(M($1)); $1 = node_deref($$.params.back()); } auto i = ctx.temp(); $$ = node_comma(M($$), C(i) %= C($1), C($1) %= node_add(C($1), -1l), C(i)); }
|     expr "||" expr            { $$ = n_cor( M($1), M($3)); }
|     expr "&&" expr            { $$ = n_cand(M($1), M($3)); }
|     expr "==" expr            { $$ = n_eq(  M($1), M($3)); }
|     expr "!=" expr %prec "==" { $$ = n_eq(n_eq(M($1), M($3)), 0); }
|     AMPERSAND expr                  { $$ = n_addrof(M($2)); }
|     '*' expr  %prec '&'       { $$ = n_deref(M($2));  }
|     '-' expr  %prec '&'       { $$ = n_neg(M($2));    }
|     '!' expr  %prec '&'       { $$ = n_eq(M($2), 0); }
|     expr '?' expr COLON expr    //{ auto i = ctx.temp(); $$ = node_comma(node_cor(node_cand(M($1), node_comma(C(i) %= M($3), 1l)), C(i) %= M($5)), C(i)); }
;

identifier: IDENTIFIER               { $$ = M($1); };

%%

// Register errors to the driver:
void yy::parser::error (const location_type& l,
                          const std::string& m)
{
    driver.error(l, m);
}

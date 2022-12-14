/* Companion source code for "flex & bison", published by O'Reilly
 * Media, ISBN 978-0-596-15597-1
 * Copyright (c) 2009, Taughannock Networks. All rights reserved.
 * See the README file for license conditions and contact info.
 * $Header: /home/johnl/flnb/code/RCS/fb3-2funcs.c,v 2.1 2009/11/08 02:53:18 johnl Exp $
 */
/*
 * helper functions for fb3-2
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "funcs.h"

/* symbol table */
/* hash a symbol */

/* simple symtab of fixed size */
#define NHASH 9997
struct symbol symtab[NHASH];

static unsigned
symhash(char *sym)
{
  unsigned int hash = 0;
  unsigned c;

  while (c = *sym++)
    hash = hash * 9 ^ c;

  return hash;
}

struct symbol *
lookup(char *sym)
{
  struct symbol *sp = &symtab[symhash(sym) % NHASH];
  int scount = NHASH; /* how many have we looked at */

  while (--scount >= 0)
  {
    if (sp->name && !strcmp(sp->name, sym))
    {
      return sp;
    }

    if (!sp->name)
    { /* new entry */
      sp->type = 0;
      sp->name = strdup(sym);
      sp->value = 0;
      sp->func = NULL;
      sp->syms = NULL;
      return sp;
    }

    if (++sp >= symtab + NHASH)
      sp = symtab; /* try the next entry */
  }
  yyerror("symbol table overflow\n");
  abort(); /* tried them all, table is full */
}

struct ast *
newast(int nodetype, struct ast *l, struct ast *r)
{
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *
newdouble(double d)
{
  struct doubleval *a = malloc(sizeof(struct doubleval));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'D';
  a->number = d;
  return (struct ast *)a;
}

struct ast *
newint(int i)
{
  struct intval *a = malloc(sizeof(struct intval));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'K';
  a->number = i;
  return (struct ast *)a;
}

struct ast *
newedge(int s, int d)
{
  struct edgeval *a = malloc(sizeof(struct edgeval));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'E';
  a->source = s;
  a->dest = d;
  return (struct ast *)a;
}

struct ast *
newstr(char *s)
{
  struct strval *a = malloc(sizeof(struct strval));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'S';
  a->s = s;
  return (struct ast *)a;
}

struct ast *
newarray(struct ast *l)
{
  struct arrayval *a = malloc(sizeof(struct arrayval));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'A';
  a->l = l;
  return (struct ast *)a;
}

struct ast *
newdef(struct symbol *f)
{
  struct funcval *a = malloc(sizeof(struct funcval));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'P';
  a->f = f;
  return (struct ast *)a;
}

struct ast *
newcmp(int cmptype, struct ast *l, struct ast *r)
{
  struct ast *a = malloc(sizeof(struct ast));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '0' + cmptype;
  a->l = l;
  a->r = r;
  return a;
}

struct ast *
newfunc(int functype, struct ast *l)
{
  struct fncall *a = malloc(sizeof(struct fncall));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'F';
  a->l = l;
  a->functype = functype;
  return (struct ast *)a;
}

struct ast *
newcall(struct symbol *s, struct ast *l)
{
  struct ufncall *a = malloc(sizeof(struct ufncall));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'C';
  a->l = l;
  a->s = s;
  return (struct ast *)a;
}

struct ast *
newref(struct symbol *s)
{
  struct symref *a = malloc(sizeof(struct symref));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = 'N';
  a->s = s;
  return (struct ast *)a;
}

struct ast *
newasgn_ops(int nodetype, struct symbol *l, struct ast *r)
{
  struct symasgn *a = malloc(sizeof(struct symasgn));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = 'a' - 1 + nodetype;
  a->s = l;
  a->v = r;

  return (struct ast *)a;
}

struct ast *
newasgn(struct symbol *s, struct ast *v)
{
  struct symasgn *a = malloc(sizeof(struct symasgn));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '=';
  a->s = s;
  a->v = v;
  return (struct ast *)a;
}

struct ast *
newinit(int inittype, struct symbol *s, struct ast *v)
{
  struct syminit *a = malloc(sizeof(struct syminit));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = '=';
  s->type = inittype;
  a->s = s;
  a->v = v;
  return (struct ast *)a;
}

struct ast *
newginit(int inittype, struct symbol* g,struct symbol *s, struct ast *v)
{
  struct syminit *a = malloc(sizeof(struct syminit));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  s->g = g;
  a->nodetype = '=';
  s->type = inittype;
  a->s = s;
  a->v = v;
  return (struct ast *)a;  
}

struct ast *settype(int inittype, struct symlist *sl)
{
  struct symlistdef *tmp = malloc(sizeof(struct symlistdef));

  tmp->sl = sl;
  tmp->nodetype = 200;

  return (struct ast *)tmp;
}

struct ast *
newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
  struct flow *a = malloc(sizeof(struct flow));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->cond = cond;
  a->tl = tl;
  a->el = el;
  return (struct ast *)a;
}

struct ast *
newfor(int nodetype, struct ast *init, struct ast *cond, struct ast *inc, struct ast *stmt)
{
  struct for_loop *a = malloc(sizeof(struct for_loop));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }
  a->nodetype = nodetype;
  a->init = init;
  a->cond = cond;
  a->inc = inc;
  a->stmt = stmt;
  return (struct ast *)a;
}

struct ast *rangefor(int nodetype, int typename, struct symbol *d, struct symbol *v, struct ast *stmt)
{
  struct rfor_loop *a = malloc(sizeof(struct rfor_loop));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = nodetype;
  a->d = d;
  a->d->type = typename;
  a->v = v;
  a->stmt = stmt;
  return (struct ast *)a;
}

struct ast *bfs(int nodetype, int typename_d, struct symbol *d, struct symbol *g, struct symbol *stSym, struct ast *stmt)
{
  // symbol d checks :  check for typename . it should be a node or levels

  //  symbol g checks : check for typename (not provided in syntax, check thru symbol table) . it should be a graph type list (G.Nodes or G.Levels)

  //  symbol stSym : check for typename . it should be a node.

  struct bfs *a = malloc(sizeof(struct bfs));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = nodetype;
  a->d = d;
  a->g = g;

  a->stSym = stSym;
  a->stmt = stmt;

  return (struct ast *)a;
}
struct ast *dfs(int nodetype, int typename_d, struct symbol *d, struct symbol *g, struct symbol *stSym, struct ast *stmt)
{
  // symbol d checks :  check for typename . it should be a node or levels

  //  symbol g checks : check for typename (not provided in syntax, check thru symbol table) . it should be a graph type list (G.Nodes or G.Levels)

  //  symbol stSym : check for typename . it should be a node.

  struct dfs *a = malloc(sizeof(struct dfs));

  if (!a)
  {
    yyerror("out of space");
    exit(0);
  }

  a->nodetype = nodetype;
  a->d = d;
  a->g = g;
  a->stSym = stSym;
  a->stmt = stmt;

  return (struct ast *)a;
}

struct symlist *
newsymlist(struct symbol *sym, struct symlist *next, struct ast* v)
{
  struct symlist *sl = malloc(sizeof(struct symlist));

  if (!sl)
  {
    yyerror("out of space");
    exit(0);
  }
  sl->sym = sym;
  sl->next = next;
  sl->sym->v = v;
  return sl;
}

void symlistfree(struct symlist *sl)
{
  struct symlist *nsl;

  while (sl)
  {
    nsl = sl->next;
    free(sl);
    sl = nsl;
  }
}

/* define a function */
void dodef(int type, struct symbol *name, struct symlist *syms, struct ast *func)
{
  if (name->syms)
    symlistfree(name->syms);
  if (name->func)
    treefree(name->func);
  name->type = type;
  name->syms = syms;
  name->func = func;
}


void treefree(struct ast *a)
{
  switch (a->nodetype)
  {

    /* two subtrees */
  case '+':
  case '-':
  case '*':
  case '/':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case 'L':
    treefree(a->r);

    /* one subtree */
  case '|':
  case 'M':
  case 'C':
  case 'F':
    treefree(a->l);

    /* no subtree */
  case 'K':
  case 'N':
    break;

  case '=':
    free(((struct symasgn *)a)->v);
    break;

  case 'I':
  case 'W':
    free(((struct flow *)a)->cond);
    if (((struct flow *)a)->tl)
      free(((struct flow *)a)->tl);
    if (((struct flow *)a)->el)
      free(((struct flow *)a)->el);
    break;

  default:
    printf("internal error: free bad node %c\n", a->nodetype);
  }

  free(a); /* always free the node itself */
}

void yyerror(char *s, ...)
{
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

int main()
{
  printf("> \n");
  return yyparse();
}

/* debugging: dump out an AST */
int debug = 1;
void dumpast(struct ast *a, int level)
{

  printf("%*s", 2 * level, ""); /* indent to this level */
  level++;

  if (!a)
  {
    // printf("NULL\n");
    return;
  }

  switch (a->nodetype)
  {
    /* double */
  case 'D':
    printf("double %4.4g\n", ((struct doubleval *)a)->number);
    break;
    /* integer */
  case 'K':
    printf("int %d\n", ((struct intval *)a)->number);
    break;
    /* edge */
  case 'E':
    printf("edge %d -> %d\n", ((struct edgeval *)a)->source, ((struct edgeval *)a)->dest);
    break;
    /* string */
  case 'S':
    printf("string  %s\n", ((struct strval *)a)->s);
    break;
    /*array */
  case 'A':
    printf("array \n");
    struct ast *l = ((struct arrayval *)a)->l;
    while (l->nodetype == 'L')
    {
      dumpast(l->l, level);
      l = l->r;
    }
    dumpast(l, level);
    break;
    /* function def */
  case 'P':
    printf("defined %s\n", ((struct funcval *)a)->f->name);
    // struct symlist* syms=f->syms;
    // while(syms){
    //   dumpast(syms->sym->type,level)
    // }
    dumpast(((struct funcval *)a)->f->func, level);
    break;
    /* name reference */
  case 'N':
    printf("var %s\n", ((struct symref *)a)->s->name);
    break;

    /* assignment */
  case '=':
    printf("= %s\n", ((struct symref *)a)->s->name);
    dumpast(((struct symasgn *)a)->v, level);
    return;

    /* assignment ops */
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
    printf("%s %s\n", map_asgn_op(a->nodetype), ((struct symref *)a)->s->name);
    dumpast(((struct symasgn *)a)->v, level);
    return;

    /* expressions */
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
  case 'L':
  case 'T':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
    printf("%c\n", a->nodetype);
    int tmplevel = level;
    dumpast(a->l, level);
    level = tmplevel;
    dumpast(a->r, level);
    level = tmplevel;
    return;

  case 1000:
  case 1001:
    printf("binop %s\n", map_logical_op(a->nodetype));
    int tmplevel1 = level;
    dumpast(a->l, level);
    level = tmplevel1;
    dumpast(a->r, level);
    level = tmplevel1;
    return;

  case '|':
  case 'M':
    printf("unop %c\n", a->nodetype);
    dumpast(a->l, level);
    return;

  case 'I':
  case 'W':
    printf("flow %c\n", a->nodetype);
    dumpast(((struct flow *)a)->cond, level);
    if (((struct flow *)a)->tl)
      dumpast(((struct flow *)a)->tl, level);
    if (((struct flow *)a)->el)
      dumpast(((struct flow *)a)->el, level);
    return;

  case 'R':
    printf("for loop %c\n", a->nodetype);
    dumpast(((struct for_loop *)a)->init, level);
    if (((struct for_loop *)a)->cond)
      dumpast(((struct flow *)a)->cond, level);
    if (((struct for_loop *)a)->inc)
      dumpast(((struct for_loop *)a)->inc, level);
    if (((struct for_loop *)a)->stmt)
      dumpast(((struct for_loop *)a)->stmt, level);
    return;

  case 'B':
    printf("rfor loop %c\n", a->nodetype);
    struct rfor_loop *tmp = (struct rfor_loop *)a;
    if (tmp->stmt)
      dumpast(tmp->stmt, level);
    return;

  case 'F':
    printf("builtin %d\n", ((struct fncall *)a)->functype);
    dumpast(a->l, level);
    return;

  case 'C':
    printf("call %s\n", ((struct ufncall *)a)->s->name);
    dumpast(a->l, level);
    return;

  // Graphs parsing
  case 16:
    printf("BFS: \n");
    struct bfs *tmp_bfs = (struct bfs *)a;
    printf("Start Node : %s\n", tmp_bfs->stSym->name);
    printf("Iteration on list %s using %s\n", tmp_bfs->g->name, tmp_bfs->d->name);

    if (tmp_bfs->stmt)
      dumpast(tmp_bfs->stmt, level);
    return;

  case 17:
    printf("DFS: \n");
    struct dfs *tmp_dfs = (struct dfs *)a;
    printf("Start Node : %s\n", tmp_dfs->stSym->name);
    printf("Iteration on list %s using %s\n", tmp_dfs->g->name, tmp_dfs->d->name);

    if (tmp_dfs->stmt)
      dumpast(tmp_dfs->stmt, level);
    return;

  /* symlist */
  case 200:
    printf("Symlist : ");
    struct symlist *tmp1 = malloc(sizeof(struct symlist));
    tmp1 = ((struct symlistdef *)a)->sl;
    while (tmp1)
    {
      printf("%s ", tmp1->sym->name);
      tmp1 = tmp1->next;
    }
    printf("\n");
    return;

  default:
    printf("bad %c\n", a->nodetype);
    return;
  }
}

char *assignment_op_map[] = {"+=", "-=", "*=", "/=", "@=", "**="};
char *logical_op_map[] = {"||", "&&"};

char *map_asgn_op(char ch)
{
  if (ch - 'a' >= 6 || ch < 'a')
    "bad";

  return assignment_op_map[ch - 'a'];
}

char *map_logical_op(int val)
{
  if (val == 1000 || val == 1001)
  {
    return logical_op_map[val - 1000];
  }


  return "BADDD";
}

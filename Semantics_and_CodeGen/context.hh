#pragma once
#include <vector>
#include <string>
#include "types.h"

#include <map>
#include "parser.hh"
#include "GrFlexLexer.hh"

class lexcontext
{
public:
  yy::location loc;
  std::vector<std::map<std::string, identifier>> scopes;
  std::vector<common_list> storage;
  unsigned tempcounter = 0;
  function fun;
  type_name temptype = type_name::INT;
  std::vector<std::pair<yy::location, std::string>> error_list;
  std::size_t temporary = 1;

  GrFlexLexer lexer;

  lexcontext() {
    scopes.emplace_back();
    scopes.back()["print"] = identifier(id_type::function, type_name::FUNC, 0, "print");
  }
  virtual ~lexcontext() {}
  const identifier &define(const std::string &name, identifier &&f);
  node def(const std::string &name);
  node defun(const std::string &name);
  node defparam(const std::string &name, type_name type);
  node temp();
  node use(const std::string &name);
  void add_function(std::string &&name, node &&code, type_name ret);
  void add_decl(node &&decl);
  void operator++(); // Enter scope
  void operator--(); // Exit scope

  void dump_ast();
  void func1(function F);
  void func2(node N, int level);
  void func3(node N, int level);

  void error(const yy::location &l, const std::string &m);
  void error(const std::string &m);
};
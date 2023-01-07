#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../include/lexExtern.h"

class ExprAST;
class NumberExprAST;
class VariableExprAST;
class VarExprAST;
class BinaryExprAST;
class UnaryExprAST;
class CallExprAST;
class PrototypeAST;
class FunctionAST;
class IfExprAST;
class ForExprAST;
class GenerateCode;

extern int justused;
extern std::map<std::string, int> varIds;
extern std::map<std::string, ExprAST*> definedFunctions;
extern int id;

class ExprAST {
 public:
  SourceLocation loc;

 public:
  ExprAST(SourceLocation loc = curLoc) : loc(loc) {}
  virtual ~ExprAST() = default;
  int getLine() const { return loc.line; }
  int getCol() const { return loc.col; }
  virtual void traverse() {}
};

class NumberExprAST : public ExprAST {
 public:
  double val;

 public:
  NumberExprAST(double val) : val(val) {}
  void traverse() override {
    id++;
    std::cout << "%" << id << " : (const number) " << val << std::endl;
    justused = id;
  }
};

class VariableExprAST : public ExprAST {
 public:
  std::string name;

 public:
  VariableExprAST(SourceLocation loc, const std::string& name)
      : ExprAST(loc), name(name) {}
  void traverse() override {
    int varId;
    if (varIds.contains(name)) {
      varId = varIds[name];
      justused = varId;
      //std::cout << "%" << varId << " : " << name << " (variable)" << std::endl;
      return;
    }
    std::cout << "Error: Variable undeclared. Proceeding by inserting a dummy "
                 "declaration."
              << std::endl;
    id++;
    varId = id;
    justused = varId;
    varIds.insert(std::pair<std::string, int>(name, varId));
    std::cout << "%" << varId << " : " << name << " (variable)" << std::endl;
  }
};

class VarExprAST : public ExprAST {
 public:
  std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> vars;
  std::unique_ptr<ExprAST> body;

 public:
  VarExprAST(std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> vars,
             std::unique_ptr<ExprAST> body)
      : vars(std::move(vars)), body(std::move(body)) {}
  void traverse() override {
    std::cout << "Var Expression:" << std::endl;
    std::vector<std::pair<std::string, int>> oldVarIds;
    for (auto& var : vars) {
      if (varIds.contains(var.first)) {
        oldVarIds.push_back(
            std::pair<std::string, int>(var.first, varIds[var.first]));
        auto it = varIds.find(var.first);
        varIds.erase(it);
      }
      var.second->traverse();
      id++;
      int varId = id;
      std::cout << "%" << varId << " : " << var.first << " (variable)"
                << std::endl;
      std::cout << "%" << varId << " = %" << justused << std::endl;
      varIds.insert(std::pair<std::string, int>(var.first, varId));
    }
    body->traverse();
    for (auto& var : vars) {
      auto it = varIds.find(var.first);
      varIds.erase(it);
    }
    for (auto& var : oldVarIds) {
      varIds.insert(std::pair<std::string, int>(var.first, var.second));
    }
  }
};

class BinaryExprAST : public ExprAST {
 public:
  char op;
  std::unique_ptr<ExprAST> LHS, RHS;

 public:
  BinaryExprAST(SourceLocation loc, char op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
      : ExprAST(loc), op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
  void traverse() override {
    std::cout << "Binary Expression:" << std::endl;
    LHS->traverse();
    int lhs = justused;
    RHS->traverse();
    int rhs = justused;
    id++;
    std::cout << "%" << id << " = %" << lhs << " " << op << " %" << rhs
              << std::endl;
    justused = id;
  }
};

class UnaryExprAST : public ExprAST {
 public:
  char op;
  std::unique_ptr<ExprAST> operand;

 public:
  UnaryExprAST(char op, std::unique_ptr<ExprAST> operand)
      : op(op), operand(std::move(operand)) {}
  void traverse() override {
    std::cout << "Unary Expression:" << std::endl;
    operand->traverse();
    int op = justused;
    id++;
    std::cout << "%" << id << " = " << op << std::endl;
    justused = id;
  }
};

class CallExprAST : public ExprAST {
 public:
  std::string callee;
  std::vector<std::unique_ptr<ExprAST>> args;

 public:
  CallExprAST(SourceLocation loc, const std::string& callee,
              std::vector<std::unique_ptr<ExprAST>> args)
      : ExprAST(loc), callee(callee), args(std::move(args)) {}
  void traverse() override {
    std::cout << "Call Expression:" << std::endl;
    if(!definedFunctions.contains(callee))
    {
      std::cout << "Error: Function " << callee << " not defined. Proceeding assuming a definition exists." << std::endl;
    }
    std::vector<int> argIds;
    for (auto& arg : args) {
      arg->traverse();
      argIds.push_back(justused);
    }
    id++;
    int callId = id;
    std::cout << "%" << callId << " = call " << callee << "(";
    if(argIds.size() > 0)
    {
      std::cout << "%" << argIds[0];
    }
    for (int i = 1; i < argIds.size(); i++) {
      std::cout << ", %" << argIds[i];
    }
    std::cout << ")" << std::endl;
    justused = callId;
  }
};

class PrototypeAST : public ExprAST {
 public:
  std::string name;
  std::vector<std::unique_ptr<ExprAST>> args;
  std::vector<std::string> argString;
  bool isOperator;
  unsigned precedence;
  int line;

 public:
  PrototypeAST(SourceLocation loc, const std::string& name,
               std::vector<std::unique_ptr<ExprAST>> args,
               std::vector<std::string> argString, bool isOperator = false,
               unsigned precedence = 0)
      : name(name),
        args(std::move(args)),
        argString(std::move(argString)),
        isOperator(isOperator),
        precedence(precedence),
        line(loc.line) {}
  const std::string getName() const { return name; }
  bool isUnaryOp() { return (isOperator && (args.size() == 1)); }
  bool isBinaryOp() { return (isOperator && (args.size() == 2)); }
  char getOperatorName() { return name[name.size() - 1]; }
  int getLine() const { return line; }
  void traverse() override {
    std::cout << "Prototype:" << std::endl;
    std::cout << "Function: " << name << std::endl;
    std::cout << "Arguments: ";
    for (auto& arg : argString) {
      id++;
      int argId = id;
      std::cout << "%" << argId << " : " << arg << " (variable) ";
      varIds.insert(std::pair<std::string, int>(arg, argId));
    }
    std::cout << std::endl;
  }
};

class FunctionAST : public ExprAST {
 public:
  std::unique_ptr<PrototypeAST> proto;
  std::unique_ptr<ExprAST> body;

 public:
  FunctionAST(std::unique_ptr<PrototypeAST> proto,
              std::unique_ptr<ExprAST> body)
      : proto(std::move(proto)), body(std::move(body)) {}
  void traverse() override {
    std::cout << "Function:" << std::endl;
    proto->traverse();
    body->traverse();
  }
};

class IfExprAST : public ExprAST {
 public:
  std::unique_ptr<ExprAST> cond, then, _else;

 public:
  IfExprAST(SourceLocation loc, std::unique_ptr<ExprAST> cond,
            std::unique_ptr<ExprAST> then, std::unique_ptr<ExprAST> _else)
      : ExprAST(loc),
        cond(std::move(cond)),
        then(std::move(then)),
        _else(std::move(_else)) {}
  void traverse() override {
    std::cout << "If Expression:" << std::endl;
    cond->traverse();
    int condId = justused;
    then->traverse();
    int thenId = justused;
    _else->traverse();
    int elseId = justused;
    id++;
    int ifId = id;
    std::cout << "%" << ifId << " = if %" << condId << " then %" << thenId
              << " else %" << elseId << std::endl;
    justused = ifId;
  }
};

class ForExprAST : public ExprAST {
 public:
  std::string varName;
  std::unique_ptr<ExprAST> start, cond, step, body;

 public:
  ForExprAST(const std::string& varName, std::unique_ptr<ExprAST> start,
             std::unique_ptr<ExprAST> cond, std::unique_ptr<ExprAST> step,
             std::unique_ptr<ExprAST> body)
      : varName(varName),
        start(std::move(start)),
        cond(std::move(cond)),
        step(std::move(step)),
        body(std::move(body)) {}
  void traverse() override {
    std::cout << "For Expression:" << std::endl;
    start->traverse();
    int startId = justused;
    cond->traverse();
    int condId = justused;
    step->traverse();
    int stepId = justused;
    body->traverse();
    int bodyId = justused;
    id++;
    int forId = id;
    std::cout << "%" << forId << " = for " << varName << " = %" << startId
              << " to %" << condId << " step %" << stepId << " do %" << bodyId
              << std::endl;
    justused = forId;
  }
};

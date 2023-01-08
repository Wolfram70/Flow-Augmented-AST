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

extern int justused;
extern std::vector<ExprAST*> justBefore;
extern std::map<std::string, int> varIds;
extern std::map<std::string, ExprAST*> definedFunctions;
extern int id;
extern std::map<std::string, int> neededFunctions;

class ExprAST {
 public:
  SourceLocation loc;
  std::vector<ExprAST*> controlEdgesTo;
  std::vector<ExprAST*> controlEdgesFrom;
  std::string nodeName;
  int controlTo;
  ExprAST* lastNode = this;
  ExprAST* startNode = this;

 public:
  ExprAST(SourceLocation loc = curLoc) : loc(loc) {}
  virtual ~ExprAST() = default;
  int getLine() const { return loc.line; }
  int getCol() const { return loc.col; }
  virtual void traverse() {}
  virtual void showControl()
  {
    std::cout << nodeName << " -> ";
    if(controlEdgesTo.size() > 0) controlTo = controlTo % controlEdgesTo.size();
    if(controlTo < controlEdgesTo.size()) controlEdgesTo[controlTo++]->showControl();
  }
};

class NumberExprAST : public ExprAST {
 public:
  double val;

 public:
  NumberExprAST(double val) : val(val) {
    nodeName = "NumberExprAST";
    controlTo = 0;
  }
  void traverse() override {
    for (auto jB : justBefore) {
      controlEdgesFrom.push_back(jB);
      jB->controlEdgesTo.push_back(this);
    }
    id++;
    std::cout << "%" << id << " : (const number) " << val << std::endl;
    justused = id;
    justBefore.clear();
    justBefore.push_back(this);
    lastNode = this;
    startNode = this;
  }
  void showControl() override {
    std::cout << nodeName << " (" << val << ") -> ";
    if (controlEdgesTo.size() > 0)
      if (controlEdgesTo.size() > 0)
        controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
  }
};

class VariableExprAST : public ExprAST {
 public:
  std::string name;

 public:
  VariableExprAST(SourceLocation loc, const std::string& name)
      : ExprAST(loc), name(name) {
    nodeName = "VariableExprAST";
    controlTo = 0;
  }
  void traverse() override {
    int varId;
    if (varIds.contains(name)) {
      for (auto jB : justBefore) {
        controlEdgesFrom.push_back(jB);
        jB->controlEdgesTo.push_back(this);
      }
      varId = varIds[name];
      justused = varId;
      justBefore.clear();
      justBefore.push_back(this);
      // std::cout << "%" << varId << " : " << name << " (variable)" <<
      // std::endl;
      lastNode = this;
      startNode = this;
      return;
    }
    for (auto jB : justBefore) {
      controlEdgesFrom.push_back(jB);
      jB->controlEdgesTo.push_back(this);
    }
    std::cout << "Error: Variable undeclared. Proceeding by inserting a dummy "
                 "declaration."
              << std::endl;
    id++;
    varId = id;
    justused = varId;
    varIds.insert(std::pair<std::string, int>(name, varId));
    std::cout << "%" << varId << " : " << name << " (variable)" << std::endl;
    justBefore.clear();
    justBefore.push_back(this);
    lastNode = this;
    startNode = this;
  }
  void showControl() override {
    std::cout << nodeName << " (" << name << ") -> ";
    if (controlEdgesTo.size() > 0)
      if (controlEdgesTo.size() > 0)
        controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
  }
};

class VarExprAST : public ExprAST {
 public:
  std::vector<std::pair<std::string, ExprAST*>> vars;
  ExprAST* body;

 public:
  VarExprAST(std::vector<std::pair<std::string, ExprAST*>> vars, ExprAST* body)
      : vars(std::move(vars)), body(body) {
    nodeName = "VarExprAST";
    controlTo = 0;
  }
  void traverse() override {
    for (auto jB : justBefore) {
      controlEdgesFrom.push_back(jB);
      jB->controlEdgesTo.push_back(this);
    }
    justBefore.clear();
    justBefore.push_back(this);
    startNode = this;
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
    lastNode = body->lastNode;
  }
  void showControl() override {
    std::cout << nodeName << " -> ";
    if (controlEdgesTo.size() > 0)
      controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
  }
};

class BinaryExprAST : public ExprAST {
 public:
  char op;
  ExprAST *LHS, *RHS;

 public:
  BinaryExprAST(SourceLocation loc, char op, ExprAST* LHS, ExprAST* RHS)
      : ExprAST(loc), op(op), LHS(LHS), RHS(RHS) {
    nodeName = "BinaryExprAST";
    controlTo = 0;
  }
  void traverse() override {
    std::cout << "Binary Expression:" << std::endl;
    if (op == ':') {
      LHS->traverse();
      int lhs = justused;
      RHS->traverse();
      int rhs = justused;
      id++;
      std::cout << "%" << id << " = %" << lhs << " " << op << " %" << rhs
                << std::endl;
      justused = id;
      for (auto jB : justBefore) {
        controlEdgesFrom.push_back(jB);
        jB->controlEdgesTo.push_back(this);
      }
      justBefore.clear();
      justBefore.push_back(this);
      startNode= LHS->startNode;
      lastNode = this;
      return;
    }
    LHS->traverse();
    int lhs = justused;
    RHS->traverse();
    int rhs = justused;
    id++;
    std::cout << "%" << id << " = %" << lhs << " " << op << " %" << rhs
              << std::endl;
    justused = id;
    for (auto jB : justBefore) {
      controlEdgesFrom.push_back(jB);
      jB->controlEdgesTo.push_back(this);
    }
    justBefore.clear();
    justBefore.push_back(this);
    startNode= LHS->startNode;
    lastNode = this;
  }
  void showControl() override {
    std::cout << nodeName << " (" << op << ") -> ";
    if (controlEdgesTo.size() > 0)
      controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
  }
};

class UnaryExprAST : public ExprAST {
 public:
  char op;
  ExprAST* operand;

 public:
  UnaryExprAST(char op, ExprAST* operand) : op(op), operand(operand) {
    nodeName = "UnaryExprAST";
    controlTo = 0;
  }
  void traverse() override {
    std::cout << "Unary Expression:" << std::endl;
    operand->traverse();
    for(auto jB : justBefore)
    {
      controlEdgesFrom.push_back(jB);
      jB->controlEdgesTo.push_back(this);
    }
    justBefore.clear();
    justBefore.push_back(this);
    startNode = operand->startNode;
    lastNode = this;
    int op = justused;
    id++;
    std::cout << "%" << id << " = " << op << std::endl;
    justused = id;
  }
  void showControl() override {
    std::cout << nodeName << " (" << op << ") -> ";
    if (controlEdgesTo.size() > 0)
      controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
  }
};

class CallExprAST : public ExprAST {
 public:
  std::string callee;
  std::vector<ExprAST*> args;

 public:
  CallExprAST(SourceLocation loc, const std::string& callee,
              std::vector<ExprAST*> args)
      : ExprAST(loc), callee(callee), args(std::move(args)) {
    nodeName = "CallExprAST";
    controlTo = 0;
  }
  void traverse() override {
    std::cout << "Call Expression:" << std::endl;
    std::vector<int> argIds;
    lastNode = this;
     for (auto& arg : args) {
       arg->traverse();
       argIds.push_back(justused);
     }
     if(args.size() > 0) startNode = args[0]->startNode;
     else startNode = this;
     for (auto jB : justBefore) {
       controlEdgesFrom.push_back(jB);
       jB->controlEdgesTo.push_back(this);
     }
     justBefore.clear();
     justBefore.push_back(lastNode);
    if (!definedFunctions.contains(callee)) {
      std::cout << "Error: Function " << callee
                << " not defined. Proceeding assuming a definition exists."
                << std::endl;
    } else {
      controlEdgesTo.push_back(definedFunctions[callee]);
      definedFunctions[callee]->controlEdgesFrom.push_back(this);
      definedFunctions[callee]->lastNode->controlEdgesTo.push_back(this);
      controlEdgesFrom.push_back(definedFunctions[callee]->lastNode);
    }
    id++;
    int callId = id;
    std::cout << "%" << callId << " = call " << callee << "(";
    if (argIds.size() > 0) {
      std::cout << "%" << argIds[0];
    }
    for (int i = 1; i < argIds.size(); i++) {
      std::cout << ", %" << argIds[i];
    }
    std::cout << ")" << std::endl;
    justused = callId;
  }
  void showControl() override {
    std::cout << nodeName << " (" << callee << ") -> ";
    if (!neededFunctions.contains(callee))
      neededFunctions.insert(std::pair<std::string, int>(callee, 0));
    if (controlEdgesTo.size() > 0)
      controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
  }
};

class PrototypeAST : public ExprAST {
 public:
  std::string name;
  std::vector<ExprAST*> args;
  std::vector<std::string> argString;
  bool isOperator;
  unsigned precedence;
  int line;

 public:
  PrototypeAST(SourceLocation loc, const std::string& name,
               std::vector<ExprAST*> args, std::vector<std::string> argString,
               bool isOperator = false, unsigned precedence = 0)
      : name(name),
        args(std::move(args)),
        argString(std::move(argString)),
        isOperator(isOperator),
        precedence(precedence),
        line(loc.line) {
    nodeName = "PrototypeAST";
    controlTo = 0;
  }
  const std::string getName() const { return name; }
  bool isUnaryOp() { return (isOperator && (args.size() == 1)); }
  bool isBinaryOp() { return (isOperator && (args.size() == 2)); }
  char getOperatorName() { return name[name.size() - 1]; }
  int getLine() const { return line; }
  void traverse() override {
    for (auto jB : justBefore) {
      controlEdgesFrom.push_back(jB);
      jB->controlEdgesTo.push_back(this);
    }
    startNode = this;
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
    justBefore.clear();
    justBefore.push_back(this);
    lastNode = this;
  }
  void showControl() override {
    std::cout << nodeName << " (" << name << ") -> ";
    if (controlEdgesTo.size() > 0)
      controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
  }
};

class FunctionAST : public ExprAST {
 public:
  PrototypeAST* proto;
  ExprAST* body;

 public:
  FunctionAST(PrototypeAST* proto, ExprAST* body) : proto(proto), body(body) {
    nodeName = "FunctionAST";
    controlTo = 0;
  }
  void traverse() override {
    std::cout << "Function:" << std::endl;
    justBefore.clear();
    justBefore.push_back(this);
    startNode = this;
    proto->traverse();
    body->traverse();
    lastNode = body->lastNode;
    justBefore.clear();
  }
  void showControl() override {
    std::cout << nodeName << " (" << proto->name << ") -> ";
    if (controlEdgesTo.size() > 0)
      controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
  }
};

class IfExprAST : public ExprAST {
 public:
  ExprAST *cond, *then, *_else, *ifcont;

 public:
  IfExprAST(SourceLocation loc, ExprAST* cond, ExprAST* then, ExprAST* _else)
      : ExprAST(loc), cond(cond), then(then), _else(_else) {
    nodeName = "IfExprAST";
    controlTo = 0;
    ifcont = new ExprAST(loc);
    ifcont->nodeName = "IfCont";
    ifcont->controlTo = 0;
  }
  void traverse() override {
    std::cout << "If Expression:" << std::endl;
    for(auto jB : justBefore) {
      controlEdgesFrom.push_back(jB);
      jB->controlEdgesTo.push_back(this);
    }
    justBefore.clear();
    justBefore.push_back(this);
    startNode = this;
    cond->traverse();
    int condId = justused;
    then->traverse();
    int thenId = justused;
    _else->traverse();
    /*_else->controlEdgesFrom[_else->controlEdgesFrom.size() - 1]
        ->controlEdgesTo.pop_back();
    _else->controlEdgesFrom.pop_back();*/

    //I WAS HERE (RESOLVING THIS IF THUNG)  (MIGHT NEED START NODE AS WELL)
    int elseId = justused;
    for(int i = then->lastNode->controlEdgesTo.size() - 1; i > -1; i--) {
      if(then->lastNode->controlEdgesTo[i] == _else->startNode) {
        then->lastNode->controlEdgesTo.erase(then->lastNode->controlEdgesTo.begin() + i);
        for(int j =  _else->startNode->controlEdgesFrom.size() - 1; j > -1; j--) {
          if(_else->startNode->controlEdgesFrom[j] == then->lastNode) {
            _else->startNode->controlEdgesFrom.erase(_else->startNode->controlEdgesFrom.begin() + j);
            break;
          }
        }
        break;
      }
    }
    cond->controlEdgesTo.push_back(_else->startNode);
    _else->startNode->controlEdgesFrom.push_back(cond);
    then->lastNode->controlEdgesTo.push_back(ifcont);
    ifcont->controlEdgesFrom.push_back(then->lastNode);
    _else->lastNode->controlEdgesTo.push_back(ifcont);
    ifcont->controlEdgesFrom.push_back(_else->lastNode);
    id++;
    int ifId = id;
    std::cout << "%" << ifId << " = if %" << condId << " then %" << thenId
              << " else %" << elseId << std::endl;
    justused = ifId;
    justBefore.clear();
    //For lastnode, might need a node where then and else meet after control end
    lastNode = ifcont;
    justBefore.push_back(ifcont);
  }
  void showControl() override {
    std::cout << nodeName << std::endl;
    std::cout << "Condition (line: " << cond->loc.line << ") True:\n-> ";
    if (controlEdgesTo.size() > 0)
      controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
    std::cout << " (program exit)";
    std::cout << "\nCondition (line: " << cond->loc.line << ") False:\n-> ";
    if (controlEdgesTo.size() > 0)
      controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
  }
};

class ForExprAST : public ExprAST {
 public:
  std::string varName;
  ExprAST *start, *cond, *step, *body;

 public:
  ForExprAST(const std::string& varName, ExprAST* start, ExprAST* cond,
             ExprAST* step, ExprAST* body)
      : varName(varName), start(start), cond(cond), step(step), body(body) {
    nodeName = "ForExprAST";
    controlTo = 0;
  }
  void traverse() override {
    std::cout << "For Expression:" << std::endl;
    start->traverse();
    int startId = justused;
    cond->traverse();
    int condId = justused;
    body->traverse();
    int bodyId = justused;
    step->traverse();
    int stepId = justused;
    step->controlEdgesTo.push_back(cond->startNode);
    cond->controlEdgesFrom.push_back(&(*step));
    id++;
    int forId = id;
    std::cout << "%" << forId << " = for " << varName << " = %" << startId
              << " to %" << condId << " step %" << stepId << " do %" << bodyId
              << std::endl;
    justused = forId;
    justBefore.clear();
    justBefore.push_back(cond->lastNode);
    lastNode = cond->lastNode;
  }
  void showControl() override {
    std::cout << nodeName << " -> " << std::endl;
    if (controlEdgesTo.size() > 0)
      controlTo = controlTo % controlEdgesTo.size();
    if (controlTo < controlEdgesTo.size())
      controlEdgesTo[controlTo++]->showControl();
  }
};

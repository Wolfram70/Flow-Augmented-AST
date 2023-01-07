#include "../include/parser.h"

#include <cstdio>

extern int getToken();

char curTok;

std::map<char, int> binOpPrecedence;

std::map<std::string, PrototypeAST*> functionProtos;
int justused = 0;
std::vector<ExprAST*> justBefore;
int id = 0;

std::map<std::string, int> varIds;
std::map<std::string, ExprAST*> definedFunctions;
std::map<std::string, int> neededFunctions;

int tempResolveTopLvlExpr = 0;

ExprAST* parseNumberExpr();
ExprAST* parseParenExpr();
ExprAST* parsePrimary();
ExprAST* parseBinOpRHS(int exprPrec,
                                       ExprAST* LHS);
ExprAST* parseExpression();
ExprAST* parseIdentifierExpr();
PrototypeAST* parseProtoype();
FunctionAST* parseDefinition();
PrototypeAST* parseExtern();
ExprAST* parseUnaryExpr();
FunctionAST* parseTopLvlExpr();
ExprAST* parseIfExpr();
ExprAST* parseForExpr();
ExprAST* parseVarExpr();

int getNextToken() { return curTok = getToken(); }

ExprAST* logError(const char *str) {
  fprintf(stderr, "LogError: %s\n", str);
  return nullptr;
}

PrototypeAST* logErrorP(const char *str) {
  logError(str);
  return nullptr;
}

ExprAST* parseNumberExpr() {
  auto result = new NumberExprAST(numVal);
  getNextToken();
  return result;
}

ExprAST* parseParenExpr() {
  getNextToken();
  auto V = parseExpression();
  if (!V) {
    return nullptr;
  }

  if (curTok != ')') {
    return logError("Expected ')'");
  } else {
    getNextToken();
    return V;
  }
}

ExprAST* parsePrimary() {
  switch (curTok) {
    case tok_identifier:
      return parseIdentifierExpr();
    case tok_number:
      return parseNumberExpr();
    case '(':
      return parseParenExpr();
    case tok_if:
      return parseIfExpr();
    case tok_for:
      return parseForExpr();
    case tok_var:
      return parseVarExpr();
    default:
      return logError("Unknown token when expecting an expression");
  }
}

ExprAST* parseUnaryExpr() {
  if (!isascii(curTok) || curTok == '(') {
    return parsePrimary();
  }

  int opc = curTok;
  getNextToken();

  if (auto operand = parseUnaryExpr()) {
    return new UnaryExprAST(opc, operand);
  }
  return nullptr;
}

ExprAST* parseForExpr() {
  getNextToken();

  if (curTok != tok_identifier) {
    logError("Expected a variable name after for");
    return nullptr;
  }

  std::string idName = identifierStr;
  getNextToken();

  if (curTok != '=') {
    logError("Expected an '=' after for");
    return nullptr;
  }
  getNextToken();

  auto start = parseExpression();
  if (!start) {
    return nullptr;
  }

  if (curTok != tok_when) {
    logError("Expected when after for");
    return nullptr;
  }
  getNextToken();

  auto cond = parseExpression();
  if (!cond) {
    return nullptr;
  }

  ExprAST* step;
  if (curTok == tok_inc) {
    getNextToken();
    step = parseExpression();
    if (!step) {
      return nullptr;
    }
  }

  if (curTok != tok_do) {
    logError("Expected do after for");
    return nullptr;
  }
  getNextToken();

  if (curTok != '(') {
    logError("Expected '(' after for ... ");
  }

  auto body = parseParenExpr();
  if (!body) {
    return nullptr;
  }

  return new ForExprAST(idName, start, cond,
                        step, body);
}

ExprAST* parseVarExpr() {
  getNextToken();

  std::vector<std::pair<std::string, ExprAST*>> vars;

  if (curTok != tok_identifier) {
    return logError("Expected identifier after var");
  }

  while (true) {
    std::string name = identifierStr;
    getNextToken();

    ExprAST* init;
    if (curTok == '=') {
      getNextToken();

      init = parseExpression();
      if (!init) {
        return nullptr;
      }
    }

    vars.push_back(std::make_pair(name, init));

    if (curTok != ',') {
      break;
    }

    getNextToken();

    if (curTok != tok_identifier) {
      return logError("Expected identifier after var");
    }
  }

  if (curTok != tok_in) {
    return logError("Expected in after var");
  }
  getNextToken();

  auto body = parseExpression();
  if (!body) {
    return nullptr;
  }

  return new VarExprAST(vars, body);
}

int getTokPrecedence() {
  if (!isascii(curTok)) {
    return -1;
  }

  int tokPrec = binOpPrecedence[curTok];

  if (tokPrec <= 0) {
    return -1;
  }

  return tokPrec;
}

ExprAST* parseBinOpRHS(int exprPrec,
                                       ExprAST* LHS) {
  while (true) {
    int tokPrec = getTokPrecedence();

    if (tokPrec < exprPrec)  // to check if this is really a binary operator as
                             // invalid tokens have precedence of -1
    {
      return std::move(LHS);
    }

    int binOp = curTok;
    SourceLocation binLoc = curLoc;
    getNextToken();

    auto RHS = parseUnaryExpr();

    if (!RHS) {
      return nullptr;
    }

    int nextPrec = getTokPrecedence();

    if (tokPrec < nextPrec) {
      RHS = parseBinOpRHS(tokPrec + 1, std::move(RHS));

      if (!RHS) {
        return nullptr;
      }
    }

    LHS = new BinaryExprAST(binLoc, binOp, LHS, RHS);
  }
}

ExprAST* parseExpression() {
  auto LHS = parseUnaryExpr();

  if (!LHS) {
    return nullptr;
  }

  return parseBinOpRHS(0, LHS);
}

ExprAST* parseIdentifierExpr() {
  std::string idName = identifierStr;
  SourceLocation litLoc = curLoc;

  getNextToken();
  if (curTok != '(') {
    return new VariableExprAST(litLoc, idName);
  }
  getNextToken();

  std::vector<ExprAST*> args;

  if (curTok != ')') {
    while (true) {
      if (auto arg = parseExpression()) {
        args.push_back(arg);
      } else {
        return nullptr;
      }

      if (curTok == ')') {
        break;
      }

      if (curTok != ',') {
        return logError("Expected ',' after an argument in function call");
      }

      getNextToken();
    }
  }

  getNextToken();

  return new CallExprAST(litLoc, idName, args);
}

ExprAST* parseIfExpr() {
  SourceLocation ifLoc = curLoc;

  getNextToken();

  auto cond = parseExpression();
  if (!cond) {
    return nullptr;
  }

  if (curTok != tok_then) {
    logError("Expected then");
    return nullptr;
  }
  getNextToken();

  if (curTok != '(') {
    logError("Expected '(' after then");
    return nullptr;
  }

  auto then = parseParenExpr();
  if (!then) {
    return nullptr;
  }

  if (curTok != tok_else) {
    logError("Expected else");
    return nullptr;
  }
  getNextToken();

  if (curTok != '(') {
    logError("Expected '(' after else");
    return nullptr;
  }

  auto _else = parseParenExpr();
  if (!_else) {
    return nullptr;
  }

  return new IfExprAST(ifLoc, cond, then, _else);
}

PrototypeAST* parseProtoype() {
  std::string fnName;

  SourceLocation fnLoc = curLoc;

  unsigned kind = 0;
  unsigned binaryPrecedence = 30;

  switch (curTok) {
    default:
      return logErrorP("Expected function name in prototype");
      break;
    case tok_identifier:
      fnName = identifierStr;
      kind = 0;
      getNextToken();
      break;
    case tok_binary:
      getNextToken();
      if (!isascii(curTok)) {
        return logErrorP("Expected binary operator");
      }
      fnName = "binary";
      fnName.push_back(curTok);
      kind = 2;
      getNextToken();

      if (curTok == tok_number) {
        if (numVal < 1 || numVal > 100) {
          return logErrorP("Precedence must be from 1 to 100");
        }
        binaryPrecedence = (unsigned)numVal;
        getNextToken();
      }
      break;
    case tok_unary:
      getNextToken();
      if (!isascii(curTok)) {
        return logErrorP("Expected unary operator");
      }
      fnName = "unary";
      fnName.push_back(curTok);
      kind = 1;
      getNextToken();
  }

  if (curTok != '(') {
    return logErrorP("Expected '(' in function prototype");
  }

  std::vector<ExprAST*> argNames;
  std::vector<std::string> argString;

  int tok = getNextToken();

  while (tok == tok_identifier) {
    argNames.push_back(new VariableExprAST(fnLoc, identifierStr));
    argString.push_back(identifierStr);

    tok = getNextToken();
    if (tok != ')') {
      if (tok != ',') {
        return logErrorP(
            "Expected comma after formal argument in function definition");
      }
      tok = getNextToken();
    }
  }

  if (curTok != ')') {
    return logErrorP("Expected ')' in function prototype");
  }
  getNextToken();

  if (kind && (argNames.size() != kind)) {
    return logErrorP("Invalid number of operands for an operator");
  }

  return new PrototypeAST(fnLoc, fnName, argNames, argString, kind != 0, binaryPrecedence);
}

FunctionAST* parseDefinition() {
  getNextToken();

  auto proto = parseProtoype();
  if (!proto) {
    return nullptr;
  }

  if (auto exp = parseExpression()) {
    return new FunctionAST(proto, exp);
  }

  return nullptr;
}

PrototypeAST* parseExtern() {
  getNextToken();

  return parseProtoype();
}

FunctionAST* parseTopLvlExpr() {
  SourceLocation exprLoc = curLoc;

  if (auto exp = parseExpression()) {
    auto proto = new PrototypeAST(
        exprLoc, "main", std::vector<ExprAST*>(),
        std::vector<std::string>());
    return new FunctionAST(proto, exp);
  }

  return nullptr;
}

bool genDefinition()
{
  if(auto fn = parseDefinition())
  {
    definedFunctions.insert(std::pair<std::string, FunctionAST*>(fn->proto->name, fn));
    std::cout << "Traversal: " << std::endl;
    fn->traverse();
    varIds.clear();
    return true;
  }
  return false;
}

bool genExtern() { return false; }

bool genTopLvlExpr() { return false; }

//CHECK CONTROL FLOW

void printControlFlow()
{
  if(!definedFunctions.contains("main"))
  {
    std::cout << "No main function (entry point) defined" << std::endl;
    return;
  }
  neededFunctions.insert(std::pair<std::string, int>("main", 1));
  std::cout << std::endl << "Control Flow:" << std::endl;
  definedFunctions["main"]->showControl();
}

void printFuncCat()
{
  std::cout << std::endl << std::endl << "Based on the control flow, we can assess the following:" << std::endl;
  std::cout << "The functions which are called (that is need to be compiled and linked) are:" << std::endl;
  for(auto it = definedFunctions.begin(); it != definedFunctions.end(); it++)
  {
    if(neededFunctions.contains(it->first))
    {
      std::cout << it->first << std::endl;
    }
  }

  std::cout << std::endl << "The functions which are not called (that is do not need to be compiled and linked) are:" << std::endl;
  for(auto it = definedFunctions.begin(); it != definedFunctions.end(); it++)
  {
    if(!neededFunctions.contains(it->first))
    {
      std::cout << it->first << std::endl;
    }
  }
}

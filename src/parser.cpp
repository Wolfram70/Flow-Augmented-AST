#include "../include/parser.h"

#include <cstdio>

extern int getToken();

char curTok;

std::map<char, int> binOpPrecedence;

std::map<std::string, std::unique_ptr<PrototypeAST>> functionProtos;
int justused = 0;
std::vector<ExprAST*> justBefore;
int id = 0;

std::map<std::string, int> varIds;
std::map<std::string, ExprAST*> definedFunctions;
std::map<std::string, int> neededFunctions;

int tempResolveTopLvlExpr = 0;

std::unique_ptr<ExprAST> parseNumberExpr();
std::unique_ptr<ExprAST> parseParenExpr();
std::unique_ptr<ExprAST> parsePrimary();
std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec,
                                       std::unique_ptr<ExprAST> LHS);
std::unique_ptr<ExprAST> parseExpression();
std::unique_ptr<ExprAST> parseIdentifierExpr();
std::unique_ptr<PrototypeAST> parseProtoype();
std::unique_ptr<FunctionAST> parseDefinition();
std::unique_ptr<PrototypeAST> parseExtern();
std::unique_ptr<ExprAST> parseUnaryExpr();
std::unique_ptr<FunctionAST> parseTopLvlExpr();
std::unique_ptr<ExprAST> parseIfExpr();
std::unique_ptr<ExprAST> parseForExpr();
std::unique_ptr<ExprAST> parseVarExpr();

int getNextToken() { return curTok = getToken(); }

std::unique_ptr<ExprAST> logError(const char *str) {
  fprintf(stderr, "LogError: %s\n", str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> logErrorP(const char *str) {
  logError(str);
  return nullptr;
}

std::unique_ptr<ExprAST> parseNumberExpr() {
  auto result = std::make_unique<NumberExprAST>(numVal);
  getNextToken();
  return std::move(result);
}

std::unique_ptr<ExprAST> parseParenExpr() {
  getNextToken();
  auto V = std::move(parseExpression());
  if (!V) {
    return nullptr;
  }

  if (curTok != ')') {
    return logError("Expected ')'");
  } else {
    getNextToken();
    return std::move(V);
  }
}

std::unique_ptr<ExprAST> parsePrimary() {
  switch (curTok) {
    case tok_identifier:
      return std::move(parseIdentifierExpr());
    case tok_number:
      return std::move(parseNumberExpr());
    case '(':
      return std::move(parseParenExpr());
    case tok_if:
      return std::move(parseIfExpr());
    case tok_for:
      return std::move(parseForExpr());
    case tok_var:
      return std::move(parseVarExpr());
    default:
      return logError("Unknown token when expecting an expression");
  }
}

std::unique_ptr<ExprAST> parseUnaryExpr() {
  if (!isascii(curTok) || curTok == '(') {
    return parsePrimary();
  }

  int opc = curTok;
  getNextToken();

  if (auto operand = parseUnaryExpr()) {
    return std::make_unique<UnaryExprAST>(opc, std::move(operand));
  }
  return nullptr;
}

std::unique_ptr<ExprAST> parseForExpr() {
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

  std::unique_ptr<ExprAST> step;
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

  return std::make_unique<ForExprAST>(idName, std::move(start), std::move(cond),
                                      std::move(step), std::move(body));
}

std::unique_ptr<ExprAST> parseVarExpr() {
  getNextToken();

  std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> vars;

  if (curTok != tok_identifier) {
    return logError("Expected identifier after var");
  }

  while (true) {
    std::string name = identifierStr;
    getNextToken();

    std::unique_ptr<ExprAST> init;
    if (curTok == '=') {
      getNextToken();

      init = std::move(parseExpression());
      if (!init) {
        return nullptr;
      }
    }

    vars.push_back(std::make_pair(name, std::move(init)));

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

  return std::make_unique<VarExprAST>(std::move(vars), std::move(body));
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

std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec,
                                       std::unique_ptr<ExprAST> LHS) {
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

    LHS = std::make_unique<BinaryExprAST>(binLoc, binOp, std::move(LHS),
                                          std::move(RHS));
  }
}

std::unique_ptr<ExprAST> parseExpression() {
  auto LHS = std::move(parseUnaryExpr());

  if (!LHS) {
    return nullptr;
  }

  return std::move(parseBinOpRHS(0, std::move(LHS)));
}

std::unique_ptr<ExprAST> parseIdentifierExpr() {
  std::string idName = identifierStr;
  SourceLocation litLoc = curLoc;

  getNextToken();
  if (curTok != '(') {
    return std::make_unique<VariableExprAST>(litLoc, idName);
  }
  getNextToken();

  std::vector<std::unique_ptr<ExprAST>> args;

  if (curTok != ')') {
    while (true) {
      if (auto arg = parseExpression()) {
        args.push_back(std::move(arg));
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

  return std::make_unique<CallExprAST>(litLoc, idName, std::move(args));
}

std::unique_ptr<ExprAST> parseIfExpr() {
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

  return std::make_unique<IfExprAST>(ifLoc, std::move(cond), std::move(then),
                                     std::move(_else));
}

std::unique_ptr<PrototypeAST> parseProtoype() {
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

  std::vector<std::unique_ptr<ExprAST>> argNames;
  std::vector<std::string> argString;

  int tok = getNextToken();

  while (tok == tok_identifier) {
    argNames.push_back(
        std::move(std::make_unique<VariableExprAST>(fnLoc, identifierStr)));
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

  return std::make_unique<PrototypeAST>(fnLoc, fnName, std::move(argNames),
                                        std::move(argString), kind != 0,
                                        binaryPrecedence);
}

std::unique_ptr<FunctionAST> parseDefinition() {
  getNextToken();

  auto proto = std::move(parseProtoype());
  if (!proto) {
    return nullptr;
  }

  if (auto exp = std::move(parseExpression())) {
    return std::make_unique<FunctionAST>(std::move(proto), std::move(exp));
  }

  return nullptr;
}

std::unique_ptr<PrototypeAST> parseExtern() {
  getNextToken();

  return std::move(parseProtoype());
}

std::unique_ptr<FunctionAST> parseTopLvlExpr() {
  SourceLocation exprLoc = curLoc;

  if (auto exp = std::move(parseExpression())) {
    auto proto = std::make_unique<PrototypeAST>(
        exprLoc, "main", std::move(std::vector<std::unique_ptr<ExprAST>>()),
        std::move(std::vector<std::string>()));
    return std::make_unique<FunctionAST>(std::move(proto), std::move(exp));
  }

  return nullptr;
}

bool genDefinition()
{
  if(auto fn = parseDefinition())
  {
    definedFunctions.insert(std::pair<std::string, FunctionAST*>(fn->proto->name, &(*fn)));
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
  std::cout << std::endl << "Control Flow:" << std::endl;
  std::cout << definedFunctions["main"]->nodeName;
}

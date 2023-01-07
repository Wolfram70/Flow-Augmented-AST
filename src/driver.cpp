#include "../include/parser.h"

extern int getNextToken();

std::ifstream file;
std::string fileName;

extern bool genDefinition();
extern bool genExtern();
extern bool genTopLvlExpr();
extern void printControlFlow();
extern void printFuncCat();

static void handleDefinition() {
  if (!genDefinition()) {
    getNextToken();
  }
}

static void handleExtern() {
  if (!genExtern()) {
    getNextToken();
  }
}

static void handleTopLvlExpr() {
  if (!genTopLvlExpr()) {
    getNextToken();
  }
}

static void mainLoop() {
  while (true) {
    switch (curTok) {
      case tok_eof:
        return;
      case ';':
        // fprintf(stderr, "Ready>>");
        getNextToken();
        break;
      case tok_def:
        handleDefinition();
        break;
      case tok_extern:
        handleExtern();
        break;
      case tok_number:
        handleTopLvlExpr();
        break;
      case tok_identifier:
        handleTopLvlExpr();
        break;
      default:
        getNextToken();
        break;
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Invalid number of arguments" << std::endl;
    return 1;
  }

  std::string arg;

  for (int i = 1; i < argc; i++) {
    switch (argv[i][0]) {
      case '-':
        break;
      default:
        fileName = argv[i];
        break;
    }
  }

  file.open(fileName, std::ios::in);

  if (!file.is_open()) {
    std::cout << "Could not open file \"" << fileName << "\"" << std::endl;
    return 1;
  }

  binOpPrecedence[':'] = 1;
  binOpPrecedence['='] = 2;
  binOpPrecedence['<'] = 10;
  binOpPrecedence['+'] = 20;
  binOpPrecedence['-'] = 20;
  binOpPrecedence['*'] = 40;

  // fprintf(stderr, "Ready>>");
  getNextToken();

  mainLoop();

  printControlFlow();

  printFuncCat();

  return 0;
}

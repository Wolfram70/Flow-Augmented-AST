#include "../include/parser.h"

extern int getNextToken();

std::ifstream file;
std::string fileName;

bool printControl = false;
bool printFunc = false;

extern bool genDefinition();
extern void printControlFlow();
extern void printFuncCat();

static void handleDefinition() {
  if (!genDefinition()) {
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
  char choice;

  for (int i = 1; i < argc; i++) {
    switch (argv[i][0]) {
      case '-':
        choice = argv[i][1];
        arg = argv[i];
        if(arg.length() > 2) {
          std::cout << "Invalid argument \"" << argv[i] << "\"" << std::endl;
          return 1;
        }
        switch (choice) {
          case 'c':
            printControl = true;
            break;
          case 'f':
            printFunc = true;
            break;
          default:
            std::cout << "Invalid argument \"" << argv[i] << "\"" << std::endl;
            return 1;
        }
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

  if(printControl) printControlFlow();

  if(printFunc) printFuncCat();

  return 0;
}
# Flow-Augmented-AST

This project is a implementation of a tool that augments abstract syntax trees (ASTs) with flow information, which allows for more accurate static analysis and type checking of code. The project is implemented in C++.

## Prerequisites

A C++20 compatible compiler (e.g. GCC 8.0 or higher, Clang 4 or higher)

## Installing
1. Clone the repository to your local machine.
        
        git clone https://github.com/Wolfram70/Flow-Augmented-AST.git

2. Navigate to the root of the project.
        
        cd flow-augmented-ast-cpp

3. Compile the code

      <b>In Clang++</b>

           clang++ src/*.cpp -std=c++20 -o <file_name>

      <b>In GCC</b>

           g++ src/*.cpp -std=c++2a -o <file_name>

4. Create a new text file say `code.txt`

5. Write some code in the custom programming language

6. Compile the file

        ./<file_name> code.txt

Necessary flags can also be passed as arguments. Using `-c` will show the control flow of the program and using `-f` will output all the function names in the program, demarcating redundant functions from the used ones.

For example:
        
        ./parser code.txt -c -f
        
## Sample Code

<b>Text File: </b>

<img width="584" alt="Screenshot 2023-01-11 at 8 34 49 PM" src="https://user-images.githubusercontent.com/108116233/211840709-05f90f12-3075-4171-98eb-2fe8439f93f0.png">

<b>Program Output: </b>

<img width="1059" alt="Screenshot 2023-01-11 at 8 36 57 PM" src="https://user-images.githubusercontent.com/108116233/211841248-48b4a813-30a5-4ce9-a27b-27887fffeca9.png">

## Custom Programming Language

The entry point for execution in the program is the main function.
Variables defined in the function prototype


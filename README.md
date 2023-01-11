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


<img width="672" alt="Screenshot 2023-01-11 at 9 59 35 PM" src="https://user-images.githubusercontent.com/108116233/211861789-2cf14a11-5c32-46e3-b915-c311db92935b.png">


<b>Program Output: </b>


<img width="1059" alt="Screenshot 2023-01-11 at 8 36 57 PM" src="https://user-images.githubusercontent.com/108116233/211841248-48b4a813-30a5-4ce9-a27b-27887fffeca9.png">

## Custom Programming Language

The entry point for execution of the program is the main function.
`Tabs` are treated as `invalid tokens`.

### Datatypes and Variables

Variables shouldn't be declared with their corresponding datatypes. All variables are implicitly double data type.

        int a = 5; //incorrect syntax
        
Variables can be declared using the ```var``` keyword but their scope is limited to the block that follows. The following example illustrates the syntax:

        var a = 5, b = a, c = 6 in (# code);


Variables can also be declared <b>only</b> in the function prototype. Details about this have been mentioned later under the <b>Functions</b> subheading.

### Functions

To declare a function use the `def` keyword. 
        
        def <function_name>(<var_1>,<var_2>...<var_n>)
            <var_1> = 10:
            <var_2> = 20;

Variables are declared in the function prototype and thereafter used within the function.

        def main(a,b,c)
            a=10:  #valid
            b=20:  #valid
            c=30:  #valid
            d=40;  #error!!! variable undeclared
              
The last expression evaluated within the function block is returned to the callee.

        def foo1(a,b)
            a:a+b;       # the value of a is returned by the function

        def main()
            foo1(5,6);


All statements within a block are terminated with a `colon (:)` except the last statement of a block which ends with a `semicolon (;)`. The `semicolon (;)` marks the end of a code block. Statements after it are ignored. If the block contains only one line of code terminate it with a `semicolon (;)`.

### Conditional Statements

The language supports the if-else construct. The following example illustrates the syntax:

        def main(a,b,c,d)
           if ( a < b ) then ( c = 10 ) else ( d = 20 );






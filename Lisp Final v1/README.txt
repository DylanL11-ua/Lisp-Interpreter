LISP INTERPRETER PROJECT
FINAL SUBMISSION

This project implements a simple Lisp interpreter using S-expressions. The interpreter supports reading, printing, and evaluating Lisp expressions.

The interpreter now maintains a global environment and a local environment stack. It supports defining, storing, and calling user defined functions, 
including nested and recursive function calls.

This is the final stage of the interpreter which can handle much more complex Lisp programs.


BUILD INSTRUCTIONS
REQUIREMENTS:
A C++ compiler that supports C++17

g++ recommended

COMPILE:
Open a terminal in the directory containing SExpression.cpp and run:

g++ -std=c++17 SExpression.cpp -o SExpression

After compiling, if on Windows, run:

.\SExpression.exe

On Mac, run:

./SExpression


TESTING:
The program can be tested informally using the expressions provided in input.txt. These tests cover:

    - Reading, parsing, and printing S-expressions
    - Quoting expressions
    - List operations: car, cdr, cons
    - Expression evaluation: eval
    - Global assignment and symbol lookup using set
    - Repeated assignments and variable shadowing
    - Predicates: nil?, not?, atom?, list?, number?
    - Boolean operations: and?, or?, eq?
    - Conditional expressions: if, cond
    - Arithmetic operations: add, sub, mul, div, rem
    - Relational operation: lt
    - Nested arithmetic and conditional expressions
    - Creating and storing user defined function expressions
    - Retrieving stored functions through global lookup
    - Passing function expressions as arguments to supported operations
    - Calling user-defined functions with evaluated arguments
    - Local variable lookup and shadowing
    - Nested and recursive function calls
    - Functions with empty parameter lists
    - Checking argument counts for user-defined functions
    - Complex Lisp programs (blackjack, bowling, etc.)

The program was run using:

Get-Content input.txt | .\SExpression.exe > output.txt

The resulting outputs are included in output.txt.

If you'd like to run your own input file through the interpreter, you can add it to the current Lisp Project folder and run in a terminal:

Get-Content <your input file>** | .\SExpression.exe > output.txt 

**NOTE: As long as the input file has no comments as the current lexer hasn't been implemented to handle Lisp comment syntax.

UNIT TESTING:
I spent a day creating a program that does a proper unit test for all 523 test cases (in the directory as UnitTest.cpp).

This program takes the inputs and expected outputs from test_cases.txt (seperated into two different files) and compares them after 
running each test case through the interpreter.

    COMPILATION/BUILD
    Open a terminal in the directory containing UnitTest.cpp and run:

    g++ -std=c++17 UnitTest.cpp -o UnitTest

    After compiling, if on Windows, run:

    .\UnitTest.exe

    On Mac, run:

    ./UnitTest


ERROR TESTING:
Additional invalid expressions are provided in error_handling.txt if you'd want to test each case individually or augment a test case.

The interpreter handles errors including:

    - Unexpected closing parentheses
    - Unexpected end of input
    - car or cdr applied to invalid values
    - Unknown functions
    - Invalid function positions
    - Incorrect numbers of function arguments
    - Invalid first arguments to set
    - Invalid cond structures
    - Invalid math operations
    - Attempting to pop an empty local environment stack
    - Mismatched local environment name and value counts
    - Invalid function definitions and calls

The interpreter no longer terminates after encountering an invalid expression, so you can test multiple error cases without needing to rerun the program.

To test all error handling cases, run:

Get-Content error_handling.txt | .\SExpression.exe > output.txt

**NOTE: This will clean the output file from its existing contents and write all outputs from the error file to it.


DIRECTORY STRUCTURE:
Contains 9 files:

    SExpression.cpp
        Contains the source code for the actual Lisp interpreter.

    UnitTest.cpp
        Takes the input.txt file and runs it through the Lisp interpreter 
        and compares the output to its expected output in expected.txt.
        Then the program spits out all test cases as either a PASS or FAIL.

    test_cases.txt
        Contains both the input and expected output for all 523 test cases. 
        Shows in one file what the unit test is looking for as a PASS.

    input.txt
        Contains the primary valid test cases on their own used to test the 
        interpreter informally.

    expected.txt
        Contains the expected outputs from all test cases (used for unit 
        testing) on their own from what was.

    unit_test.txt
        Contains the formal results of the unit test of the input.txt file
        (copy and pasted from the test).

    error_handling.txt
        Contains invalid Lisp expressions used to test error handling.
        Now included in input.txt for testing.

    output.txt
        Contains the output produced by any input file run from the command:
            Get-Content <your input file>** | .\SExpression.exe > output.txt 
        Currently has the output from input.txt.
        Can be used to generate fast output from any input file.

    README.txt
        Contains project information, build instructions, testing
        instructions, directory structure, and known limitations.


DEFICIENCIES / NOTES:
The lexer does not currently support Lisp comments.

The 'set' operation returns the evaluated value being assigned. 

No other known deficiencies at the time of submission.
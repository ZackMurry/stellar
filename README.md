# The Stellar Programming Language

## Compilation
To compile a given file, execute the compiler with the name of the file to compile as an argument.
This will produce a file named `output.o`, an object file that needs to be compiled into machine code.
This is done using a C++ runner file (`runner.cpp`), which invokes the entry point of your program.
To use the runner to compile your program, run `g++ runner.cp output.o` (`clang++` also works),
which will produce an executable file named `a.out`.

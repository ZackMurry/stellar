# The Stellar Programming Language
Stellar is a compiled language that is built using the LLVM compiler toolchain. The syntax is a mixture of Java and C.

## Compilation
### Compiling the compiler
```bash
$ cmake .
$ cmake --build .
```

### Compiling a program
To compile a given file, execute the compiler with the name of the file to compile as an argument.
This will produce a file named `output.o`, an object file that needs to be compiled into machine code.
This is done using a C++ runner file (`runner.cpp`), which invokes the entry point of your program.
To use the runner to compile your program, run `g++ runner.cp output.o -static` (`clang++` also works),
which will produce an executable file named `a.out`.

#### Configure standard library
The Stellar compiler allows you to use any standard library. A default standard library is in development, but it is not ready.
If you'd like to create and use your own standard library, you can pass the path to the standard library as the second argument
to the compiler. You can import the standard library like this:
```c
import "std/list.stellar"
```

## Examples
### Hello, world!
```c
extern void printf(string, ...)
printf("Hello, world!\n")
```
### Functions
```c
extern void printf(string, ...)
void sayHi() {
  printf("Hi!\n")
}
```

### Classes
```java
class Box {
  i32 width
  i32 height
  
  new(i32 width, i32 height) {
    this.width = width
    this.height = height
  }
  
  i32 getArea() {
    // "this." is required when accessing class fields
    ret this.width * this.height
  }
}

new Box box(12, 34)
printf("Area: %d\n", box.getArea())
// Another way to instantiate a class
Box box2 = new Box(12, 34)
```

### Generics
```java
class Box<T> {
  T value
  
  void setValue(T value) {
    this.value = value
  }
  
  T getValue() {
    ret this.value
  }
}
new Box<i32> box
box.setValue(12)
printf("%d\n", box.getValue())
```

### Null
```c
Box box
if (box?) { // True
  printf("Box is null\n")
}
box = new Box
if (!box?) { // True
  printf("Box is not null\n")
}
```

### Arrays
```c
i32[5] arr
for (i32 i = 0; i < 5; i++) {
  arr[i] = i
}
for (i32 j = 0; j < 5; j++) {
  printf("%d\n", arr[j])
}
```

### Data types
```rust
f32 myFloat = 12.4
f64 myDouble = 12.4f64
i8 myI8 = 3i8
i16 myI16 = 5i16
i32 myI32 = 2343i32
i64 myI64 = 2321321321i64
bool myBoolean = false
string myString = "this is a string"
```

### Imports
```c
import "/home/zack/stellar/file_name.stellar"
// The preprocessor is currently only capable of handling absolute file paths
```

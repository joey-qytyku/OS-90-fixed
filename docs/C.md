
# Why I Made This Tutorial

So many tutorials teach less about the C language and more about the standard library.

I wanted to experiment with a different approach.

# Required Knowledge

* You have at least tried programming before
* You know what bits and bytes are
* Understand what "abstraction" is
* Basic knowledge of UNIX/Linux commands

If you took AP Computer Science Principles or A, you are prepared for the content here. You do not need to know C in order to learn in C++.

# Toolchain

C sources are dot-c files which are passed to a compiler. The compiler is not merely one program, but a toolchain. The first step is the preprocessor, which we will get to eventually. The second step is when the program is converted to assembly language for the CPU to execute. An assembler, which specializes in converting machine code into an object file, will be given the newly generated assembly code. Finally, a program called the linker is called with all the generated object files, also known as translation units, and covnerts it into a final executable.

The GNU Compiler Collection and Binutils, which I will be using in this tutorial, provide the following features to do this:
* ld (Linker)
* gcc (Compiler)
* as (assembler)
* LD and AS do not need to be manually invoked because the compiler can automatically execute the assembler or linker after it finishes.

For example:

```bash
gcc main.c
```

I will not focus on that way right now. I want you to understand the whole process. We will do it like this:
```
gcc main.c -c -o main.o
```
The `-c` option will tell the compiler to convert it to assembly and then assemble the file into an object. The `-o` option specifies the name of the output file. If you want, try passing `-s` to the compiler to see the assembly code.

```
ld main.o -o main
```
Now we take the object files, in this case only one, and convert to an executable. Object files are not executable and contains incomplete data. This program can be executed by referencing its path in the command line.
```
./main
```

# Preprocessor

The preprocessor is the first phase of the compiler. It is a simple string substitution system.

We can define preprocessor symbols like this:
```
#define NAME value
```

Symbols can be undefined, though it should be avoided:
```
#undef NAME
```

We can use conditional processing to determine if the code within a block should be kept or removed.

```c
#ifndef NAME
#ifdef NAME
```
This sequence can be terminated with `#endif` directive.

Most of the time, we will not work with the preprocessor unless in a header file, which has the extension of `.h`. Headers are copied to an implementation file (.c) by inserting an `#include <NAME>` or `#include "NAME"` directive, where NAME is the path to the file.

Paths work like they do on unix. `..` means to go back one directory level. `.` references the current directory, though this behavior is implicit. `/` moves into a directive or refers to the root.

Some header files are packaged with the system and are referenced with the bracket notation. For example `<stdio.h>`.

# Comments

Comments are regions of text which are not read by the compiler. They can be used for explaining code. There are two ways to write comments.

Originally, the only way was this:
```c
/*
  ...
*/
```

C++ introduced single line comments, which can be used as a replacement for block comments or if only one line needs to be commented out. C supports these as well.

```c
// This
// is
// a
// comment
```

Use comments for when the code is not enough to explain what is happening.

# Variables and Types

## How to Try This Out

Compiler the file to object form to check if the code is correct. For now, we will not be writing any code yet, so compiling to executable form will fail.

## Basic Numerical Types

C is a statically typed language. A datum is declared with a type, or a fixed machine interpretation and size in memory, which cannot change during runtime. If you are familiar with Java, some of these types will be familiar.

The following are the primitive types that C supports:
```c
// Whole numbers (integers):
char
short
int
long int // (int can be omitted)
long long int

// Floating point numbers:
float
double
```

Integers are whole numbers. By default, they can handle negative numbers unless prefixed with `unsigned`. For example `unsigned char`. There is nothing stoping you from using negative numbers in an unsigned type, but there are problems.

The sign of a number is determined by the very top bit. If it is set, the number is negative. Modern computers use radix operations, meaning that integer addition/subtraction is done with the same operation. What matters is interpretation. Sometimes, we do not need negative numbers (e.g. age).

It is important to ensure that the sign of the type is properly specified. Machine instructions for multiplication are different between the two due to the negative times negative property. A signed operation for multiplying could cause unwanted behavior.

The `char` type is large enough to store a single character.

## Defining Variables

To add a variable, write the type followed by the name and optionally an initializer. Semicolons are required in C.

When a variable is added to the file, it will go into the data section or the "BSS" section if uninitialized.

```c
int my_number = 10;
int your_number;
```

your_number is defined without an initializer but `my_number` is initialized to 10. If a number will be initialized later in your program and does not need an initial value, then there is no need to initialize it to anything.

On the other side of the equal sign, we can put any numerical expression. For example:

```c
int my_number = 10 * 10;
```

We can also use character constants. These can only store one single ASCII character. Usually, the `char` type is used when storing single letters.

```c
char exam_grade = 'A';
```

Floating point values can be defined as expected:
```c
float f0  = 1.0;
double f1 = 4.9;
```

Double is 64-bit and a more precise version of the 32-bit float. We will not be dealing with floating point a lot in this tutorial.

If data should never change in the program, then it is advisable to add the `const` specifier before the type.

Initializers cannot reference other data unless it is constant. Therefore, this is wrong:
```c
int i = 10;
int j = i + 1;
```
The compiler will complain about a non-constant initializer.

```c
const int i = 10;
int j = i + 1;
```

Now if we change `i` to a constant, we can declare a non-constant variable using `i` in the expression.

The reason this must be done is because without `const`, the value of `i` could change at any time, so the compiler does not know the value of `i` that should be used in the expression.

## Arrays and Strings

An array is a special type that is a collection of data of the same type. Here is how we declare them. For our purposes, the initializer will have to be a constant expression.

```c
int array_of_ints[10];
```

Strings are similar to arrays, but we declare them like this:
```c
const char *my_string = "Hello, world";
```
The handling of strings is confusing to many beginners and will be better understood when pointers are described.

Strings can be declared without a const if needed, but that will be addressed later as well.

## Making Our Own Types

The default types have their drawbacks. First of all, the programmer does not know the size in bytes of the value (it can vary between platforms) or what abstract object it represents in the code. Second of all, we may want to group related data together under the same name. C has two features for these problems.

We can declare our own type using `typedef`. The format is like this
```
typedef [meaning] [type name];
```
There are many uses for typedef that we will not cover yet.

The next feature is data structures. They contain names values of a certain type.
```c
struct name_of_struct
{
    int i;
    int j;
    int k;
    float l;
};

struct name_of_struct my_struct_var;
```
You cannot just access the structure. When you define it, a new type is created.

Structures can be initialized with one of two bracket notations.
```
name_of_struct first_way  = { 1, 2, 3, 1.9 };

name_of_struct second_way = {
    .i = 10,
    .j = 4,
    .k = 1,
    .l = 1.004
};
```

> Note that C does not give any special significance to newlines or whitespace outside of preprocessor directives. Feel free to arange code however is most readable.

It is annoying to write `struct` all the time, so instead we can use a `typedef` and a `struct` to avoid this. The only difference is that the name of the type has to be last, as expected with typedef.
```c
typedef struct
{
    // ...
}name_of_struct;
```

## Try This

Because the code we have explored thus far does not have any executable code, it cannot be compiled directly to an executable. We can compile it to an object file.

Create a file with the extension `.c` and define some variables. For example:
```c
// file: myint.c
int my_int;
```

Compile the file to an object file:
```c
gcc myint -c -o myint.o
```

Try running the `size` tool.
```
size myint.o
```

For this example, we will get the following output:
```
   text    data     bss     dec     hex filename
     32       0       4      36      24 main.o
```
This tells us how many bytes are in the program. The reason text is not zero is not important. bss contains 4 bytes of uninitialized data because that is the number of bytes that an `int` takes up on the Intel/AMD architecture. If the data is initialized, it will appear in data rather than bss.

The C language does not explicitly specify the exact size of all the types for all CPU models. `char` is garaunteed to be a byte for all platforms. `int` is usually 4 bytes but not on all CPU architectures. `long int` is 64-bit on x86 Linux.

We will learn how to abstract the sizes of different types by explictly specifying their sizes in the future.

# Expressions

So far, we know how to define variables in C programs. Next we will learn how to assign values that are more complicated than just a single number, but are mathematical expressions.

The value assigned to a variable is an expression, which can use various arithmetic, bitwise, and boolean operator characters.

## Constant Expressions

When the variable is defined at the outermost layer of the program, the value assigned must be a constant expression. This means that it cannot use any values that are not constant, such as other variables. All values must be known at compile-time.

```
int var = 10 + 10; // var = 20
int bad = var + 1; // THIS DOES NOT WORK
```

This example is wrong because `bad` is not being assigned a constant value. `var` is subject to change. What value is the compiler supposed to use?

Here is the fixed version:
```c
const int var = 10 + 10; // var = 20
int bad = var + 1;
```
`var + 1` is a constant expression. Constant variables do generally get memory reserved for them, but the compiler knows they will not change, so it can find out what `var + 1` is.

## Parentheses

Parentheses work exactly like in mathematics. They have the highest precedence and are evaluated before everything else in the expression.

## Integer Operators

C supports all of the standard arithmetic operators: `+`, `-`, `*`, `/`. It also has modulus as `%`, which divides a number and returns the remainder. The order of operations is as expected, with modulus being grouped with `*` and `/`.

For a full description of operator precedence in C and C++, visit: https://en.cppreference.com/w/c/language/operator_precedence. This includes other operators we will learn about.

## Boolean Operators

A boolean is a value that can be true or false. C does not have a boolean data type by default (it can be added by including stdbool.h), but it does allow integer types to be used for that purpose. A 1 represents true and a zero represents false.

First, we have the `==` operator, which return true (1) if the two __operands__ are equal.
```
int i = 2-1 == 1;
```

we have the `&&` operator, which means AND.
```c
int i = 1 && 1;
```
`i` will be set to 1 because both sides are true.


# Recap #1

You should now have an understanding of:
* Initialized and uninitialized variables, as well as the differences between the two
* What a constant expression is and the effect of the `const` specifier, when they must be used
* Boolean and integer expressions

# Code Blocks and Functions

All of the examples in this section can be compiled to an executable.

```
gcc -c name.c -o name
```

Or the short form:
```
gcc name.c -o name
```

Run the file (replace name with the actual name):
```
./name
```

## Introduction to Functions

* Program code is enclosed in functions
* Functions take zero or more inputs, and no more than one output. Both can be any type.
* Functions allow code to be repeated.

Let is examine a hello world program and see what is happening.
```c
#include <stdio.h>

int main(int argc, char **argv)
{
    printf("Hello, world!\n");
    return 0;
}
```
The first line includes a header file which included the function `printf`.

`main` is a function which takes two arguments related to the command line arguments and returns an `int` which indicates the program status. The function signature (name, args, return type) is followed by a code block, which specifies commands to be executed.

> Note that an executable program must have `main`. It must have either zero parameters or the two specified.

Let's look at some more functions:
```c
int square(int value)
{
    return value * value;
}

int main()
{
    return square(10);
}
```

If we compile to executable form and check the exit status of the program by running `echo $?`, the value should be 100. The exit status should not be used this way, but the example illustrates how functions work. square(5) would be 25, square(2) would be 4 and so forth.

Functions are called by referencing their name and placing the arguments in a comma-separated list. The return value is what the function evaluates to in the expression. `return` is the special statement in C that causes the function to return where it occurs.

Some functions may not return anything at all. In this case, we write `void` as the return type. `void` cannot be used to initialize a variable. Some functions take no arguments. In this case, the parameter list can be empty or have a `void`. If the parameter list is empty, the compiler may not warn you if extra arguments are passed, so place `void` in the parameter list if it takes none.

```c
void i_take_none_and_return_none(void)
{
    // ...
}
```

### Changing Variables

Being able to perform mathematical calculations is useful, but functions can do much more in C. Variables can be reassigned to different values. This will be important for implementing algorithms.

```c
#include <stdio.h>

int i = 10;

void print_num(int i)
{
    printf("%i\n", i); // Do not worry about this yet
}

int main()
{
    print_num(i);
    i = 5;
    print_num(i);
    return 0;
}
```

This will print:
```
10
5
```

The code defines the variable `i` as 10. It prints out this value, sets it to 5, and prints it again. In this example, we also see an example of abstraction. We do not need to remember how to use the stdio.h function printf to print a number. Instead, we call a self-documenting function of our own creation.

## Using printf to Track Variables

`printf` is a function that takes a string constant enclosed in double quotes (it must be constant) and any number of additional arguments of various types. The function substitutes the format characters (in this case `%i`) with the extra argument at the corresponding index. Finally, it prints the resulting string.

For example
```c
printf("%i\n", 10);
```

Output:
```
10
```

The '\n' sequence represents a newline. `printf` does not automatically insert them.

Adding more arguments is possible:
```c
printf("%i %i %i\n", 1, 2, 3);
```

The output:
```
1 2 3
```

## Scoping and Local Variables

Sometimes, we will have variables that are only relevant within a certain context. We can make them local to a function by declaring them in a curly-bracketted code block.

```c
int main()
{
    int local_var1 = 1;
    {
        int local_var2 = 2;
    }
    printf("%i", local_var2);
}
```

This program will fail because `local_var2` went out of scope and cannot be referenced again.

Local variables are dynamically allocated on the stack or in machine registers by the compiler. They are different from variables at the outer level because they are not allocated when the program is compiled but when it runs. This allows local variables to have __non-constant__ initializers, so they can reference other variables and even get the return value of function calls.

```c
int main(int argc, char** argv)
{
    int array[argc];
}
```
While this is not a practical example, it illustrates the concept. Be careful though. If the number is too big, it could cause a stack overflow.

## Pass by Value

All arguments passed to a function are passed by value, which means they are copied to the stack before calling the function. That means if a local variable is passed to a function, it will remain unchanged even if the function modifies the argument for any reason.

# Conditionals

If (no pun intended) you have tried Java or JavaScript, conditionals will be a breeze in C because they are very similar.

## Introduction and Examples

if-statements take a boolean expression inside parentheses and execute the code inside if the condition is true. Otherwise, the code block after the if-statement is not executed.

Any boolean expression can work. Providing an integer that is not either zero or one will cause it to be converted to TRUE if non-zero and FALSE if zero.

```c
int main(int argc, char **argv)
{
    if (argv > 1)
        printf("I don't want arguments!!! >:(");
}
```

if-statements can have more complex expressions. Operators like `&&` and `||` have lower precedence than the comparison operators and are evaluated last.
```c
int main(int argc, char **argv)
{
    if (argc => 1 && argc <= 4)
        printf("Number of arguments between 1,4 incusive");
}
```

## else statement

The `else` keyword specifies a code block that runs if the above condition is not true. If the above condition is true, the `else` code will not run.

Let's look at the previous example:
```c
int main(int argc, char **argv)
{
    if (argc => 1 && argc <= 4)
    {
        printf("Number of arguments between 1,4 incusive");
    }
    else {
        printf(":(");
    }
}
```

## else if

Else if allows specifying if statements that are checked sequentially if and only if the previous `if` or `else if` is not taken. This is useful in some situations when you want to check something only if another thing was checked before.

```c
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc => 1 && argc <= 4)
    {
        printf("Number of arguments between 1,4 incusive");
    }
    else if (argc >= 2 && argc <= 7)
    {
        printf("Number of arguments between 2,7 incusive");
    }
```

If we pass two arguments, condition one will be found to be true and run. Because we used an `else if`, the second code block in `main` will __not__ run because the top one was true.

If we change `else if` to `if`:
```c
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc => 1 && argc <= 4)
    {
        printf("Number of arguments between 1,4 incusive");
    }
    if (argc >= 2 && argc <= 7)
    {
        printf("Number of arguments between 2,7 incusive");
    }
}
```
We pass 2 arguments to the program (btw the name of the program is the first one). __Both__ of the conditions will be evaluated separately to be true and both will run. The output will look like this:

```
Number of arguments between 1,4 incusive
Number of arguments between 2,7 incusive
```

## switch

A switch statement takes a single value inside the parentheses and compares is with a series of `case` statements. Here is the format:
```
switch (value)
{
    case constant-expression:
        // Code here
    break;

    default:
}
```

`break` is required to terminate the statement and continue after the code block. The `default` section does not require one and it runs only if the other cases are false.

If a certain action should be taken for more than one value, the `break` can be omitted:
```c
switch (value)
{
    case 3:
    case 4:
        printf("Number is three or four\n");
    break;

    case 5:
    case 6:
        printf("Number is five or six\n");
    break;
}
```
>The default section can be omitted. If none of the cases are true, the whole switch is skipped.

Using if statements, it would look like this:
```c
if (value == 3 || value == 4)
{
    printf("Number is three or four\n");
}
else if (value == 5 || value == 6)
```
Compilers will generate roughly equivalent code for both.

# Loops

# A Deeper Dive into Functions

This section involves advanced topics and is not necessary to read, but I highly recommend readers to learn about the structure of functions and what they mean to a computer.

## What is a Stack

The stack is a structure present on almost every computer in history. It is a growing list of same-sized entries typically the same size as a pointer. It supports two basic operations, push to add to the top of the stack and pop to remove from the top of the stack.

popping does not "delete" the value but it does change the stack pointer. In most CPU architectures, and in the case of x86, the stack pointer points to the last pushed element and the stack typically grows downward.

## What a Function Is

All modern computers have a register, or fast temporary storage location, called the program counter (aka instruction pointer). On Intel/AMD, this is called EIP or RIP for 32-bit and 64-bit respectively.

The most basic task of a processor is to:
* Fetch   (Get instruction from RAM)
* Decode  (Determine operation, size of instruction, data to operate on)
* Execute (Perform the operation with the information)

Of course, modern processors have all kinds of tricks to increase performance, but this is the most fundamental thing all CPUs will do.

Sometimes a program will:
* Make a decision to run a certain block of code (branch)
* Refer to the same code several times (function)

Invoking a function works like this:
* Save the program counter onto the stack
* Jump to the address of the function
* Complete the task of the function
* Pop the old program counter (PC) from the stack
* Jump to the location

Of course, the PC that is on the stack points to the instruction AFTER the invoke operation so that we do not get stuck.

The Intel/AMD architecture, more commonly known as x86, has a single instruction for calling functions.
```
call FunctionName
```
It translates to a byte with the hex value `E8` followed by a 32-bit address (for both x86 and x64) relative to the instruction pointer.

A single byte instruction called `ret` is used to return from the procedure, which is `C3` in hex.

# Pointers

Pointers may be the most important individual concept in the C language. In its most basic definition, a pointer is an integer object that stores a memory address of another object. Pointers are about as hard as C gets. It does not seem very useful at first, but there are many cases in which a programmer wants to write code that does not assume the location of value.

> Pointers are dangerous! Accessing invalid memory will certainly cause an error.

## Understanding Memory

On a typical computer, memory is an array of bytes addressible with an integer. The CPU can read or write to this memory just like a C program can read or write to an array of bytes. A CPU can also access the memory is larger quantities such as two, four, or eight bytes.

## Using Pointers

Pointers are variables which have the same size as an address on the target CPU. For x86-64, pointers are generally 64-bit. They are declared by specifying the type as with variables and putting an asterick before the name.
```c
int *p;
```

Never return a pointer from a function that points to a local variable. If the pointer is dereferenced, this will lead to stack corruption and will crash the program.

## Void Pointer

A void pointer is a pointer that does not have a specific type. It can point to absolutely anything, but should be used with care. You have to cast it before accessing.

## Pointer to an Array

Pointers to arrays are possible and recognized by C, but they are mostly useless because __pointers can be accessed like arrays with brackets__. I will not teach the way to make one because of reasons that will be made clear later.

There is a minor difference between arrays and pointers in C. Arrays behave mostly like pointers, but the main difference lies in the fact that the symbolic name for the array refers to the memory address where all the data is stored. The size of the initialized data is bound to the symbol, unlike pointers. The `sizeof` operator done on an array returns the size in bytes of the array. Doing the same to a pointer will be the same for all pointers, being equal to the native address size of the platform.

```c
char array[10];
sizeof(array); // This is equal to 10
```


### Array as Parameter

You cannot find the size of an array if it is an argument to a function. In general, never have arrays as arguments because they are automatically converted to pointers anyway and it just makes things more confusing.

```c
#include <stdio.h>

void func(int array[10])
{
    printf("Supposed size of array: %i", sizeof(array));
}

int main()
{
    int array[10];
    func(array);
    return 0;
}
```

What happens here is that `main` creates an aray of 10 elements and passes it to `func` by __reference__ and not by value. `int array[10]` in `func` amounts to nothing more than a pointer of type `int*`.

## Function Pointers

Functions have memory addresses just like variables. This means they can also be pointed to. The syntax for writing a function pointer is a bit strange. Let us write a function that returns void and takes void.

```c
void (*name_of_ptr)(void);
```
Putting the name and asterick in parentheses tells the compiler that this is not a void pointer, but a pointer to a function that returns void.

Here is an example returning int with two int parameters:
```c
int (*ptr2)(int,int);
```

To avoid the confusing syntax, simply use a typedef.
```c
typedef int (*my_pointer_type)(int,int);
```

### How To Use Them

C supports two identical methods of calling a function pointer. The regular call syntax is possible.
```c
#include <stdio.h>

typedef int (*math_op_ptr)(int,int);

int add_nums(int a, int b)
{
    return a + b;
}

int sub_nums(int a, int b)
{
    return a - b;
}

int main()
{
    math_op_ptr mp;
    int result;

    mp = &add_nums;
    result = mp(10,10);

    printf("%i\n", result);
}
```

You can even make an array of function pointers. This can be done with the typedef or without it (by putting the brackets after the name).

## Practical Applications of Pointers (TODO Move down)

Applications for pointers are endless.

Let us suppose we would like to implement a function that adds together arrays of variable sizes into a single value and returns the sum.

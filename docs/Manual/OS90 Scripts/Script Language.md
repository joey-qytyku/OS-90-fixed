# OS/90 Shell (TODO WIP)

OS/90 has a command-line shell built into the kernel that can also be ported to user mode. It is used to initiate the boot process in a configurable manner.

The language is strongly typed and internally uses threaded dispatch to optimize performance. It can be single-pass executed with no recursion or any lexical analysis whatsoever, with the only delimiter of statements being spaces and newlines. Special symbols are used for certain language features to simplify parsing. The parser is based on "eating" the next token, seeing if it makes sense, potenitally doing some extra processing, and outputting syntax errors as necessary.

Any keywords that start with a dot in a line are lexical scopes.

```
.kernel;

.proc main;
	echo "hello, world";
.endproc;

```

## Commands

Everything is a command. Commands are always semicolon-delimited (although the parser can detect missing semicolons). Any semicolons before a newline are comments.

The shell does not specify any external commands, but does have a number of internal ones. The general structure considers all special characters on the number row to indicate non-external commands, but some commands are opaqely internal.

## Variables and Data

The equal sign is the assignment operator, but it uses Polish notation. It can be an 'int' (32-bit signed), 'uint' (32-bit unsigned), string, or some array of the former two.

```
=a 2;           Unsigned
=b +2;          Signed (using + or -)
=c "Str"        String
=d {1,2,3,4}
```

This can be used for registers too, as they are essentially local variables.

## Registers

Eight registers are part of the script execution state. They have a type tag and can be references or values, as long as they fit in a pointer size.

Strings cannot be values, but can be converted to values using the `.str` utility.

## Library and Dot-Calls

### .call

```
.call ProcName ...
```

Call a procedure. The 8 registers are used to pass arguments. String and numerical constants are permitted, as well as symbol references. Expressions can be used to represent numerical arguments.

Example:
```
.call printf "Hello world" [+$0 $1]; Square brackets indicate expression
```

### .len

Sets first argument to the length of the array or string.

### .str

String utility.

```
cpy:
	<dest> <source> <count>
```

### Utilities

## String Utilities

## Example Programs

Hello world:
```
echo "Hello, world";
```

Finding a value in an array:
```
.entry main

.proc main
	.len $2 $1;
	.for i 0, i < $2, =i i+1;
		? == i 5;
			printf "Found";
		?end;
	.endfor;
.endproc;
```

# Implementation Details

## Symbols and Lookup

Symbols are bound values at run time and this value can change in type. Each access to a value requires a string lookup.

As commonly done on OS/90, the interpreter uses alphabetical ordering for symbols.

## Expression Evaluation

Some parts of the syntax allow for the insertion of expressions. An expression is a series of operations that collapse into a value.

Operators supported are:
```
*
/
&
|
!
^
```

Boolean operations:
```
||
&&
```

Remember that Polish notation is used. This actually makes parsing easier. If the parser each the next two expressions and finds another operator, it will chain very easily and can substitute the lack of parentheses or operator precedence. For example `+ 5 * 10 10` would be `5+(10*10)` Parsing can be done recursively or non-recursively.

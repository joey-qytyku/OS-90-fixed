# C String Insanity

C has a way of handling strings that is very confusing. This document will discuss how C compilers generate strings and how OS/90 handles them.

# Java Strings

Java has a `String` type that makes string manipulation very easy. A `String` behaves like any other type. It can be reassigned to a different value, but the contents are immutable and modifications require making a new string.

This is because java has a concept of references, and `String` is not a primitive type--it has methods. When you assign a new string value, a new object is being created and the variable has the reference changed. The idea of references will be relevant later.

Java strings are nice.

# C Strings

C has references in the form of pointers, but separates them from the concept of the value of an object.

Unlike Java, C does not treat arrays as any other objects. Arrays are just a sequence of bytes reserved to a name. They cannot have a reference changed because there is not reference at all. The value, however, is a reference as a constant rather than a value.

Strings are strange. They are array-like but have special code generation attributes.

They can be initialized like this:
```c
char *str = "Hello";
int *error = {1,2,3,4};
```
Why does the second one not work? No idea.

## String Emit Table

Here is a table of different types of strings and how the compiler emits them. GCC was used.

Code|                        Pointer in Memory| String |
-|-|-
`char *str = "Hello";`        | YES | YES |
`static char *str = "Hello";` | NO  | YES |
`[static] char str[] = "Hello";`| NO | YES |

> Because `static` is a storage class, it has the right to alter how the compiler emits it.

If the string is globally visible, the compiler will generate the pointer because it can be reassigned.

# The Solution

I surrender. `const char *` is simply the C idiom for "constant string." `char *` is for a mutable string. No need to try and reinvent the wheel.

Programmer discretion should be used to decide when a string should be constant or not.

If C had array pass by value it could work in a reasonable way, but C does not.

## OS/90 Policy on Strings

See `Code.md`.

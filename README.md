# dcc

[![Build and test dcc - Linux](https://github.com/dmezh/dcc/actions/workflows/ci-linux.yml/badge.svg)](https://github.com/dmezh/dcc/actions/workflows/ci-linux.yml)
[![Build and test dcc - Darwin](https://github.com/dmezh/dcc/actions/workflows/ci-mac.yml/badge.svg)](https://github.com/dmezh/dcc/actions/workflows/ci-mac.yml)

dcc is a primitive C compiler originally built as a capstone project for a graduate course in compilers at Cooper Union - ECE466, Prof. Jeff Hakner. dcc targets the entire C99 standard, with a subset of C99 features currently available. 

You can compile a sizeable set of programs with dcc (but there's a lot of work to do to get to full C99). The target is x86-64 assembly.

## Key info

- Written in C, lex, and yacc.
- Lexer, (LALR) parser, AST, symbol table, recursive descent quad generator, and linear assembly generator.
- Preprocessor and linker are provided by gcc or clang.

- Basic compilation process is described below -
  - The lexer reads and tokenizes preprocessed C input
  - The tokenized input and semantic values are consumed by the parser, spinning up an abstract syntax tree. 
    - The symbol table is built at this time, maintaining a stack of lexical scopes (more symbol tables).
    - The AST links directly to symbol table entries.
  - The quad generator will descend the AST and produce a linear and externalizable IR.
  - The assembly generator consumes the quads/IR and produces x86 assembly.

## More notes

- The generated code is bad. This should be a lightly optimizing compiler one day.
- The type engine needs a lot of investment.
- There is a fair amount of insanity.
- I don't _try_ to leak memory, but I don't really try not to leak memory either.
- You can summon the oopsies mascot on purpose with the compiler internal
```
#pragma do-perish
```
- You can use `#pragma do-examine yourvariablename` and `#pragma do-dumpsymtab` to peek at symbols/symbol tables. These internals don't work at the start of a block (grammar needs tuning).
- There is a lot of cleanup to be done.
- Be gentle

# Usage
- Dependencies: `bison`, `flex`, and `requirements.txt`.
- To build:

  ```
  $ scons dcc
  ```
- To use: (`./dcc -h` for usage)
  ```
  $ ./dcc yourprogram.c
  $ ./a.out
  ```

- To test:
  ```
  $ scons test
  ```

# License
This project uses the [MIT License](LICENSE.md).

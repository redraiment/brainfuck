# Brainfuck Interpreters and Compilers

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

This is my learning project to practice building compilers.

# About the Project

I'd like to implement some [Brainfuck](https://en.wikipedia.org/wiki/Brainfuck) interpreters and compilers in my favorite programming languages (only Clojure and C for now, and perhaps Haskell in the future). Brainfuck language is simple enough, and it designed for implementing the smallest possible compiler.

# Roadmap

## C Versions

* [x] [ansi-c](https://github.com/redraiment/brainfuck/tree/main/ansi-c): an interpreter in pure ANSI C, without any third-party dependencies.
* [ ] [llvm-c](https://github.com/redraiment/brainfuck/tree/main/llvm-c): a compiler and JIT interpreter in C with flex, bison and LLVM C API etc.
  * [ ] Generating Makefile with [GNU Automake](https://www.gnu.org/software/automake/) and [GNU Autoconf](https://www.gnu.org/software/autoconf/).
  * [x] Building project with [GNU Make](https://www.gnu.org/software/make/).
  * [x] Parsing Command line options with [getopt](https://www.gnu.org/software/libc/manual/html_node/Getopt.html).
    * [x] Option for scripting mode to execute code.
    * [x] Option for preprocess mode to emit LLVM IR.
    * [x] Option for compile mode to emit object file.
    * [x] Option for show overview/help.
  * [x] Lexical analysis with [flex](https://github.com/westes/flex).
  * [x] Syntax analysis with [bison](https://www.gnu.org/software/bison/).
  * [x] Preprocess mode with [LLVM C API](https://llvm.org/doxygen/group__LLVMC.html).
  * [x] Compile mode with [LLVM C API](https://llvm.org/doxygen/group__LLVMC.html).
  * [x] Link mode to generate executable file.
  * [x] Scripting mode with [LLVM MCJIT](https://llvm.org/doxygen/group__LLVMCExecutionEngine.html).
  * [ ] Deploying with [docker](https://hub.docker.com/).

## Clojure Versions

* [ ] [clojure](https://github.com/redraiment/brainfuck/tree/main/clojure): compiler and JIT interpreter in Clojure with instaparse, JNA (to wrap LLVM) etc.
  * [x] Building project with [leiningen](https://github.com/technomancy/leiningen).
  * [x] Translating Brainfuck to Clojure in compiling time with Clojure macro.
  * [ ] Lexical and Syntax analysis with [instaparse](https://github.com/Engelberg/instaparse).
  * [ ] Executing with pure Clojure.
  * [ ] Wrap LLVM with [JNA](https://github.com/java-native-access/jna).
  * [ ] Executing with [LLVM MCJIT](https://llvm.org/doxygen/group__LLVMCExecutionEngine.html).
  * [ ] Deploying with [docker](https://hub.docker.com/).

# Language Specification

Here are some key behaviors:

* Memory size: 30,000 bytes, and initialized to zero.
* Data pointer initialized to point to the leftmost byte of the array.
* Two streams of bytes for input and output.
* End-of-file behavior: setting the cell to 0.
* Use "\n" for end-of-line.
* Commands:

| Character | Meaning |
| -- | -- |
| `#` | Single line comment. **It's an EXTRA command**. |
| `>` | Increment the data pointer (to point to the next cell to the right). |
| `<` | Decrement the data pointer (to point to the next cell to the left). |
| `+` | Increment (increase by one) the byte at the data pointer. |
| `-` | Decrement (decrease by one) the byte at the data pointer. |
| `.` | Output the byte at the data pointer. |
| `,` | Accept one byte of input, storing its value in the byte at the data pointer. |
| `[` | If the byte at the data pointer is zero, then instead of moving the instruction pointer forward to the next command, jump it forward to the command after the matching `]` command. |
| `]` | If the byte at the data pointer is nonzero, then instead of moving the instruction pointer forward to the next command, jump it back to the command after the matching `[` command. |
| others | Comments |

**Note**: Single line comment command (`#`) was added to avoid file path conflict with commands. For example, there is `.` in `#!/bin/brainfuck.exe`, which is output command;

# Getting Started

**TODO**

## Prerequisites

## Installation

# Usage

```sh
brainfuck [OPTIONS] <source-file>
```

It will create an executable file default.

## Options

* `-c/--compile`: only run preprocess, compile and assemble steps, then emit native object (`.o`) to output file. By default, the object file name for a source file is made by replacing the extension with `.o`.
* `-r/--representation`: emit LLVM representation (`.ll`) to standard output.
* `-s/--script`: run source file as Brainfuck script.
* `-m/--enable-single-line-comment`: enable single line comment command `#`. It's useful used with Shebang.
* `-o/--output <output-file>`: write output to file. This applies to whatever sort of output is being produced, whether it be an executable file, an object file, an IR file. If `-o` is not specified, the default executable file name for a source file is made by removing the extension.
* `-h/--help`: show this help and exit.

## Examples

1. Creating an executable file: `brainfuck helloworld.bf`
2. Running a file as scripting: `brainfuck -s helloworld.bf`
3. Using with Shebang: `#!/usr/local/bin/brainfuck -ms`
4. Creating native object file: `brainfuck -c helloworld.bf`
5. Creating LLVM representation file: `brainfuck -p helloworld.bf`

# Snippets

Here some Brainfuck snippets for testing.

## Hello World

From [Wikipedia](https://en.wikipedia.org/wiki/Brainfuck#Hello_World!). It will write "Hello world" to standard output.

```brainfuck
++++++++++
[>+++++++>++++++++++>+++>+<<<<-]
>++.>+.+++++++..+++.>++.<<+++++++++++++++.
>.+++.------.--------.>+.>.
```

## cat

It will read data from standard input and write to standard output directly, until end of file.

```brainfuck
,[.,]
```

## wc

from [brainfuck.org](http://brainfuck.org/wc.b). the standard (line and) word (and character) count utility.

```brainfuck
>>>+>>>>>+>>+>>+[<<],[
    -[-[-[-[-[-[-[-[<+>-[>+<-[>-<-[-[-[<++[<++++++>-]<
        [>>[-<]<[>]<-]>>[<+>-[<->[-]]]]]]]]]]]]]]]]
    <[-<<[-]+>]<<[>>>>>>+<<<<<<-]>[>]>>>>>>>+>[
        <+[
            >+++++++++<-[>-<-]++>[<+++++++>-[<->-]+[+>>>>>>]]
            <[>+<-]>[>>>>>++>[-]]+<
        ]>[-<<<<<<]>>>>
    ],
]+<++>>>[[+++++>>>>>>]<+>+[[<++++++++>-]<.<<<<<]>>>>>>>>]
```

# Contributing

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement". Don't forget to give the project a star! Thanks again!

1. Fork the Project
1. Create your Feature Branch (git checkout -b feature/AmazingFeature)
1. Commit your Changes (git commit -m 'Add some AmazingFeature')
1. Push to the Branch (git push origin feature/AmazingFeature)
1. Open a Pull Request

# License

Distrubuted under the GPLv3 License. See `LICENSE` for more information.

# Contact

* Zhang, Zepeng - [@redraiment](https://twitter.com/redraiment) - [redraiment@gmail.com](mailto:redraiment@gmail.com)

# Acknowledgments

* [Wikipedia](https://en.wikipedia.org/)
* [brainfuck.org](http://brainfuck.org/)
* [LLVM.org](https://llvm.org/)
* [GNU](https://www.gnu.org/)
  * [GNU Automake](https://www.gnu.org/software/automake/)
  * [GNU Autoconf](https://www.gnu.org/software/autoconf/)
  * [GNU Make](https://www.gnu.org/software/make/)
  * [GNU getopt](https://www.gnu.org/software/libc/manual/html_node/Getopt.html)
  * [GNU bison](https://www.gnu.org/software/bison/).
* [flex](https://github.com/westes/flex).
* [Docker](https://www.docker.com/).

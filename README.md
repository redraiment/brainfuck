# Brainfuck Interpreters and Compilers

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

This is my learning project to practice building compiler and interpreter with LLVM C API.

The project is too complext for someone, who just want to implement a simple interpreter for Brainfuck. This [snippet](https://gist.github.com/redraiment/87413e5edc5c33458812492b5358ed61) is a better alternative, which implements the interpreter with one C file only.

# About the Project

There are so many LLVM tutorials in C++, however, I'd like to build a compiler and interpreter with LLVM in C. Therefore, I created this project to build a compiler and JIT interpreter for [Brainfuck](https://en.wikipedia.org/wiki/Brainfuck). Brainfuck language is simple enough, and it designed for implementing the smallest possible compiler.

# Roadmap

* [x] Generating Makefile with [GNU Autoconf](https://www.gnu.org/software/autoconf/).
* [x] Building project with [GNU Make](https://www.gnu.org/software/make/).
* [x] Parsing Command line options with [getopt](https://www.gnu.org/software/libc/manual/html_node/Getopt.html).
* [x] Lexical analysis with [flex](https://github.com/westes/flex).
* [x] Syntax analysis with [bison](https://www.gnu.org/software/bison/).
* [x] Creating LLVM IR file with [LLVM C API](https://llvm.org/doxygen/group__LLVMC.html).
* [x] Creating native object file with [LLVM C API](https://llvm.org/doxygen/group__LLVMC.html).
* [x] Creating executable file with linker.
* [x] Running script file with [LLVM MCJIT](https://llvm.org/doxygen/group__LLVMCExecutionEngine.html).
* [x] Deploying with [docker](https://hub.docker.com/).

# Getting Started

The program can only working on Linux for now.

## Running using Docker (Recommended)

```sh
docker run --rm redraiment/brainfuck brainfuck -v
```

You can see below version information if the above command run success.

```
brainfuck v0.1.1

Home page: <https://github.com/redraiment/brainfuck/>.
E-mail bug reports to: <redraiment@gmail.com>.
```

Example for creating executable file and run it then:

```sh
docker run --rm -v $PWD:/root/ redraiment/brainfuck brainfuck hello-world.bf
docker run --rm -v $PWD:/root/ redraiment/brainfuck ./hello-world
```

## Install with Source Code

Install packages on ArchLinux:

```sh
sudo pacman -S binutils flex bison clang llvm make
```

Install packages on Ubuntu / Debain:

```sh
sudo apt install binutils flex bison clang llvm make
```

Build program:

```
wget https://github.com/redraiment/brainfuck/releases/download/v0.1.1/brainfuck-0.1.1.tar.gz
tar xvf brainfuck-0.1.1.tar.gz
cd brainfuck-0.1.1
./configure CC=clang
make
sudo make install
```

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
* `-v/--version`: show version and exit.

## Examples

1. Creating an executable file: `brainfuck helloworld.bf`
2. Running a file as scripting: `brainfuck -s helloworld.bf`
3. Using with Shebang: `#!/usr/local/bin/brainfuck -ms`
4. Creating native object file: `brainfuck -c helloworld.bf`
5. Creating LLVM representation file: `brainfuck -p helloworld.bf`

# Language Specification

Here are some key behaviors:

* Memory size: 30,000 bytes, and initialized to zero.
* Data pointer initialized to point to the leftmost byte of the array.
* Two streams of bytes for input and output.
* End-of-file behavior: setting the cell to 0.
* Use "\n" for end-of-line.

## Commands

| Character | Meaning |
| -- | -- |
| `>` | Increment the data pointer (to point to the next cell to the right). |
| `<` | Decrement the data pointer (to point to the next cell to the left). |
| `+` | Increment (increase by one) the byte at the data pointer. |
| `-` | Decrement (decrease by one) the byte at the data pointer. |
| `.` | Output the byte at the data pointer. |
| `,` | Accept one byte of input, storing its value in the byte at the data pointer. |
| `[` | If the byte at the data pointer is zero, then instead of moving the instruction pointer forward to the next command, jump it forward to the command after the matching `]` command. |
| `]` | If the byte at the data pointer is nonzero, then instead of moving the instruction pointer forward to the next command, jump it back to the command after the matching `[` command. |
| `#` | Single line comment. **Disabled default**. |
| others | Comments |

**HINT**: Single line comment command (`#`) is an extra command to ignore the text until end-of-line. It was added to avoid command of shebang conflict with Brainfuck commands. For example, there is `-` in `#!/bin/brainfuck -s`, which is backward command of Brainfuck.

## Snippets

Here some Brainfuck snippets for testing.

### Hello World

From [Wikipedia](https://en.wikipedia.org/wiki/Brainfuck#Hello_World!). It will write "Hello world" to standard output.

```brainfuck
++++++++++
[>+++++++>++++++++++>+++>+<<<<-]
>++.>+.+++++++..+++.>++.<<+++++++++++++++.
>.+++.------.--------.>+.>.
```

### cat

It will read data from standard input and write to standard output directly, until end of file.

```brainfuck
,[.,]
```

### wc

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
  * [GNU Autoconf](https://www.gnu.org/software/autoconf/)
  * [GNU Make](https://www.gnu.org/software/make/)
  * [GNU getopt](https://www.gnu.org/software/libc/manual/html_node/Getopt.html)
  * [GNU bison](https://www.gnu.org/software/bison/).
* [flex](https://github.com/westes/flex).
* [Docker](https://www.docker.com/).

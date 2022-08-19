# brainfuck

Here are toy interpreters and compilers of [brainfuck](https://en.wikipedia.org/wiki/Brainfuck) language.

## Projects

* **DONE** [brainfuck-interpreter-in-clojure-macro](https://github.com/redraiment/brainfuck/tree/main/brainfuck-interpreter-in-clojure-macro): it's a clojure macro to translate brainfuck codes to clojure codes in compile time.
* **DONE** [brainfuck-interpreter-in-ansi-c](https://github.com/redraiment/brainfuck/tree/main/brainfuck-interpreter-in-ansi-c): it's an interpreter in pure ANSI C without any third party library dependency.
* **DOING** brainfuck-interpreter-in-llvm-jit: an interpreter with LLVM JIT in C.
* **DOING** brainfuck-compiler-in-llvm: a compiler can generate executable file with flex, bison and LLVM in C.
* samples: some brainfuck samples.

## Language Specification

Information from Wikipedia above.

* Memory size: 30,000 bytes, and initialized to zero.
* Data pointer initialized to point to the leftmost byte of the array.
* Two streams of bytes for input and output.
* Eight commands:

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

* End-of-file behavior: setting the cell to 0.

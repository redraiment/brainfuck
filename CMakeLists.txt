cmake_minimum_required(VERSION 3.20)

# Workaround to prevent project() from enabling default languages
set(CMAKE_NONE_STANDARD_COMPUTE "")
project(brainfuck
  VERSION 0.5.0
  DESCRIPTION "Brainfuck Compiler and Interpreter with LLVM in C"
  HOMEPAGE_URL "https://github.com/redraiment/brainfuck/"
  LANGUAGES "NONE"
)
set(PROJECT_BUG_REPORT "redraiment@gmail.com")

# Compilers Setup

find_package(LLVM REQUIRED CONFIG)
set(CMAKE_C_COMPILER "${LLVM_TOOLS_BINARY_DIR}/clang")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_COMPILER "${LLVM_TOOLS_BINARY_DIR}/clang++")
set(CMAKE_CXX_STANDARD 14)
enable_language(C CXX)

# Directories Setup

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${SRC_DIR}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Preprocess Setup

## Flex & Bison

find_package(FLEX)
find_package(BISON)
flex_target(SCANNER "${SRC_DIR}/scanner.l" "${CMAKE_CURRENT_BINARY_DIR}/scanner.c" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/scanner.h")
bison_target(PARSER "${SRC_DIR}/parser.y" "${CMAKE_CURRENT_BINARY_DIR}/parser.c" DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/parser.h")
add_flex_bison_dependency(SCANNER PARSER)

## Configure Files

configure_file("${SRC_DIR}/options.h.in" "${CMAKE_CURRENT_BINARY_DIR}/options.h")

# Compiler & Linker Options

set(LLVM_CONFIG_EXECUTABLE "${LLVM_TOOLS_BINARY_DIR}/llvm-config")
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --cflags OUTPUT_VARIABLE LLVM_C_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags OUTPUT_VARIABLE LLVM_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs --link-static OUTPUT_VARIABLE LLVM_SYSTEM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs --link-static OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags OUTPUT_VARIABLE LLVM_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LLVM_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -static ${LLVM_LD_FLAGS}")

# Libraries Setup

## LLD libraries

find_library(LIB_LLD_COMMON lldCommon REQUIRED HINTS ${LLVM_LIBRARY_DIR})
find_library(LIB_LLD_ELF lldELF REQUIRED HINTS ${LLVM_LIBRARY_DIR})

## CRT libraries

find_program(XXD_EXECUTABLE NAMES xxd REQUIRED)
set(CRT_C_FILE "${CMAKE_CURRENT_BINARY_DIR}/crt.c")
file(WRITE "${CRT_C_FILE}" "#include \"crt.h\"\n\n")

function(import_musl_library FILE_NAME)
  find_file(MUSL_LIBRARY_FILE
    NAMES ${FILE_NAME}
    HINTS ${MUSL_PATH}
    /usr/lib/x86_64-linux-musl /usr/lib/i386-linux-musl # ubuntu/debian
    /usr/x86_64-linux-musl/lib64 /usr/i386-linux-musl/lib # centos/rhel
    /usr/lib/musl/lib # archlinux
  )
  string(REGEX REPLACE "/[^/]*$" "/" VARIABLE_PREFIX "${MUSL_LIBRARY_FILE}")
  string(REGEX REPLACE "[-/.]" "_" VARIABLE_PREFIX "${VARIABLE_PREFIX}")
  string(TOUPPER "${VARIABLE_PREFIX}" VARIABLE_PREFIX)

  if (MUSL_LIBRARY_FILE)
    execute_process(COMMAND ${XXD_EXECUTABLE} -C -i ${MUSL_LIBRARY_FILE} OUTPUT_VARIABLE MUSL_LIBRARY_CONTENT)
    string(REPLACE "${VARIABLE_PREFIX}" "" MUSL_LIBRARY_CONTENT "${MUSL_LIBRARY_CONTENT}")
    file(APPEND "${CRT_C_FILE}" "${MUSL_LIBRARY_CONTENT}\n")
  else()
    message(FATAL_ERROR "MUSL library `${FILE_NAME}' not found, specify `MUSL_PATH' for search.")
  endif()
  unset(MUSL_LIBRARY_FILE CACHE)
  unset(VARIABLE_PREFIX CACHE)
  unset(MUSL_LIBRARY_CONTENT CACHE)
endfunction()

import_musl_library(Scrt1.o)
import_musl_library(crti.o)
import_musl_library(crtn.o)
import_musl_library(libc.a)

# Target

add_executable(brainfuck "${SRC_DIR}/ast.c" "${SRC_DIR}/compiler.c" "${SRC_DIR}/engine.c" "${SRC_DIR}/fs.cpp" "${SRC_DIR}/linker.cpp" "${SRC_DIR}/options.c" "${SRC_DIR}/main.c" "${FLEX_SCANNER_OUTPUTS}" "${BISON_PARSER_OUTPUTS}" "${CRT_C_FILE}")
target_link_libraries(brainfuck PRIVATE ${LLVM_SYSTEM_LIBS} ${LLVM_LIBS} ${LIB_LLD_COMMON} ${LIB_LLD_ELF})

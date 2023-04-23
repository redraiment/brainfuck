cmake_minimum_required(VERSION 3.20)

# Workaround to prevent project() from enabling default languages
set(CMAKE_NONE_STANDARD_COMPUTE "")
project(brainfuck
  VERSION 0.4.2
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

find_library(LIB_LLD_COMMON lldCommon HINTS ${LLVM_LIBRARY_DIR})
if(NOT LIB_LLD_COMMON)
  message(FATAL_ERROR "liblldCommon.a not found")
endif()

find_library(LIB_LLD_ELF lldELF HINTS ${LLVM_LIBRARY_DIR})
if(NOT LIB_LLD_ELF)
  message(FATAL_ERROR "liblldELF.a not found")
endif()

# Target

add_executable(brainfuck "${SRC_DIR}/ast.c" "${SRC_DIR}/compiler.c" "${SRC_DIR}/engine.c" "${SRC_DIR}/linker.cpp" "${SRC_DIR}/options.c" "${SRC_DIR}/main.c" "${FLEX_SCANNER_OUTPUTS}" "${BISON_PARSER_OUTPUTS}")
target_link_libraries(brainfuck PRIVATE ${LLVM_SYSTEM_LIBS} ${LLVM_LIBS} ${LIB_LLD_COMMON} ${LIB_LLD_ELF})

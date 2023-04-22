cmake_minimum_required(VERSION 3.20)

# C Compiler
find_program(CLANG_EXECUTABLE NAMES clang clang-15 clang-14 clang-13 clang-12 clang-11 clang-10)
if(NOT CLANG_EXECUTABLE)
  message(FATAL_ERROR "clang not found. Make sure it is installed and in your path.")
else()
  message(STATUS "CLANG_EXECUTABLE=${CLANG_EXECUTABLE}")
endif()
set(CMAKE_C_COMPILER "${CLANG_EXECUTABLE}")
set(CMAKE_C_STANDARD 11)

# CXX Compiler
find_program(CLANGXX_EXECUTABLE NAMES clang++ clang++-15 clang++-14 clang++-13 clang++-12 clang++-11 clang++-10)
if(NOT CLANGXX_EXECUTABLE)
  message(FATAL_ERROR "clang++ not found. Make sure it is installed and in your path.")
else()
  message(STATUS "CLANGXX_EXECUTABLE=${CLANGXX_EXECUTABLE}")
endif()
set(CMAKE_CXX_COMPILER "${CLANGXX_EXECUTABLE}")
message(STATUS "CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
set(CMAKE_CXX_STANDARD 14)

# Flex & Bison

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
flex_target(SCANNER "scanner.l" "scanner.c" DEFINES_FILE "scanner.h")
bison_target(PARSER "parser.y" "parser.c" DEFINES_FILE "parser.h")
add_flex_bison_dependency(SCANNER PARSER)

# llvm-config
unset(LLVM_CONFIG_EXECUTABLE)
find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config llvm-config-15 llvm-config-14 llvm-config-13 llvm-config-12 llvm-config-11 llvm-config-10)
if (NOT LLVM_CONFIG_EXECUTABLE)
  message(FATAL_ERROR "llvm-config not found. Make suer LLVM is installed and in your path.")
else()
  message(STATUS "LLVM_CONFIG_EXECUTABLE=${LLVM_CONFIG_EXECUTABLE}")
endif()

execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --cflags OUTPUT_VARIABLE LLVM_C_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "LLVM_C_FLAGS=${LLVM_C_FLAGS}")
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags OUTPUT_VARIABLE LLVM_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "LLVM_CXX_FLAGS=${LLVM_CXX_FLAGS}")
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir OUTPUT_VARIABLE LLVM_LIBRARY_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "LLVM_LIBRARY_DIR=${LLVM_LIBRARY_DIR}")
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs --link-static OUTPUT_VARIABLE LLVM_SYSTEM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "LLVM_SYSTEM_LIBS=${LLVM_SYSTEM_LIBS}")
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs --link-static OUTPUT_VARIABLE LLVM_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "LLVM_LIBS=${LLVM_LIBS}")
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags OUTPUT_VARIABLE LLVM_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "LLVM_LD_FLAGS=${LLVM_LD_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LLVM_C_FLAGS}")
message(STATUS "CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -static ${LLVM_LD_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}")

# lld libraries

find_library(LIB_LLD_COMMON lldCommon HINTS ${LLVM_LIBRARY_DIR})
if(NOT LIB_LLD_COMMON)
  message(FATAL_ERROR "liblldCommon.a not found")
else()
  message(STATUS "LIB_LLD_COMMON=${LIB_LLD_COMMON}")
endif()

find_library(LIB_LLD_ELF lldELF HINTS ${LLVM_LIBRARY_DIR})
if(NOT LIB_LLD_ELF)
  message(FATAL_ERROR "liblldELF.a not found")
else()
  message(STATUS "LIB_LLD_ELF=${LIB_LLD_ELF}")
endif()

# Compiling and Linking

project(brainfuck VERSION 0.4.0 HOMEPAGE_URL "https://github.com/redraiment/brainfuck/" LANGUAGES C CXX)
set(PROJECT_BUG_REPORT "redraiment@gmail.com")
configure_file("options.h.in" "options.h")

add_executable(brainfuck ast.c compiler.c engine.c linker.cpp options.c main.c "${FLEX_SCANNER_OUTPUTS}" "${BISON_PARSER_OUTPUTS}")
target_link_libraries(brainfuck PRIVATE ${LLVM_SYSTEM_LIBS} ${LLVM_LIBS} ${LIB_LLD_COMMON} ${LIB_LLD_ELF})

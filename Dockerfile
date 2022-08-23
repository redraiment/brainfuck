FROM ubuntu
WORKDIR /workspaces/
COPY ast.c compiler.c engine.c linker.c options.c main.c parser.y scanner.l \
     ast.h compiler.h engine.h linker.h options.h \
     configure Makefile.in config.h.in \
     /workspaces/
RUN apt-get update \
 && apt-get upgrade -y \
 && apt-get install -y flex bison clang llvm make \
 && ./configure CC=clang \
 && make

FROM ubuntu
WORKDIR /root/
COPY --from=0 /workspaces/brainfuck /usr/local/bin
RUN apt-get update  \
 && apt-get upgrade -y  \
 && apt-get install -y llvm-dev binutils

FROM ubuntu
WORKDIR /workspaces/
COPY ast.c compiler.c engine.c linker.cpp options.c main.c parser.y scanner.l \
     ast.h compiler.h engine.h linker.h options.h \
     configure Makefile.in config.h.in \
     /workspaces/
RUN sed -i 's@//.*archive.ubuntu.com@//mirrors.ustc.edu.cn@g' /etc/apt/sources.list \
 && apt-get update \
 && apt-get upgrade -y \
 && apt-get install --no-install-recommends -y flex bison clang-15 lld-15 liblld-15-dev llvm-15 llvm-15-dev llvm-15-tools zlib1g-dev libtinfo-dev binutils-dev make \
 && ln -s /usr/bin/clang-15 /usr/bin/clang \
 && ln -s /usr/bin/clang++-15 /usr/bin/clang++ \
 && ln -s /usr/bin/ld.lld-15 /usr/bin/ld.lld \
 && ln -s /usr/bin/llvm-config-15 /usr/bin/llvm-config \
 && ./configure \
 && make

FROM alpine
COPY --from=0 /workspaces/brainfuck /usr/local/bin
RUN sed -i 's/dl-cdn.alpinelinux.org/mirrors.ustc.edu.cn/g' /etc/apk/repositories \
  && apk add --no-cache musl-dev
WORKDIR /workspaces/
CMD ["/usr/local/bin/brainfuck"]

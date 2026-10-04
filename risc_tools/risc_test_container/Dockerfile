FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install RISC-V cross-compiler, C library, build tools, and utilities
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-riscv64-unknown-elf \
    binutils-riscv64-unknown-elf \
    picolibc-riscv64-unknown-elf \
    build-essential \
    autoconf \
    ca-certificates \
    cmake \
    make \
    git \
    device-tree-compiler \
    qemu-system-misc \
    && rm -rf /var/lib/apt/lists/*

# Clone and compile riscv-tests for RV32
WORKDIR /opt
RUN git clone --depth 1 https://github.com/riscv-software-src/riscv-tests \
    && cd riscv-tests \
    && git submodule update --init --recursive --depth 1 \
    && autoconf \
    && ./configure --prefix=/opt/riscv-tests --with-xlen=32 \
    && make -C isa -j$(nproc) XLEN=32 RISCV_GCC_OPTS='--specs=picolibc.specs -static -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles'

ENV RISCV_TESTS=/opt/riscv-tests/isa
WORKDIR /workspace

CMD ["/bin/bash"]
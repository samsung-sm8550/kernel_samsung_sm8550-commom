#!/bin/bash

#set -e

KERNEL_DEFCONFIG=gki_defconfig
CLANG_VERSION=clang-r536225
CLANG_DIR="$HOME/tools/google-clang"
CLANG_BINARY="$CLANG_DIR/bin/clang"
export PATH="$CLANG_DIR/bin:$PATH"
export KBUILD_COMPILER_STRING="$($CLANG_BINARY --version | head -n 1 | perl -pe 's/\(http.*?\)//gs' | sed -e 's/  */ /g' -e 's/[[:space:]]*$//')"

if ! [ -d "$CLANG_DIR" ]; then
    echo "Clang not found! Cloning..."
    mkdir -p "$CLANG_DIR"
    if ! wget --show-progress -O "$CLANG_DIR/${CLANG_VERSION}.tar.gz" "https://android.googlesource.com/platform/prebuilts/clang/host/linux-x86/+archive/refs/heads/main/${CLANG_VERSION}.tar.gz"; then
        echo "Cloning failed! Aborting..."
        exit 1
    fi
    echo "Cloning successful. Extracting the tar file..."
    tar -xzf "$CLANG_DIR/${CLANG_VERSION}.tar.gz" -C "$CLANG_DIR"
    rm "$CLANG_DIR/${CLANG_VERSION}.tar.gz"
fi

make -j$(nproc --all) CC=clang \
                      LD=ld.lld \
                      LLVM=1 \
                      LLVM_IAS=1 \
                      $KERNEL_DEFCONFIG
 
make -j$(nproc --all) CC=clang \
                      LD=ld.lld \
                      LLVM=1 \
                      LLVM_IAS=1

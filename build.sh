#!/bin/bash

#set -e

KERNEL_DEFCONFIG=gki_defconfig
CLANG_VERSION=clang-r536225
CLANG_DIR="$HOME/tools/google-clang"
CLANG_BINARY="$CLANG_DIR/bin/clang"
export PATH="$CLANG_DIR/bin:$PATH"
export KBUILD_COMPILER_STRING="$($CLANG_BINARY --version | head -n 1 | perl -pe 's/\(http.*?\)//gs' | sed -e 's/  */ /g' -e 's/[[:space:]]*$//')"
export KernelSU=true
export SUSFS4KSU=true

# Function to ensure specific configuration is set in defconfig
ensure_config() {
    local config_file=$1
    shift
    for config in "$@"; do
        if ! grep -q "^$config=y" "$config_file"; then
            if grep -q "^# $config is not set" "$config_file"; then
                sed -i "s|^# $config is not set|$config=y|" "$config_file"
            else
                echo "$config=y" >>"$config_file"
            fi
        fi
    done
}

# Clang setup
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

# KernelSU setup
if [ "$KernelSU" = true ]; then
    if [ "$SUSFS4KSU" = true ]; then
        echo "SUSFS4KSU is enabled. Cloning GalaxyBuild KernelSU-Next..."
        curl -LSs "https://raw.githubusercontent.com/galaxybuild-project/KernelSU-Next/next/kernel/setup.sh" | bash -
        ensure_config arch/arm64/configs/$KERNEL_DEFCONFIG CONFIG_KSU CONFIG_KSU_SUSFS CONFIG_KSU_SUSFS_SUS_SU KSU_SUSFS_HAS_MAGIC_MOUNT
    else
        if [ ! -d "KernelSU" ]; then
            echo "KernelSU folder not found. Cloning..."
            curl -LSs "https://raw.githubusercontent.com/rifsxd/KernelSU-Next/next/kernel/setup.sh" | bash -
        fi
        ensure_config arch/arm64/configs/$KERNEL_DEFCONFIG CONFIG_KSU
    fi
fi

# Build Kernel
make -j$(nproc --all) CC=clang \
                      LD=ld.lld \
                      LLVM=1 \
                      LLVM_IAS=1 \
                      $KERNEL_DEFCONFIG
 
make -j$(nproc --all) CC=clang \
                      LD=ld.lld \
                      LLVM=1 \
                      LLVM_IAS=1

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
        curl -LSs "https://raw.githubusercontent.com/rifsxd/KernelSU-Next/next/kernel/setup.sh" | bash -s next
        ensure_config arch/arm64/configs/$KERNEL_DEFCONFIG CONFIG_KSU CONFIG_KSU_SUSFS CONFIG_KSU_SUSFS_SUS_SU KSU_SUSFS_HAS_MAGIC_MOUNT
    else
        if [ ! -d "KernelSU" ]; then
            echo "KernelSU folder not found. Cloning..."
            curl -LSs "https://raw.githubusercontent.com/rifsxd/KernelSU-Next/next/kernel/setup.sh" | bash -s next
        fi
        ensure_config arch/arm64/configs/$KERNEL_DEFCONFIG CONFIG_KSU
    fi
fi


### Clone SusFS for KernelSU
    git clone https://gitlab.com/simonpunk/susfs4ksu.git -b gki-android13-5.15
    echo "SusFS cloned successfully!"

        
##################
# Copy necessary files and patches
    # cp susfs4ksu/kernel_patches/KernelSU/10_enable_susfs_for_ksu.patch KernelSU/
    cp KernelSU-Next-Implement-SUSFS-v1.5.5-Universal.patch KernelSU-Next/
    echo "Copying patch for SuSFS successfully"

        
#### Copy Patches To Core Kernel
    cp susfs4ksu/kernel_patches/50_add_susfs_in_gki-android13-5.15.patch .
    # cp Fixes/69_hide_stuff.patch .
    # cp Fixes/susfs.c_fix_ksu.patch .
    cp susfs4ksu/kernel_patches/fs/* fs/
    cp susfs4ksu/kernel_patches/include/linux/* include/linux/

### Apply SuSFS Patch To KernelSU
    cd KernelSU-Next
    # patch -p1 --fuzz=3 --forward < 10_enable_susfs_for_ksu.patch || true
    patch -p1 --fuzz=3 --forward < KernelSU-Next-Implement-SUSFS-v1.5.5-Universal.patch || true
    echo "Patching Susfs with KSU successfully"
    cd ..
    echo "Back To kernel root folder"

    # Apply SuSFS Patch To Core Kernel
    patch -p1 --fuzz=3 < 50_add_susfs_in_gki-android13-5.15.patch || true
    # patch -p1 --fuzz=3 < 69_hide_stuff.patch || true
    # patch -p1 --fuzz=3 < susfs.c_fix_ksu.patch || true



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

# If build is successful, copy the /out/arch/arm64/boot/Image to anykernel3 folder
if [ -f "out/arch/arm64/boot/Image" ]; then
    cp out/arch/arm64/boot/Image anykernel3/
    echo "Kernel built successfully!"
else
    echo "Kernel build failed!"
    exit 1
fi

# make zip file
# H61M-Kernel-$(make kernelversion).zip
# if include kernelsu, H61M-Kernel-$(make kernelversion)-NEXT.zip
# if include susfs4ksu, H61M-Kernel-$(make kernelversion)-NEXT-SUSFS.zip

# Determine kernel version
KERNEL_VERSION=$(make kernelversion | grep -v "Entering\|Leaving")

# Determine the zip file name
ZIP_NAME="H61M-Kernel-$KERNEL_VERSION"
if [ "$KernelSU" = true ]; then
    ZIP_NAME="$ZIP_NAME-NEXT"
    if [ "$SUSFS4KSU" = true ]; then
        ZIP_NAME="$ZIP_NAME-SUSFS"
    fi
fi
ZIP_NAME="$ZIP_NAME.zip"

# Create the zip file
if [ -d "anykernel3" ]; then
    echo "Creating zip file: $ZIP_NAME"
    cd anykernel3
    zip -r9 "../$ZIP_NAME" ./*
    cd ..
    echo "Zip file created: $ZIP_NAME"
else
    echo "Anykernel3 folder not found! Cannot create zip file."
    exit 1
fi

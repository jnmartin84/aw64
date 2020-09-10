#! /bin/bash
# N64 toolchain install script for Unix distributions by Shaun Taylor.
# Tested working on Ubuntu 9.04 ia32 and Debian sparc64.

# You may be prompted for your password for installation steps.
# If the script tells you that writing failed on chksum64 and n64tool,
# this does not mean it failed.  The script simply retried with sudo
# and your password was cached.

# Before calling this script, make sure you have GMP, MPFR and TexInfo
# packages installed in your system.  On a Debian-based system this is
# achieved by typing the following commands:
#
# sudo apt-get install libmpfr-dev
# sudo apt-get install texinfo

# Exit on error
set -e

# EDIT THIS LINE TO CHANGE YOUR INSTALL PATH!
export INSTALL_PATH=${N64_INST:-/usr/local}

# Set up path for newlib to compile later
export PATH=$PATH:$INSTALL_PATH/bin

# Versions
export BINUTILS_V=2.35
export GCC_V=10.2.0
export NEWLIB_V=2.2.0

# Download stage
wget -c ftp://sourceware.org/pub/binutils/releases/binutils-$BINUTILS_V.tar.bz2
wget -c ftp://sourceware.org/pub/gcc/releases/gcc-$GCC_V/gcc-$GCC_V.tar.gz
wget -c ftp://sourceware.org/pub/newlib/newlib-$NEWLIB_V.tar.gz

# Extract stage
test -d binutils-$BINUTILS_V || tar -xvjf binutils-$BINUTILS_V.tar.bz2
test -d gcc-$GCC_V || tar -xvzf gcc-$GCC_V.tar.gz
test -d newlib-$NEWLIB_V || tar -xvzf newlib-$NEWLIB_V.tar.gz

# Binutils and newlib support compiling in source directory, GCC does not
rm -rf gcc_compile
mkdir gcc_compile

# Compile binutils
cd binutils-$BINUTILS_V
./configure --prefix=${INSTALL_PATH} --target=mips64-elf --with-cpu=mips64vr4300 --disable-werror
make
make install || sudo make install || su -c "make install"

# Compile gcc (pass 1)
cd ../gcc_compile
../gcc-$GCC_V/configure --prefix=${INSTALL_PATH} --target=mips64-elf --enable-languages=c --without-headers --with-newlib --disable-libssp --enable-multilib --disable-shared --with-gcc --with-gnu-ld --with-gnu-as --disable-threads --disable-win32-registry --disable-nls --disable-debug --disable-libmudflap --disable-werror
make
make install || sudo make install || su -c "make install"

# Compile newlib
cd ../newlib-$NEWLIB_V
CFLAGS="-O2" CXXFLAGS="-O2 -fno-rtti -fno-exceptions" ./configure --target=mips64-elf --prefix=${INSTALL_PATH} --with-cpu=mips64vr4300 --disable-threads --disable-libssp  --disable-werror
make
make install || sudo env PATH="$PATH" make install || su -c "env PATH=$PATH make install"

# Compile gcc (pass 2)
cd ..
rm -rf gcc_compile
mkdir gcc_compile
cd gcc_compile
CFLAGS_FOR_TARGET="-G0 -O2" CXXFLAGS_FOR_TARGET="-G0 -O2 -fno-rtti -fno-exceptions" ../gcc-$GCC_V/configure --prefix=${INSTALL_PATH} --target=mips64-elf --enable-languages=c,c++ --with-newlib --disable-libssp --enable-multilib --disable-shared --with-gcc --with-gnu-ld --with-gnu-as --disable-threads --disable-win32-registry --disable-nls --disable-debug --disable-libmudflap
make
make install || sudo make install || su -c "make install"

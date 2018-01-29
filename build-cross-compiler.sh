#!/bin/sh

PREFIX=`pwd`/cross
WORKDIR=`mktemp -d`

echo "Installing cross-compiler to $PREFIX"
echo "Building in directory $WORKDIR"

pushd "$WORKDIR"

# get and extract sources

if [ ! -d binutils-2.29.1 ]
then
	curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.29.1.tar.gz
	tar -zxf binutils-2.29.1.tar.gz
fi

if [ ! -d gcc-7.2.0 ]
then
	curl -O https://ftp.gnu.org/gnu/gcc/gcc-7.2.0/gcc-7.2.0.tar.gz
	tar -zxf gcc-7.2.0.tar.gz
fi

# build and install libtools
cd binutils-2.29.1
./configure --prefix="$PREFIX" --target=i686-elf --disable-nls --disable-werror --with-sysroot
make && make install
cd ..

# build and install gcc
mkdir gcc-7.2.0-elf-objs
cd gcc-7.2.0-elf-objs
../gcc-7.2.0/configure --prefix="$PREFIX" --target=i686-elf --disable-nls --enable-languages=c --without-headers
make all-gcc && make all-target-libgcc && make install-gcc && make install-target-libgcc
cd ..

popd
rm -rf "$WORKDIR"


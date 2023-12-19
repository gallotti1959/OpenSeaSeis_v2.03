#!/bin/bash

# -------------------------------------------------------
# SEASEIS make utility
# Windows 32bit version, compiled on Linux system
#
# NOTES
# This make utility works with the MinGW cross-compiler, installed on a Linux system.
# MinGW is available at http://mingw-w64.sourceforge.net/
#
# USAGE
# (1) Build Linux version using ./make_seaseis.sh
# (2) Build Windows 32bit version using ./make_seaseis_win32.sh
# (3) Copy directory 'win_bin' to Windows system, rename to 'bin' and add to binary PATH
#
# -------------------------------------------------------
#
WINVER=32

# MinGW 'bin' directory must be in the binary path. For example:
# export PATH=$PATH:/disk/sources/mingw/mingw${WINVER}_linux/bin
# Linux Ubuntu package names for win32 compilers:
#   g++-mingw-w64-x86-64
#   gfortran-mingw-w64-x86-64

export SRCDIR=./src
export JAVADIR=./java
export WIN_LIBDIR=./win/win${WINVER}

export CPP=i686-w64-mingw32-g++
export CC=i686-w64-mingw32-gcc
export LD=i686-w64-mingw32-g++
export F77=i686-w64-mingw32-gfortran
export MAKE=make

export GLOBAL_FLAGS="-fexpensive-optimizations -O3 -Wno-long-long -Wall -pedantic"
export F77_FLAGS="-ffixed-line-length-132 -O3 -fexpensive-optimizations"
export COMMON_FLAGS="${GLOBAL_FLAGS} -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE"
export RM="rm -f"
export COPY=cp

# -------------------------------------------------------------------

source $SRCDIR/make/linux/set_environment.sh

export BINDIR=${LIBDIR}/../win${WINVER}_bin
export OBJDIR=${LIBDIR}/../win${WINVER}_obj

echo "SeaSeis ${VERSION} source root directory:  '${CSEISDIR_SRCROOT}'"
echo "SeaSeis ${VERSION} obj/lib/bin root dir:   '${CSEISDIR}'"
echo "SeaSeis ${VERSION} library directory:      '${LIBDIR}'"

# -------------------------------------------------------------------
# Check if output directories exist or not. If not create them

if [ ! -d ${LIBDIR} ]; then
   echo "Directory ${LIBDIR} does not exist"
   echo "Run make_seaseis.sh first before running this make script"
   exit 1
fi

if [ ! -d $OBJDIR ]; then
  mkdir $OBJDIR
fi
if [ ! -d $OBJDIR ]; then
  echo "$OBJDIR is not a directory" ; exit 1
fi

if [ ! -d $BINDIR ]; then
  mkdir $BINDIR
fi
if [ ! -d $BINDIR ]; then
  echo "$BINDIR is not a directory" ; exit 1
fi

# -------------------------------------------------------------------
# Build CSEIS

# Create module 'h' file from module list (${SRCDIR}/include/cseis_modules.txt)
src/make/linux/prepare_cseis_build.sh

echo "Start building CSEIS..."

if [ -e $BINDIR/seaseis.exe ]; then
  ${RM} $BINDIR/seaseis.exe
fi

$MAKE -f $SRCDIR/make/win/Makefile_all
$MAKE -f $SRCDIR/make/win/Makefile_seaview

cp ${JAVADIR}/jar/CSeisLib.jar ${BINDIR}
cp ${JAVADIR}/jar/SeaView.jar ${BINDIR}
cp ${JAVADIR}/bin/seaview.bat ${BINDIR}
cp ${JAVADIR}/bin/plotimage.bat ${BINDIR}

echo "End..."

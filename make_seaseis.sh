#!/bin/bash

# Usage:   make_seaseis.sh [option]
# Options: clean, bleach, verbose, debug

#*****************************************************
# Compiler settings
#
export CPP=g++
export CC=gcc
export F77=gfortran
export LD=$CPP
export BUILD_F77=1  # Set to 0 if Fortran compiler is not available
export BUILD_SU=0   # Set to 1 if SU module shall be compiled. Requires special SU installation, see file doc/README_SEISMIC_UNIX

#*****************************************************
# Make file settings, command line arguments
#
export MAKE_BLEACH=0
export MAKE_CLEAN=0
export MAKE_DEBUG=0
export VERBOSE=0
export NO_MAKE_BUILD=0

# Determine special setting for shared object build (SONAME)
unameString=$( uname | awk '{print tolower($0)}' )
if [ $unameString == "linux" ]; then
  export platform="linux"
  export SONAME=soname
elif [ $unameString == "darwin" ]; then
  export platform="apple"
  export SONAME=install_name
else
  # ..otherwise just assume Linux
  export platform="linux"
  export SONAME=soname
fi

make_argument="-s"

export GLOBAL_FLAGS="-fexpensive-optimizations -O3 -Wno-long-long -Wall -pedantic"
export F77_FLAGS="-ffixed-line-length-132"

for arg in $@
do
    if [ $arg == "verbose" ]; then
        export VERBOSE=1
        make_argument=""
    fi
    if [ $arg == "debug" ]; then
        export MAKE_DEBUG=1
        make_argument=""
        GLOBAL_FLAGS="-g -DOS_DEBUG=1 -Wno-long-long -Wall -pedantic -Wconversion"
        F77_FLAGS+=" -g"
    fi
    if [ $arg == "clean" ]; then
        export MAKE_CLEAN=1
        make_argument="clean"
    fi
    if [ $arg == "bleach" ]; then
        export MAKE_BLEACH=1
        make_argument="bleach"
    fi
done

numMakeOptions=$(echo "${MAKE_BLEACH} + ${MAKE_CLEAN} + ${MAKE_DEBUG}" | bc -l)
if [ ${numMakeOptions} -gt 1 ]; then
    echo "ERROR: Too many make options. Specify one option only: debug, clean or bleach"
    exit
fi
export COMMON_FLAGS="${GLOBAL_FLAGS} -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE"
export RM="rm -f"

#********************************************************************************
# Setup environment variables for Seaseis directories
#

source src/make/linux/set_environment.sh

# *** FOR DEVELOPERS:  Comment out the following three lines to skip legal statement etc
./license.sh
./mailhome.sh
make_argument=""; export VERBOSE=1


echo "SeaSeis ${VERSION} source root directory:  '${CSEISDIR_SRCROOT}'"
echo "SeaSeis ${VERSION} obj/lib/bin root dir:   '${CSEISDIR}'"
echo "SeaSeis ${VERSION} library directory:      '${LIBDIR}'"
mkdir -p ${LIBDIR}/include
export CSEIS_MODULE_INCS="-I${LIBDIR}/include"

#********************************************************************************
# Check if FFTW3 library is installed
# How to install fftw library for use with OpenSeaSeis:
# 1) Download and extract fftw source code distribution from http://www.fftw.org/download.html
# 2) ./configure --enable-float
# 3) make
# 4) make install

export NO_FFTW=$($CC -lfftw3f 2>&1 | grep -i "cannot find -l" | wc -l)
if [ ${NO_FFTW} -eq 0 ]; then
    export NO_FFTW=$($CC -lfftw3f 2>&1 | grep -i "library not found for" | wc -l)
fi
if [ ${NO_FFTW} -eq 1 ]; then
    echo "WARNING: Library 'libfftw3f.so' not found in standard library path."
    echo " - Seaseis FFT modules will be compiled without FFTW3."
    echo " - Visit www.fftw.org to download and install FFTW libraries."
fi

#********************************************************************************
# Make Seaseis
#

${CSEISDIR_SRCROOT}/src/make/linux/cmake.sh ${make_argument}

if [ ${MAKE_BLEACH} -eq 0 -a ${MAKE_CLEAN} -eq 0 ]; then
    echo "Auto-generate HTML self-documentation:  ${BINDIR}/seaseis -html > ${DOCDIR}/SeaSeis_help.html"
    ${BINDIR}/seaseis -html > ${DOCDIR}/SeaSeis_help.html
    ${THISDIR}/set_links ${VERSION}
fi

echo "Done."

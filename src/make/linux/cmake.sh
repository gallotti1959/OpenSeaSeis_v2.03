#!/bin/bash

# -------------------------------------------------------
# CSEIS make utility
#
# Usage: cmake.sh [make argument list]
# The make argument list can consist of any arguments for the make utility.
#

# -------------------------------------------------------
# Set argument list for 'make'
arg_list=$@

# Check directories
${CSEISDIR_SRCROOT}/src/make/linux/check_dirs.sh
ret=$?; if [ $ret -ne 0 ]; then exit $ret; fi

#-------------------------------------------------------------------
# Build SeaSeis

# Go to source root directory before running make.
# Most Make files are relative to source root directory.
cd ${CSEISDIR_SRCROOT}

# Build module header files
if [ $NO_MAKE_BUILD -ne 1 ]; then
  make ${arg_list} -f src/make/linux/Makefile_build
fi

echo "Building SeaSeis..."
make ${arg_list} -f src/make/linux/Makefile_libs
make ${arg_list} -f src/make/linux/Makefile_segy
make ${arg_list} -f src/make/linux/Makefile_segd
make ${arg_list} -f src/make/linux/Makefile_main
echo "Done."

#--------------------------------------------------
# Build modules

echo "Building SeaSeis modules..."

makefiles=$(find src/cs/modules -name Makefile -print)
# Add make files for modules that use FFTW library:
if [ ${NO_FFTW} -eq 0 ]; then
    makefiles="$makefiles $(find src/cs/modules -name Makefile_fftw -print)"
else
    makefiles="$makefiles $(find src/cs/modules -name Makefile_nofftw -print)"
fi
# Add make files for Fortran modules:
if [ ${BUILD_F77} -eq 1 ]; then
    makefiles="$makefiles $(find src/cs/modules -name Makefile_f77 -print)"
fi

for makefile in $makefiles
do
  make ${arg_list} -f $makefile
done

echo "Done."

#--------------------------------------------------
# Build SU module if requested
if [ ${BUILD_SU} -eq 1 ]; then
  echo "Building SU SeaSeis module & associated SU modules..."
  make ${arg_list} -f src/cs/su/Makefile_sulib
  make ${arg_list} -f src/cs/su/Makefile_su
  make ${arg_list} -f src/cs/su/Makefile
  echo "Done."
fi

#--------------------------------------------------
# Build SeaView

echo "Building SeaView..."
make ${arg_list} -f src/make/linux/Makefile_seaview
echo "Building XCSeis..."
make ${arg_list} -f src/make/linux/Makefile_xcseis
echo "Done."

if [ -f ${LIBDIR}/seaseis ]; then
 \cp -f ${LIBDIR}/seaseis ${BINDIR}
fi

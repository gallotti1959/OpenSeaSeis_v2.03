echo off

rem -------------------------------------------------------
rem SEASEIS make utility
rem Windows 32bit version, compiled on Windows system
rem
rem NOTES
rem This make utility works with the MinGW cross-compiler, installed on a Windows system.
rem MinGW is available at http://mingw-w64.sourceforge.net/
rem
rem USAGE
rem (1) Build Windows 32bit version using make_seaseis_win32.bat
rem (2) Add 'bin' directory to binary PATH
rem

set SRCDIR=.\src
set JAVADIR=.\java
set CSEISDIR=.\..

set WIN_LIBDIR=win\win32

set CPP=g++.exe
set CC=gcc.exe
set LD=g++.exe
set F77=gfortran.exe
rem set CPP=x86_64-w64-mingw32-g++.exe
rem set CC=x86_64-w64-mingw32-gcc.exe
rem set LD=x86_64-w64-mingw32-g++.exe
rem set F77=x86_64-w64-mingw32-gfortran.exe
set MAKE=mingw32-make

set BINDIR=%CSEISDIR%\bin
set LIBDIR=%BINDIR%
set OBJDIR=%CSEISDIR%\obj
set GLOBAL_FLAGS=-fexpensive-optimizations -O3 -Wno-long-long -Wall -pedantic
set F77_FLAGS=-ffixed-line-length-132 -O3 -fexpensive-optimizations
set COMMON_FLAGS=%GLOBAL_FLAGS% -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE
set RM="del /F"
set COPY=copy

rem -------------------------------------------------------------------
rem Check if output directories exist or not. If not create them

if not exist %OBJDIR% ( mkdir %OBJDIR% )
if not exist %BINDIR% ( mkdir %BINDIR% )
if not exist %BINDIR%\include ( mkdir %BINDIR%\include )

rem -------------------------------------------------------------------
rem Build CSEIS

echo "Start building CSEIS..."

if exist %BINDIR%\seaseis.exe ( del %BINDIR%\seaseis.exe )
%MAKE% -f %SRCDIR%\make\win\Makefile_build
%MAKE% -f %SRCDIR%\make\win\Makefile_all
%MAKE% -f %SRCDIR%\make\win\Makefile_seaview

copy %JAVADIR%\jar\CSeisLib.jar %BINDIR%
copy %JAVADIR%\jar\SeaView.jar %BINDIR%
copy %JAVADIR%\bin\seaview.bat %BINDIR%
copy %JAVADIR%\bin\plotimage.bat %BINDIR%

echo "End..."

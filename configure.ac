# Copyright (C) 2004-2005 Robin Hugh Johnson <robbat2@orbis-terrarum.net>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AC_PREREQ([2.71])
# version stamp is 0.YYYYMMDD.HHMM in UTC!
# date +"1.%Y%m%d.%H%M" -u
AC_INIT([readahead-list],[1.20060421.1016],[Robin H. Johnson robbat2@orbis-terrarum.net],[readahead-list])

WARN_CFLAGS="-Wall -W"
CFLAGS="$CFLAGS $WARN_CFLAGS"
CXXFLAGS="$CXXFLAGS $WARN_CFLAGS"
export CFLAGS CXXFLAGS

AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([dist-bzip2])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PATH_PROG(MV, mv, mv)
AC_PATH_PROG(RM, rm, rm)
AC_PATH_PROG(CP, cp, cp)
AC_PATH_PROG(SED, sed, sed)
AC_PATH_PROG(CMP, cmp, cmp)

AC_CANONICAL_HOST
AC_PROG_RANLIB

# Provides _GNU_SOURCE for readahead()
AC_USE_SYSTEM_EXTENSIONS

AC_SYS_LARGEFILE

# Checks for header files.
AC_CHECK_HEADERS([ errno.h fcntl.h getopt.h linux/fs.h sched.h stdio.h stdlib.h string.h sys/ioctl.h sys/mman.h sys/syscall.h sys/types.h unistd.h ])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_STRUCT_ST_BLOCKS
AC_CHECK_MEMBERS([struct stat.st_rdev])

AC_CONFIG_FILES([
   Makefile
   README
   doc/Makefile
   m4/Makefile
   src/Makefile
   contrib/Makefile
   contrib/data/Makefile
   contrib/init/Makefile
   contrib/init/gentoo/Makefile
   contrib/scripts/Makefile
])

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MMAP
AC_CHECK_FUNCS([ioctl memchr readahead])

AC_OUTPUT

# Copyright (C) 2004 Robin Hugh Johnson <robbat2@orbis-terrarum.net>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AC_PREREQ(2.59)
AC_INIT([readahead-list],[0.0.1],[Robin H. Johnson robbat2@gentoo.org],[readahead-list])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([dist-bzip2])

LF_CONFIGURE_CC
LF_CONFIGURE_CXX
LF_HOST_TYPE
LF_SET_WARNINGS
AC_PROG_RANLIB

# Checks for header files.
AC_HEADER_STDC
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
])

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MMAP
AC_CHECK_FUNCS([ioctl memchr])

AC_OUTPUT

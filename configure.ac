#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([gboing], [0.1], [daniel.santos@pobox.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([autoscripts])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/lib/gboing.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h stddef.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_CHECK_FUNCS([clock_gettime])

# Output
LT_INIT

AC_ENABLE_SHARED
#AC_DISABLE_STATIC
#AC_PROG_LIBTOOL(libtool)

AC_CONFIG_FILES([
  Makefile
  src/lib/Makefile
  src/test/Makefile
])
AC_OUTPUT

#  doc/Makefile

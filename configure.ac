AC_INIT(aclocal.m4)
AM_INIT_AUTOMAKE([blc], [0.4.0])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AM_PROG_LIBTOOL
AM_CONFIG_HEADER(config.h)
AC_CHECK_PROG(CAT,cat,cat)
AC_CHECK_PROG(TOUCH,touch,touch)
AC_CHECK_PROG(DIFF,diff,diff)
AC_CHECK_PROG(PANDOC,pandoc,pandoc)
AC_CHECK_PROG(POVRAY,povray,povray)
AC_CHECK_PROG(CONVERT,convert,convert)
AC_CHECK_HEADERS([assert.h stdio.h stdlib.h string.h])
AC_FUNC_MALLOC
AC_CHECK_FUNCS([fmemopen strcpy])

dnl Switch for debug or release mode.
AC_ARG_ENABLE(debug,
  [  --enable-debug=DEBUG    compile with debugging assertions (default=no)],
  enable_debug=$enableval, enable_debug=no)
if test "x$enable_debug" == "xyes"; then
  AM_CFLAGS='-Wall -g -O0'
else
  AM_CFLAGS='-Wall -Os -DNDEBUG'
fi
AC_SUBST(AM_CFLAGS)

info="$info

prefix                               : $prefix
debug mode                           : $enable_debug"

AC_OUTPUT(Makefile,
  echo "$info",
  info="$info")

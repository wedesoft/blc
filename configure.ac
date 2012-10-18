AC_INIT(aclocal.m4)
AM_INIT_AUTOMAKE([bracket], [0.2.0])
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_LIBTOOL
AC_PROG_CC
AC_CHECK_PROG(CAT,cat,cat)
AC_CHECK_PROG(TOUCH,touch,touch)
AC_CHECK_PROG(DIFF,diff,diff)

dnl Switch for debug or release mode.
AC_ARG_ENABLE(debug,
  [  --enable-debug=DEBUG    compile with debugging assertions (default=no)],
  enable_debug=$enableval, enable_debug=no)
if test "x$enable_debug" == "xyes"; then
  AM_CFLAGS='-Wall -g'
else
  AM_CFLAGS='-Wall -O -DNDEBUG'
fi
AC_SUBST(AM_CFLAGS)

info="$info

prefix                               : $prefix
debug mode                           : $enable_debug"

AC_OUTPUT(Makefile,
  echo "$info",
  info="$info")


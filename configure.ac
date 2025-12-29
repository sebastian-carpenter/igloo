AC_INIT(igloo, 0.1)

# clear CFLAGS if unset in user environment, otherwise it defaults to "-g -O2"
: ${CFLAGS=""}

AC_PROG_CC

AM_INIT_AUTOMAKE([subdir-objects])

AC_ARG_VAR(EXTRA_CFLAGS, [Extra CFLAGS to be put into the Makefile])
AC_ARG_VAR(EXTRA_LDFLAGS, [Extra LDFLAGS to be put into the Makefile])

AC_CONFIG_FILES([Makefile])

EXTRA_CFLAGS="-Wall -Wextra -O3"

# wolfssl
AC_ARG_ENABLE([wolfssl],
    [  --disable-wolfssl       Disable linking of wolfSSL],
    [disable_wolfssl="yes"],
    [disable_wolfssl="no"]
    )

AC_MSG_RESULT([Disable wolfSSL? $disable_wolfssl])
if test "$disable_wolfssl" = "yes"; then
  AC_DEFINE(DISABLE_WOLFSSL)
else
  EXTRA_LDFLAGS="$EXTRA_LDFLAGS -L/usr/local/lib -lm -lwolfssl"
fi

AM_CFLAGS="$AM_CFLAGS $EXTRA_CFLAGS"
AM_LDFLAGS="$AM_LDFLAGS $EXTRA_LDFLAGS"

AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_OUTPUT

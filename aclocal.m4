dnl $Id: aclocal.m4,v 1.1 1997/05/18 20:22:12 tom Exp $
dnl ---------------------------------------------------------------------------
dnl Special test to workaround gcc 2.6.2, which cannot parse C-preprocessor
dnl conditionals.
AC_DEFUN([CF_POSIX_VDISABLE],
[
AC_MSG_CHECKING(if POSIX VDISABLE symbol should be used)
AC_CACHE_VAL(cf_cv_posix_vdisable,[
	AC_TRY_RUN([
#if HAVE_TERMIOS_H && HAVE_TCGETATTR
#include <termios.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if defined(_POSIX_VDISABLE)
int main() { exit(_POSIX_VDISABLE == -1); }
#endif],
	[cf_cv_posix_vdisable=yes],
	[cf_cv_posix_vdisable=no],
	[AC_TRY_COMPILE([
#if HAVE_TERMIOS_H && HAVE_TCGETATTR
#include <termios.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif],[
#if defined(_POSIX_VDISABLE) && (_POSIX_VDISABLE != -1)
int temp = _POSIX_VDISABLE;
#else
this did not work
#endif],
	[cf_cv_posix_vdisable=yes],
	[cf_cv_posix_vdisable=no],
	)])
])
AC_MSG_RESULT($cf_cv_posix_vdisable)
test $cf_cv_posix_vdisable = yes && AC_DEFINE(HAVE_POSIX_VDISABLE)
])dnl

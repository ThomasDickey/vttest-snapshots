dnl $Id: aclocal.m4,v 1.8 2004/12/21 01:51:14 tom Exp $
dnl autoconf macros for vttest - T.E.Dickey
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl CF_ARG_DISABLE version: 3 updated: 1999/03/30 17:24:31
dnl --------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl CF_ARG_OPTION version: 3 updated: 1997/10/18 14:42:41
dnl -------------
dnl Restricted form of AC_ARG_ENABLE that ensures user doesn't give bogus
dnl values.
dnl
dnl Parameters:
dnl $1 = option name
dnl $2 = help-string
dnl $3 = action to perform if option is not default
dnl $4 = action if perform if option is default
dnl $5 = default option value (either 'yes' or 'no')
AC_DEFUN([CF_ARG_OPTION],
[AC_ARG_ENABLE($1,[$2],[test "$enableval" != ifelse($5,no,yes,no) && enableval=ifelse($5,no,no,yes)
  if test "$enableval" != "$5" ; then
ifelse($3,,[    :]dnl
,[    $3]) ifelse($4,,,[
  else
    $4])
  fi],[enableval=$5 ifelse($4,,,[
  $4
])dnl
  ])])dnl
dnl ---------------------------------------------------------------------------
dnl CF_DISABLE_ECHO version: 10 updated: 2003/04/17 22:27:11
dnl ---------------
dnl You can always use "make -n" to see the actual options, but it's hard to
dnl pick out/analyze warning messages when the compile-line is long.
dnl
dnl Sets:
dnl	ECHO_LT - symbol to control if libtool is verbose
dnl	ECHO_LD - symbol to prefix "cc -o" lines
dnl	RULE_CC - symbol to put before implicit "cc -c" lines (e.g., .c.o)
dnl	SHOW_CC - symbol to put before explicit "cc -c" lines
dnl	ECHO_CC - symbol to put before any "cc" line
dnl
AC_DEFUN([CF_DISABLE_ECHO],[
AC_MSG_CHECKING(if you want to see long compiling messages)
CF_ARG_DISABLE(echo,
	[  --disable-echo          display "compiling" commands],
	[
    ECHO_LT='--silent'
    ECHO_LD='@echo linking [$]@;'
    RULE_CC='	@echo compiling [$]<'
    SHOW_CC='	@echo compiling [$]@'
    ECHO_CC='@'
],[
    ECHO_LT=''
    ECHO_LD=''
    RULE_CC='# compiling'
    SHOW_CC='# compiling'
    ECHO_CC=''
])
AC_MSG_RESULT($enableval)
AC_SUBST(ECHO_LT)
AC_SUBST(ECHO_LD)
AC_SUBST(RULE_CC)
AC_SUBST(SHOW_CC)
AC_SUBST(ECHO_CC)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_FCNTL_VS_IOCTL version: 3 updated: 2004/08/03 20:29:33
dnl -----------------
dnl Test if we have a usable ioctl with FIONREAD, or if fcntl.h is preferred.
AC_DEFUN([CF_FCNTL_VS_IOCTL],
[
AC_MSG_CHECKING(if we may use FIONREAD)
AC_CACHE_VAL(cf_cv_use_fionread,[
	AC_TRY_COMPILE([
#if HAVE_SYS_FILIO_H
#  include <sys/filio.h>	/* FIONREAD */
#endif
	],[
int l1;
ioctl (0, FIONREAD, &l1);
	],
	[cf_cv_use_fionread=yes],
	[cf_cv_use_fionread=no])
])
AC_MSG_RESULT($cf_cv_use_fionread)
test $cf_cv_use_fionread = yes && AC_DEFINE(USE_FIONREAD)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_ATTRIBUTES version: 9 updated: 2002/12/21 19:25:52
dnl -----------------
dnl Test for availability of useful gcc __attribute__ directives to quiet
dnl compiler warnings.  Though useful, not all are supported -- and contrary
dnl to documentation, unrecognized directives cause older compilers to barf.
AC_DEFUN([CF_GCC_ATTRIBUTES],
[
if test "$GCC" = yes
then
cat > conftest.i <<EOF
#ifndef GCC_PRINTF
#define GCC_PRINTF 0
#endif
#ifndef GCC_SCANF
#define GCC_SCANF 0
#endif
#ifndef GCC_NORETURN
#define GCC_NORETURN /* nothing */
#endif
#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif
EOF
if test "$GCC" = yes
then
	AC_CHECKING([for $CC __attribute__ directives])
cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"
#include "conftest.h"
#include "conftest.i"
#if	GCC_PRINTF
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
#define GCC_PRINTFLIKE(fmt,var) /*nothing*/
#endif
#if	GCC_SCANF
#define GCC_SCANFLIKE(fmt,var)  __attribute__((format(scanf,fmt,var)))
#else
#define GCC_SCANFLIKE(fmt,var)  /*nothing*/
#endif
extern void wow(char *,...) GCC_SCANFLIKE(1,2);
extern void oops(char *,...) GCC_PRINTFLIKE(1,2) GCC_NORETURN;
extern void foo(void) GCC_NORETURN;
int main(int argc GCC_UNUSED, char *argv[[]] GCC_UNUSED) { return 0; }
EOF
	for cf_attribute in scanf printf unused noreturn
	do
		CF_UPPER(CF_ATTRIBUTE,$cf_attribute)
		cf_directive="__attribute__(($cf_attribute))"
		echo "checking for $CC $cf_directive" 1>&AC_FD_CC
		case $cf_attribute in
		scanf|printf)
		cat >conftest.h <<EOF
#define GCC_$CF_ATTRIBUTE 1
EOF
			;;
		*)
		cat >conftest.h <<EOF
#define GCC_$CF_ATTRIBUTE $cf_directive
EOF
			;;
		esac
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... $cf_attribute)
			cat conftest.h >>confdefs.h
		fi
	done
else
	fgrep define conftest.i >>confdefs.h
fi
rm -rf conftest*
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_VERSION version: 3 updated: 2003/09/06 19:16:57
dnl --------------
dnl Find version of gcc
AC_DEFUN([CF_GCC_VERSION],[
AC_REQUIRE([AC_PROG_CC])
GCC_VERSION=none
if test "$GCC" = yes ; then
	AC_MSG_CHECKING(version of $CC)
	GCC_VERSION="`${CC} --version|sed -e '2,$d' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GCC_VERSION" && GCC_VERSION=unknown
	AC_MSG_RESULT($GCC_VERSION)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_WARNINGS version: 18 updated: 2004/12/03 20:51:07
dnl ---------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally).  This
dnl		is enabled for ncurses using "--enable-const".
dnl	-pedantic
dnl
dnl Parameter:
dnl	$1 is an optional list of gcc warning flags that a particular
dnl		application might want to use, e.g., "no-unused" for
dnl		-Wno-unused
dnl Special:
dnl	If $with_ext_const is "yes", add a check for -Wwrite-strings
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[
AC_REQUIRE([CF_INTEL_COMPILER])
AC_REQUIRE([CF_GCC_VERSION])
if test "$INTEL_COMPILER" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #269: invalid format string conversion
	EXTRA_CFLAGS="$EXTRA_CFLAGS -Wall \
 -wd1419 \
 -wd193 \
 -wd279 \
 -wd593 \
 -wd810 \
 -wd869 \
 -wd981"
elif test "$GCC" = yes
then
	cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF
	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="-W -Wall"
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	for cf_opt in \
		Wbad-function-cast \
		Wcast-align \
		Wcast-qual \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes \
		Wundef $cf_warn_CONST $1
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			case $cf_opt in #(vi
			Wcast-qual) #(vi
				CPPFLAGS="$CPPFLAGS -DXTSTRINGDEFINES"
				;;
			Winline) #(vi
				case $GCC_VERSION in
				3.3*)
					CF_VERBOSE(feature is broken in gcc $GCC_VERSION)
					continue;;
				esac
				;;
			esac
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	rm -f conftest*
	CFLAGS="$cf_save_CFLAGS"
fi
AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 1 updated: 2004/12/03 20:27:48
dnl -----------------
dnl Check if the given compiler is really the Intel compiler for Linux.
dnl It tries to imitate gcc, but does not return an error when it finds a
dnl mismatch between prototypes, e.g., as exercised by CF_MISSING_CHECK.
dnl
dnl This macro should be run "soon" after AC_PROG_CC, to ensure that it is
dnl not mistaken for gcc.
AC_DEFUN([CF_INTEL_COMPILER],[
AC_REQUIRE([AC_PROG_CC])

INTEL_COMPILER=no

if test "$GCC" = yes ; then
	case $host_os in
	linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel compiler)
		cf_save_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
make an error
#endif
],[INTEL_COMPILER=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147 -no-gcc"
],[])
		CFLAGS="$cf_save_CFLAGS"
		AC_MSG_RESULT($INTEL_COMPILER)
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_VDISABLE version: 4 updated: 2000/01/22 00:19:54
dnl -----------------
dnl Special test to workaround gcc 2.6.2, which cannot parse C-preprocessor
dnl conditionals.
dnl
dnl AC_CHECK_HEADERS(termios.h unistd.h)
dnl AC_CHECK_FUNCS(tcgetattr)
dnl
AC_DEFUN([CF_POSIX_VDISABLE],
[
AC_MSG_CHECKING(if POSIX VDISABLE symbol should be used)
AC_CACHE_VAL(cf_cv_posix_vdisable,[
	AC_TRY_RUN([
#if defined(HAVE_TERMIOS_H) && defined(HAVE_TCGETATTR)
#include <termios.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#if defined(_POSIX_VDISABLE)
int main() { exit(_POSIX_VDISABLE == -1); }
#endif],
	[cf_cv_posix_vdisable=yes],
	[cf_cv_posix_vdisable=no],
	[AC_TRY_COMPILE([
#if defined(HAVE_TERMIOS_H) && defined(HAVE_TCGETATTR)
#include <termios.h>
#endif
#ifdef HAVE_UNISTD_H
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
dnl ---------------------------------------------------------------------------
dnl CF_UPPER version: 5 updated: 2001/01/29 23:40:59
dnl --------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
$1=`echo "$2" | sed y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%`
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_VERBOSE version: 2 updated: 1997/09/05 10:45:14
dnl ----------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_WITH_WARNINGS version: 5 updated: 2004/07/23 14:40:34
dnl ----------------
dnl Combine the checks for gcc features into a configure-script option
dnl
dnl Parameters:
dnl	$1 - see CF_GCC_WARNINGS
AC_DEFUN([CF_WITH_WARNINGS],
[
if ( test "$GCC" = yes || test "$GXX" = yes )
then
AC_MSG_CHECKING(if you want to check for gcc warnings)
AC_ARG_WITH(warnings,
	[  --with-warnings         test: turn on gcc warnings],
	[cf_opt_with_warnings=$withval],
	[cf_opt_with_warnings=no])
AC_MSG_RESULT($cf_opt_with_warnings)
if test "$cf_opt_with_warnings" != no ; then
	CF_GCC_ATTRIBUTES
	CF_GCC_WARNINGS([$1])
fi
fi
])dnl

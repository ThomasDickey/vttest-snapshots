dnl $Id: aclocal.m4,v 1.12 2006/11/26 17:33:09 tom Exp $
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
dnl CF_CHECK_CACHE version: 10 updated: 2004/05/23 13:03:31
dnl --------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname).  Normally we'll use AC_CANONICAL_HOST, but allow
dnl an extra parameter that we may override, e.g., for AC_CANONICAL_SYSTEM
dnl which is useful in cross-compiles.
dnl
dnl Note: we would use $ac_config_sub, but that is one of the places where
dnl autoconf 2.5x broke compatibility with autoconf 2.13
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f $srcdir/config.guess || test -f $ac_aux_dir/config.guess ; then
	ifelse([$1],,[AC_CANONICAL_HOST],[$1])
	system_name="$host_os"
else
	system_name="`(uname -s -r) 2>/dev/null`"
	if test -z "$system_name" ; then
		system_name="`(hostname) 2>/dev/null`"
	fi
fi
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name")
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT(Configuring for $cf_cv_system_name)

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_ERROR("Please remove config.cache and try again.")
fi
])dnl
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
dnl CF_GCC_ATTRIBUTES version: 10 updated: 2005/05/28 13:16:28
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
		CF_UPPER(cf_ATTRIBUTE,$cf_attribute)
		cf_directive="__attribute__(($cf_attribute))"
		echo "checking for $CC $cf_directive" 1>&AC_FD_CC
		case $cf_attribute in
		scanf|printf)
		cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
			;;
		*)
		cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE $cf_directive
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
dnl CF_GCC_VERSION version: 4 updated: 2005/08/27 09:53:42
dnl --------------
dnl Find version of gcc
AC_DEFUN([CF_GCC_VERSION],[
AC_REQUIRE([AC_PROG_CC])
GCC_VERSION=none
if test "$GCC" = yes ; then
	AC_MSG_CHECKING(version of $CC)
	GCC_VERSION="`${CC} --version| sed -e '2,$d' -e 's/^.*(GCC) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
	test -z "$GCC_VERSION" && GCC_VERSION=unknown
	AC_MSG_RESULT($GCC_VERSION)
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GCC_WARNINGS version: 20 updated: 2005/08/06 18:37:29
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
AC_REQUIRE([CF_GCC_VERSION])
CF_INTEL_COMPILER(GCC,INTEL_COMPILER,CFLAGS)

cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[[]]) { return (argv[[argc-1]] == 0) ; }
EOF

if test "$INTEL_COMPILER" = yes
then
# The "-wdXXX" options suppress warnings:
# remark #1419: external declaration in primary source file
# remark #1682: implicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1683: explicit conversion of a 64-bit integral type to a smaller integral type (potential portability problem)
# remark #1684: conversion from pointer to same-sized integral type (potential portability problem)
# remark #193: zero used for undefined preprocessing identifier
# remark #593: variable "curs_sb_left_arrow" was set but never used
# remark #810: conversion from "int" to "Dimension={unsigned short}" may lose significant bits
# remark #869: parameter "tw" was never referenced
# remark #981: operands are evaluated in unspecified order
# warning #269: invalid format string conversion

	AC_CHECKING([for $CC warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="-Wall"
	for cf_opt in $1 \
		wd1419 \
		wd1682 \
		wd1683 \
		wd1684 \
		wd193 \
		wd279 \
		wd593 \
		wd810 \
		wd869 \
		wd981
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
		fi
	done
	CFLAGS="$cf_save_CFLAGS"

elif test "$GCC" = yes
then
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
	CFLAGS="$cf_save_CFLAGS"
fi
rm -f conftest*

AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_GNU_SOURCE version: 6 updated: 2005/07/09 13:23:07
dnl -------------
dnl Check if we must define _GNU_SOURCE to get a reasonable value for
dnl _XOPEN_SOURCE, upon which many POSIX definitions depend.  This is a defect
dnl (or misfeature) of glibc2, which breaks portability of many applications,
dnl since it is interwoven with GNU extensions.
dnl
dnl Well, yes we could work around it...
AC_DEFUN([CF_GNU_SOURCE],
[
AC_CACHE_CHECK(if we must define _GNU_SOURCE,cf_cv_gnu_source,[
AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_gnu_source=no],
	[cf_save="$CPPFLAGS"
	 CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_gnu_source=no],
	[cf_cv_gnu_source=yes])
	CPPFLAGS="$cf_save"
	])
])
test "$cf_cv_gnu_source" = yes && CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_INTEL_COMPILER version: 3 updated: 2005/08/06 18:37:29
dnl -----------------
dnl Check if the given compiler is really the Intel compiler for Linux.  It
dnl tries to imitate gcc, but does not return an error when it finds a mismatch
dnl between prototypes, e.g., as exercised by CF_MISSING_CHECK.
dnl
dnl This macro should be run "soon" after AC_PROG_CC or AC_PROG_CPLUSPLUS, to
dnl ensure that it is not mistaken for gcc/g++.  It is normally invoked from
dnl the wrappers for gcc and g++ warnings.
dnl
dnl $1 = GCC (default) or GXX
dnl $2 = INTEL_COMPILER (default) or INTEL_CPLUSPLUS
dnl $3 = CFLAGS (default) or CXXFLAGS
AC_DEFUN([CF_INTEL_COMPILER],[
ifelse($2,,INTEL_COMPILER,[$2])=no

if test "$ifelse($1,,[$1],GCC)" = yes ; then
	case $host_os in
	linux*|gnu*)
		AC_MSG_CHECKING(if this is really Intel ifelse($1,GXX,C++,C) compiler)
		cf_save_CFLAGS="$ifelse($3,,CFLAGS,[$3])"
		ifelse($3,,CFLAGS,[$3])="$ifelse($3,,CFLAGS,[$3]) -no-gcc"
		AC_TRY_COMPILE([],[
#ifdef __INTEL_COMPILER
#else
make an error
#endif
],[ifelse($2,,INTEL_COMPILER,[$2])=yes
cf_save_CFLAGS="$cf_save_CFLAGS -we147 -no-gcc"
],[])
		ifelse($3,,CFLAGS,[$3])="$cf_save_CFLAGS"
		AC_MSG_RESULT($ifelse($2,,INTEL_COMPILER,[$2]))
		;;
	esac
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MAKE_TAGS version: 2 updated: 2000/10/04 09:18:40
dnl ------------
dnl Generate tags/TAGS targets for makefiles.  Do not generate TAGS if we have
dnl a monocase filesystem.
AC_DEFUN([CF_MAKE_TAGS],[
AC_REQUIRE([CF_MIXEDCASE_FILENAMES])
AC_CHECK_PROG(MAKE_LOWER_TAGS, ctags, yes, no)

if test "$cf_cv_mixedcase" = yes ; then
	AC_CHECK_PROG(MAKE_UPPER_TAGS, etags, yes, no)
else
	MAKE_UPPER_TAGS=no
fi

if test "$MAKE_UPPER_TAGS" = yes ; then
	MAKE_UPPER_TAGS=
else
	MAKE_UPPER_TAGS="#"
fi
AC_SUBST(MAKE_UPPER_TAGS)

if test "$MAKE_LOWER_TAGS" = yes ; then
	MAKE_LOWER_TAGS=
else
	MAKE_LOWER_TAGS="#"
fi
AC_SUBST(MAKE_LOWER_TAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MIXEDCASE_FILENAMES version: 3 updated: 2003/09/20 17:07:55
dnl ----------------------
dnl Check if the file-system supports mixed-case filenames.  If we're able to
dnl create a lowercase name and see it as uppercase, it doesn't support that.
AC_DEFUN([CF_MIXEDCASE_FILENAMES],
[
AC_CACHE_CHECK(if filesystem supports mixed-case filenames,cf_cv_mixedcase,[
if test "$cross_compiling" = yes ; then
	case $target_alias in #(vi
	*-os2-emx*|*-msdosdjgpp*|*-cygwin*|*-mingw32*|*-uwin*) #(vi
		cf_cv_mixedcase=no
		;;
	*)
		cf_cv_mixedcase=yes
		;;
	esac
else
	rm -f conftest CONFTEST
	echo test >conftest
	if test -f CONFTEST ; then
		cf_cv_mixedcase=no
	else
		cf_cv_mixedcase=yes
	fi
	rm -f conftest CONFTEST
fi
])
test "$cf_cv_mixedcase" = yes && AC_DEFINE(MIXEDCASE_FILENAMES)
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_MSG_LOG version: 3 updated: 1997/09/07 14:05:52
dnl ----------
dnl Write a debug message to config.log, along with the line number in the
dnl configure script.
AC_DEFUN([CF_MSG_LOG],[
echo "(line __oline__) testing $* ..." 1>&AC_FD_CC
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_POSIX_C_SOURCE version: 6 updated: 2005/07/14 20:25:10
dnl -----------------
dnl Define _POSIX_C_SOURCE to the given level, and _POSIX_SOURCE if needed.
dnl
dnl	POSIX.1-1990				_POSIX_SOURCE
dnl	POSIX.1-1990 and			_POSIX_SOURCE and
dnl		POSIX.2-1992 C-Language			_POSIX_C_SOURCE=2
dnl		Bindings Option
dnl	POSIX.1b-1993				_POSIX_C_SOURCE=199309L
dnl	POSIX.1c-1996				_POSIX_C_SOURCE=199506L
dnl	X/Open 2000				_POSIX_C_SOURCE=200112L
dnl
dnl Parameters:
dnl	$1 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_POSIX_C_SOURCE],
[
cf_POSIX_C_SOURCE=ifelse($1,,199506L,$1)

cf_save_CFLAGS="$CFLAGS"
cf_save_CPPFLAGS="$CPPFLAGS"

CF_REMOVE_DEFINE(cf_trim_CFLAGS,$cf_save_CFLAGS,_POSIX_C_SOURCE)
CF_REMOVE_DEFINE(cf_trim_CPPFLAGS,$cf_save_CPPFLAGS,_POSIX_C_SOURCE)

AC_CACHE_CHECK(if we should define _POSIX_C_SOURCE,cf_cv_posix_c_source,[
	CF_MSG_LOG(if the symbol is already defined go no further)
	AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
make an error
#endif],
	[cf_cv_posix_c_source=no],
	[cf_want_posix_source=no
	 case .$cf_POSIX_C_SOURCE in #(vi
	 .[[12]]??*) #(vi
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		;;
	 .2) #(vi
		cf_cv_posix_c_source="-D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE"
		cf_want_posix_source=yes
		;;
	 .*)
		cf_want_posix_source=yes
		;;
	 esac
	 if test "$cf_want_posix_source" = yes ; then
		AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _POSIX_SOURCE
make an error
#endif],[],
		cf_cv_posix_c_source="$cf_cv_posix_c_source -D_POSIX_SOURCE")
	 fi
	 CF_MSG_LOG(ifdef from value $cf_POSIX_C_SOURCE)
	 CFLAGS="$cf_trim_CFLAGS"
	 CPPFLAGS="$cf_trim_CPPFLAGS $cf_cv_posix_c_source"
	 CF_MSG_LOG(if the second compile does not leave our definition intact error)
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _POSIX_C_SOURCE
make an error
#endif],,
	 [cf_cv_posix_c_source=no])
	 CFLAGS="$cf_save_CFLAGS"
	 CPPFLAGS="$cf_save_CPPFLAGS"
	])
])

if test "$cf_cv_posix_c_source" != no ; then
	CFLAGS="$cf_trim_CFLAGS"
	CPPFLAGS="$cf_trim_CPPFLAGS"
	if test "$cf_cv_cc_u_d_options" = yes ; then
		cf_temp_posix_c_source=`echo "$cf_cv_posix_c_source" | \
				sed -e 's/-D/-U/g' -e 's/=[[^ 	]]*//g'`
		CPPFLAGS="$CPPFLAGS $cf_temp_posix_c_source"
	fi
	CPPFLAGS="$CPPFLAGS $cf_cv_posix_c_source"
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
dnl CF_PROG_CC_U_D version: 1 updated: 2005/07/14 16:59:30
dnl --------------
dnl Check if C (preprocessor) -U and -D options are processed in the order
dnl given rather than by type of option.  Some compilers insist on apply all
dnl of the -U options after all of the -D options.  Others allow mixing them,
dnl and may predefine symbols that conflict with those we define.
AC_DEFUN([CF_PROG_CC_U_D],
[
AC_CACHE_CHECK(if $CC -U and -D options work together,cf_cv_cc_u_d_options,[
	cf_save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="-UU_D_OPTIONS -DU_D_OPTIONS -DD_U_OPTIONS -UD_U_OPTIONS"
	AC_TRY_COMPILE([],[
#ifndef U_D_OPTIONS
make an undefined-error
#endif
#ifdef  D_U_OPTIONS
make a defined-error
#endif
	],[
	cf_cv_cc_u_d_options=yes],[
	cf_cv_cc_u_d_options=no])
	CPPFLAGS="$cf_save_CPPFLAGS"
])
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_PROG_EXT version: 10 updated: 2004/01/03 19:28:18
dnl -----------
dnl Compute $PROG_EXT, used for non-Unix ports, such as OS/2 EMX.
AC_DEFUN([CF_PROG_EXT],
[
AC_REQUIRE([CF_CHECK_CACHE])
case $cf_cv_system_name in
os2*)
    CFLAGS="$CFLAGS -Zmt"
    CPPFLAGS="$CPPFLAGS -D__ST_MT_ERRNO__"
    CXXFLAGS="$CXXFLAGS -Zmt"
    # autoconf's macro sets -Zexe and suffix both, which conflict:w
    LDFLAGS="$LDFLAGS -Zmt -Zcrtdll"
    ac_cv_exeext=.exe
    ;;
esac

AC_EXEEXT
AC_OBJEXT

PROG_EXT="$EXEEXT"
AC_SUBST(PROG_EXT)
test -n "$PROG_EXT" && AC_DEFINE_UNQUOTED(PROG_EXT,"$PROG_EXT")
])dnl
dnl ---------------------------------------------------------------------------
dnl CF_REMOVE_DEFINE version: 2 updated: 2005/07/09 16:12:18
dnl ----------------
dnl Remove all -U and -D options that refer to the given symbol from a list
dnl of C compiler options.  This works around the problem that not all
dnl compilers process -U and -D options from left-to-right, so a -U option
dnl cannot be used to cancel the effect of a preceding -D option.
dnl
dnl $1 = target (which could be the same as the source variable)
dnl $2 = source (including '$')
dnl $3 = symbol to remove
define([CF_REMOVE_DEFINE],
[
# remove $3 symbol from $2
$1=`echo "$2" | \
	sed	-e 's/-[[UD]]$3\(=[[^ 	]]*\)\?[[ 	]]/ /g' \
		-e 's/-[[UD]]$3\(=[[^ 	]]*\)\?[$]//g'`
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
dnl ---------------------------------------------------------------------------
dnl CF_XOPEN_SOURCE version: 24 updated: 2006/04/02 16:41:09
dnl ---------------
dnl Try to get _XOPEN_SOURCE defined properly that we can use POSIX functions,
dnl or adapt to the vendor's definitions to get equivalent functionality,
dnl without losing the common non-POSIX features.
dnl
dnl Parameters:
dnl	$1 is the nominal value for _XOPEN_SOURCE
dnl	$2 is the nominal value for _POSIX_C_SOURCE
AC_DEFUN([CF_XOPEN_SOURCE],[

AC_REQUIRE([CF_PROG_CC_U_D])

cf_XOPEN_SOURCE=ifelse($1,,500,$1)
cf_POSIX_C_SOURCE=ifelse($2,,199506L,$2)

case $host_os in #(vi
aix[[45]]*) #(vi
	CPPFLAGS="$CPPFLAGS -D_ALL_SOURCE"
	;;
freebsd*) #(vi
	# 5.x headers associate
	#	_XOPEN_SOURCE=600 with _POSIX_C_SOURCE=200112L
	#	_XOPEN_SOURCE=500 with _POSIX_C_SOURCE=199506L
	cf_POSIX_C_SOURCE=200112L
	cf_XOPEN_SOURCE=600
	CPPFLAGS="$CPPFLAGS -D_BSD_TYPES -D__BSD_VISIBLE -D_POSIX_C_SOURCE=$cf_POSIX_C_SOURCE -D_XOPEN_SOURCE=$cf_XOPEN_SOURCE"
	;;
hpux*) #(vi
	CPPFLAGS="$CPPFLAGS -D_HPUX_SOURCE"
	;;
irix[[56]].*) #(vi
	CPPFLAGS="$CPPFLAGS -D_SGI_SOURCE"
	;;
linux*|gnu*) #(vi
	CF_GNU_SOURCE
	;;
mirbsd*) #(vi
	# setting _XOPEN_SOURCE or _POSIX_SOURCE breaks <arpa/inet.h>
	;;
netbsd*) #(vi
	# setting _XOPEN_SOURCE breaks IPv6 for lynx on NetBSD 1.6, breaks xterm, is not needed for ncursesw
	;;
openbsd*) #(vi
	# setting _XOPEN_SOURCE breaks xterm on OpenBSD 2.8, is not needed for ncursesw
	;;
osf[[45]]*) #(vi
	CPPFLAGS="$CPPFLAGS -D_OSF_SOURCE"
	;;
nto-qnx*) #(vi
	CPPFLAGS="$CPPFLAGS -D_QNX_SOURCE"
	;;
sco*) #(vi
	# setting _XOPEN_SOURCE breaks Lynx on SCO Unix / OpenServer
	;;
solaris*) #(vi
	CPPFLAGS="$CPPFLAGS -D__EXTENSIONS__"
	;;
*)
	AC_CACHE_CHECK(if we should define _XOPEN_SOURCE,cf_cv_xopen_source,[
	AC_TRY_COMPILE([#include <sys/types.h>],[
#ifndef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_xopen_source=no],
	[cf_save="$CPPFLAGS"
	 CPPFLAGS="$CPPFLAGS -D_XOPEN_SOURCE=$cf_XOPEN_SOURCE"
	 AC_TRY_COMPILE([#include <sys/types.h>],[
#ifdef _XOPEN_SOURCE
make an error
#endif],
	[cf_cv_xopen_source=no],
	[cf_cv_xopen_source=$cf_XOPEN_SOURCE])
	CPPFLAGS="$cf_save"
	])
])
	if test "$cf_cv_xopen_source" != no ; then
		CF_REMOVE_DEFINE(CFLAGS,$CFLAGS,_XOPEN_SOURCE)
		CF_REMOVE_DEFINE(CPPFLAGS,$CPPFLAGS,_XOPEN_SOURCE)
		test "$cf_cv_cc_u_d_options" = yes && \
			CPPFLAGS="$CPPFLAGS -U_XOPEN_SOURCE"
		CPPFLAGS="$CPPFLAGS -D_XOPEN_SOURCE=$cf_cv_xopen_source"
	fi
	CF_POSIX_C_SOURCE($cf_POSIX_C_SOURCE)
	;;
esac
])

/* $Id: vttest.h,v 1.21 1996/08/19 00:20:04 tom Exp $ */

#ifndef VTTEST_H
#define VTTEST_H 1

#define VERSION "1.7b 1985-04-19"
#include <patchlev.h>

/* Choose one of these */

#define UNIX            /* UNIX                                         */
/* #define VMS          ** VMS not done yet -- send me your version!!!! */

#ifdef HAVE_CONFIG_H
#include <config.h>
#define UNIX
#else

#define RETSIGTYPE void

#endif

#define SIG_ARGS int sig	/* FIXME: configure-test */

#include <stdio.h>

#ifdef UNIX

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#include <ctype.h>

#if HAVE_TERMIOS_H && HAVE_TCGETATTR
#  define USE_POSIX_TERMIOS 1
#else
#  if HAVE_TERMIO_H
#    define USE_TERMIO 1
#  else
#    if HAVE_SGTTY_H
#      define USE_SGTTY 1
#      define USE_FIONREAD 1
#    else
       please fix me
#    endif
#  endif
#endif

#include <signal.h>
#include <setjmp.h>

#if USE_POSIX_TERMIOS
#  include <termios.h>
#  define TTY struct termios
#else
#  if USE_TERMIO
#    include <termio.h>
/*#    define TCSANOW TCSETA */
/*#    define TCSADRAIN TCSETAW */
/*#    define TCSAFLUSH TCSETAF */
#    define TTY struct termio
#    define tcsetattr(fd, cmd, arg) ioctl(fd, cmd, arg)
#    define tcgetattr(fd, arg) ioctl(fd, TCGETA, arg)
#    define VDISABLE (unsigned char)(-1)
#  else
#    if USE_SGTTY
#      include <sgtty.h>
#      define TTY struct sgttyb 
#    endif
#  endif
#endif

#if HAVE_SYS_FILIO_H
#  include <sys/filio.h>	/* FIONREAD */
#endif

#if !defined(sun) || !defined(NL0)
#  if HAVE_IOCTL_H
#   include <ioctl.h>
#  else
#   if HAVE_SYS_IOCTL_H
#    include <sys/ioctl.h>
#   endif
#  endif
#endif

/*FIXME: really want 'extern' for errno*/
#include <errno.h>

extern jmp_buf intrenv;
extern int brkrd;
extern int reading;
extern int max_lines;
extern int max_cols;
extern int min_cols;
extern int input_8bits;
extern int output_8bits;
extern int user_geometry;

#if USE_FCNTL
#include <fcntl.h>
#endif
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(p) /* nothing */
#endif

#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))

/* my SunOS 4.1.x doesn't have prototyped headers */
#if defined(__GNUC__) && defined(sun) && !defined(__SVR4)
extern int fclose(FILE *);
extern int fflush(FILE *);
extern int fprintf(FILE *, const char *fmt, ...);
extern int ioctl(int, unsigned long, void *);
extern int printf(const char *fmt, ...);
extern int scanf(const char *fmt, ...);
#endif

#define MENU_ARGS    char *   the_title
#define PASS_ARGS /* char * */the_title

typedef struct {
  char *description;
  int (*dispatch)(MENU_ARGS);
} MENU;

#define MENU_NOHOLD 0
#define MENU_HOLD   1
#define MENU_MERGE  2

#define TITLE_LINE  3

/* main.c */
extern RETSIGTYPE onbrk(SIG_ARGS);
extern RETSIGTYPE onterm(SIG_ARGS);
extern char *skip_prefix(char *prefix, char *input);
extern int bug_a(MENU_ARGS);
extern int bug_b(MENU_ARGS);
extern int bug_c(MENU_ARGS);
extern int bug_d(MENU_ARGS);
extern int bug_e(MENU_ARGS);
extern int bug_f(MENU_ARGS);
extern int bug_l(MENU_ARGS);
extern int bug_s(MENU_ARGS);
extern int bug_w(MENU_ARGS);
extern int main(int argc, char *argv[]);
extern int menu(MENU *table);
extern int not_impl(MENU_ARGS);
extern int scanto(char *str, int *pos, int toc);
extern int tst_ECH(MENU_ARGS);
extern int tst_SD_DEC(MENU_ARGS);
extern int tst_SU(MENU_ARGS);
extern int tst_bugs(MENU_ARGS);
extern int tst_characters(MENU_ARGS);
extern int tst_colors(MENU_ARGS);
extern int tst_doublesize(MENU_ARGS);
extern int tst_insdel(MENU_ARGS);
extern int tst_keyboard(MENU_ARGS);
extern int tst_movements(MENU_ARGS);
extern int tst_nonvt100(MENU_ARGS);
extern int tst_reports(MENU_ARGS);
extern int tst_rst(MENU_ARGS);
extern int tst_screen(MENU_ARGS);
extern int tst_setup(MENU_ARGS);
extern int tst_vt420(MENU_ARGS);
extern int tst_vt52(MENU_ARGS);
extern int tst_xterm(MENU_ARGS);
extern void bye(void);
extern void chrprint(char *s);
extern void do_scrolling(void);
extern void initterminal(int pn);
extern void title(int offset);

#endif /* VTTEST_H */

/* $Id: ttymodes.c,v 1.7 1996/08/07 01:49:28 tom Exp $ */

#include <vttest.h>
#include <ttymodes.h>
#include <esc.h>	/* inflush() */

static TTY old_modes, new_modes;

#if !USE_POSIX_TERMIOS && !USE_TERMIO && USE_SGTTY
static struct tchars  old_tchars;
static struct ltchars old_ltchars;
#endif

#if USE_POSIX_TERMIOS || USE_TERMIO
static void
disable_control_chars(TTY *modes)
{
# if USE_POSIX_TERMIOS
    int n;
    int temp;
#   if defined(_POSIX_VDISABLE) && (_POSIX_VDISABLE != -1)
      temp = _POSIX_VDISABLE;
#   else
      errno = 0;
      temp = fpathconf(0, _PC_VDISABLE);
      if (temp == -1) {
	if (errno != 0) {
	  restore_ttymodes();
	  fprintf(stderr, "Cannot disable special characters!\n");
	  exit(EXIT_FAILURE);
	}
	temp = 0377;
      }
#     endif
    for (n = 0; n < NCCS; n++)
      modes->c_cc[n] = temp;
# else	/* USE_TERMIO */
#   ifdef	VSWTCH
      modes->c_cc[VSWTCH] = VDISABLE;
#   endif
    modes->c_cc[VSUSP]  = VDISABLE;
#   if defined (VDSUSP) && defined(NCCS) && VDSUSP < NCCS
      modes->c_cc[VDSUSP]  = VDISABLE;
#   endif
    modes->c_cc[VSTART] = VDISABLE;
    modes->c_cc[VSTOP]  = VDISABLE;
# endif
    modes->c_cc[VMIN]  = 1;
    modes->c_cc[VTIME] = 0;
}
#endif

static void
set_ttymodes(TTY *modes)
{
# if USE_POSIX_TERMIOS
    fflush(stdout);
    tcsetattr(0, TCSAFLUSH, modes);
# else
#   if USE_TERMIO
      tcsetattr(0, TCSETAF, modes);
#   else /* USE_SGTTY */
      stty(0, modes);
#   endif
# endif
}

#ifndef log_ttymodes
void log_ttymodes(char *file, int line)
{
  FILE *fp = fopen("ttymodes.log", "a");
  if (fp != 0) {
    fprintf(fp, "%s @%d\n", file, line);
    fclose(fp);
  }
}
#endif

#ifndef dump_ttymodes
void dump_ttymodes(char *tag, int flag)
{
#ifdef UNIX
  FILE *fp = fopen("ttymodes.log", "a");
  TTY tmp_modes;
  if (fp != 0) {
    fprintf(fp, "%s (%d):\n", tag, flag);
# if USE_POSIX_TERMIOS || USE_TERMIO
    tcgetattr(0, &tmp_modes);
    fprintf(fp, " iflag %08o\n", tmp_modes.c_iflag);
    fprintf(fp, " oflag %08o\n", tmp_modes.c_oflag);
    fprintf(fp, " lflag %08o\n", tmp_modes.c_lflag);
    if (!tmp_modes.c_lflag & ICANON) {
      fprintf(fp, " %d:min  =%d\n", VMIN,  tmp_modes.c_cc[VMIN]);
      fprintf(fp, " %d:time =%d\n", VTIME, tmp_modes.c_cc[VTIME]);
    }
# else
    gtty(0, &tmp_modes);
    fprintf(fp, " flags %08o\n", tmp_modes.sg_flags);
# endif
    fclose(fp);
  }
#endif
}
#endif

void init_ttymodes(int pn)
{
  dump_ttymodes("init_ttymode", pn);
#ifdef UNIX
  if (pn==0) {
    fflush(stdout);
# if USE_POSIX_TERMIOS || USE_TERMIO
    tcgetattr(0, &old_modes);
# else
#   if USE_SGTTY
      gtty(0, &old_modes);
      ioctl(0, TIOCGETC, &old_tchars);
      ioctl(0, TIOCGLTC, &old_ltchars);
#   endif
# endif
    new_modes = old_modes;
  } else {
    fflush(stdout);
    inflush();
    sleep(2);
  }
# if USE_POSIX_TERMIOS || USE_TERMIO
    new_modes.c_iflag = BRKINT | old_modes.c_iflag;
# else /* USE_SGTTY */
    new_modes.sg_flags = old_modes.sg_flags | CBREAK;
# endif
  set_ttymodes(&new_modes);
# if USE_FCNTL
  close(2);
  open("/dev/tty", O_RDWR|O_NDELAY);
# endif
#endif /* UNIX */
#ifdef SARG10
  /* Set up neccesary TOPS-10 terminal parameters	*/

  trmop(02041, `VT100`);	/* tty type vt100	*/
  trmop(02002, 0);	/* tty no tape	*/
  trmop(02003, 0);	/* tty lc	*/
  trmop(02005, 1);	/* tty tab	*/
  trmop(02010, 1);	/* tty no crlf	*/
  trmop(02020, 0);	/* tty no tape	*/
  trmop(02021, 1);	/* tty page	*/
  trmop(02025, 0);	/* tty blanks	*/
  trmop(02026, 1);	/* tty no alt	*/
  trmop(02040, 1);	/* tty defer	*/
#endif
#ifdef SARG20
  ttybin(1);	/* set line to binary mode */
#endif
  dump_ttymodes("...init_ttymode", pn);
}

void restore_ttymodes(void)
{
  dump_ttymodes("restore_ttymodes", -1);
#ifdef SARG20
  ttybin(0);	/* reset line to normal mode */
#endif
#ifdef UNIX
  set_ttymodes(&old_modes);
#endif
  dump_ttymodes("...restore_ttymodes", -1);
}

void
set_tty_crmod(int enabled)
{
  dump_ttymodes("set_tty_crmod", enabled);
#ifdef UNIX
# if USE_POSIX_TERMIOS || USE_TERMIO
#   if USE_POSIX_TERMIOS
#     define MASK_CRMOD ((unsigned long) (ICRNL | IXON))
#   else
#     define MASK_CRMOD ((unsigned long) (ICRNL))
#   endif
    if (enabled) {
      new_modes.c_iflag |= MASK_CRMOD;
      new_modes.c_lflag |= ICANON;
      memcpy(new_modes.c_cc, old_modes.c_cc, sizeof(new_modes.c_cc));
    } else {
      new_modes.c_iflag &= ~MASK_CRMOD;
      new_modes.c_lflag &= ~ICANON;
      disable_control_chars(&new_modes);
    }
# else
    if (enabled)
      new_modes.sg_flags |= CRMOD;
    else
      new_modes.sg_flags &= ~CRMOD;
# endif
  set_ttymodes(&new_modes);
#endif
  dump_ttymodes("...set_tty_crmod", enabled);
}

void
set_tty_echo(int enabled)
{
  dump_ttymodes("set_tty_echo", enabled);
#ifdef UNIX
# if USE_POSIX_TERMIOS || USE_TERMIO
    if (enabled)
      new_modes.c_lflag |= ECHO;
    else
      new_modes.c_lflag &= ~ECHO;
# else /* USE_SGTTY */
    if (enabled)
      new_modes.sg_flags |= ECHO;
    else
      new_modes.sg_flags &= ~ECHO;
# endif
  set_ttymodes(&new_modes);
#endif
  dump_ttymodes("...set_tty_echo", enabled);
}

void
set_tty_raw(int enabled)
{
  dump_ttymodes("set_tty_raw", enabled);
  if (enabled) {
#ifdef UNIX
# if USE_POSIX_TERMIOS || USE_TERMIO
    new_modes.c_iflag      = 0;
    new_modes.c_lflag      = 0;
    new_modes.c_cc[VMIN]   = 1;
    new_modes.c_cc[VTIME]  = 0;
    set_ttymodes(&new_modes);
    set_tty_crmod(FALSE);
# else /* USE_SGTTY */
#   if USE_FCNTL
      new_modes.sg_flags &= ~CBREAK;
#   endif
    new_modes.sg_flags |= RAW;
    set_ttymodes(&new_modes);
    {
      struct tchars tmp_tchars;
      struct ltchars tmp_ltchars;
      memset(&tmp_tchars,  -1, sizeof(tmp_tchars));
      memset(&tmp_ltchars, -1, sizeof(tmp_ltchars));
      ioctl(0, TIOCSETC, &tmp_tchars);
      ioctl(0, TIOCSLTC, &tmp_ltchars);
    }
# endif
#endif
#ifdef SARG10
    ttybin(1);
#endif
#ifdef SARG20
    ttybin(1);
    page(0);		/* Turn off all character processing at input */
    superbin(1);	/* Turn off ^C (among others). Keep your fingers crossed!! */
#endif
  } else {
#ifdef UNIX
# if USE_POSIX_TERMIOS || USE_TERMIO
    new_modes = old_modes;	/* FIXME */
# else /* USE_SGTTY */
    new_modes.sg_flags &= ~RAW;
#   if USE_FCNTL
      new_modes.sg_flags |= CBREAK;
#   endif
    ioctl(0, TIOCSETC, &old_tchars);
    ioctl(0, TIOCSLTC, &old_ltchars);
# endif
    set_ttymodes(&new_modes);
#endif
#ifdef SARG10
    ttybin(0);
#endif
#ifdef SARG20
    ttybin(0);
    superbin(0);	/* Puuuh! We made it!? */
    page(1);		/* Back to normal input processing */
    ttybin(1);		/* This must be the mode for DEC20 */
#endif
  }
  dump_ttymodes("...set_tty_raw", enabled);
}

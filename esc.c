/* $Id: esc.c,v 1.25 1996/08/18 22:25:46 tom Exp $ */

#include <stdarg.h>
#include <vttest.h>
#include <esc.h>

/* FIXME: for Solaris 2.5, which is broken */
#define FLUSH fflush(stdout)

/******************************************************************************/

static char csi_7[] = { ESC, '[', 0 };
static unsigned char csi_8[] = { 0x9b, 0 };

char *
csi_input(void)
{
  return input_8bits ? (char *)csi_8 : csi_7;
}

char *
csi_output(void)
{
  return output_8bits ? (char *)csi_8 : csi_7;
}

/******************************************************************************/

static char dcs_7[] = { ESC, 'P', 0 };
static unsigned char dcs_8[] = { 0x90, 0 };

char *
dcs_input(void)
{
  return input_8bits ? (char *)dcs_8 : dcs_7;
}

char *
dcs_output(void)
{
  return output_8bits ? (char *)dcs_8 : dcs_7;
}

/******************************************************************************/

static char st_7[] = { ESC, '\\', 0 };
static unsigned char st_8[] = { 0x9c, 0 };

char *
st_input(void)
{
  return input_8bits ? (char *)st_8 : st_7;
}

char *
st_output(void)
{
  return output_8bits ? (char *)st_8 : st_7;
}

/******************************************************************************/

void
println(char *s)
{
  printf("%s\r\n", s);
}

static void
va_out(va_list ap, char *fmt)
{
  while (*fmt != '\0') {
    if (*fmt == '%') {
      switch(*++fmt) {
      case 'c':
	printf("%c", va_arg(ap, int));
	break;
      case 'd':
	printf("%d", va_arg(ap, int));
	break;
      case 's':
	printf("%s", va_arg(ap, char *));
	break;
      }
    } else {
      putchar(*fmt);
    }
    fmt++;
  }
}

/* CSI xxx */
void
do_csi(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  printf("%s", csi_output());
  va_out(ap, fmt);
  va_end(ap);
  FLUSH;
}

/* DCS xxx ST */
void
do_dcs(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  printf("%s", dcs_output());
  va_out(ap, fmt);
  va_end(ap);
  puts(st_output());
  FLUSH;
}

void
esc(char *s)
{
  printf("%c%s", ESC, s);
}

void
brc(int pn, int c)
{
  printf("%s%d%c", csi_output(), pn, c);
}

void
brc2(int pn1, int pn2, int c)
{
  printf("%s%d;%d%c", csi_output(), pn1, pn2, c);
}

void
brc3(int pn1, int pn2, int pn3, int c)
{
  printf("%s%d;%d;%d%c", csi_output(), pn1, pn2, pn3, c);
}

/******************************************************************************/

void
cbt(int pn) /* Cursor Back Tab */
{
  brc(pn, 'Z');
}

void
cha(int pn) /* Cursor Character Absolute */
{
  brc(pn, 'G');
}

void
cht(int pn) /* Cursor Forward Tabulation */
{
  brc(pn, 'I');
}

void
cnl(int pn) /* Cursor Next Line */
{
  brc(pn,'E');
}

void
cpl(int pn) /* Cursor Previous Line */
{
  brc(pn,'F');
}

void
cub(int pn)  /* Cursor Backward */
{
  brc(pn,'D');
}

void
cud(int pn)  /* Cursor Down */
{
  brc(pn,'B');
}

void
cuf(int pn)  /* Cursor Forward */
{
  brc(pn,'C');
}

void
cup(int pn1, int pn2)  /* Cursor Position */
{
  brc2(pn1, pn2, 'H');
}

void
cuu(int pn)  /* Cursor Up */
{
  brc(pn,'A');
}

void
da(void)  /* Device Attributes */
{
  brc(0,'c');
}

void
decaln(void)  /* Screen Alignment Display */
{
  esc("#8");
}

void
decdhl(int lower)  /* Double Height Line (also double width) */
{
  if (lower) esc("#4");
  else       esc("#3");
}

void
decdwl(void)  /* Double Wide Line */
{
  esc("#6");
}

void
deckpam(void)  /* Keypad Application Mode */
{
  esc("=");
}

void
deckpnm(void)  /* Keypad Numeric Mode */
{
  esc(">");
}

void
decll(char *ps)  /* Load LEDs */
{
  do_csi("%sq", ps);
}

void
decrc(void)  /* Restore Cursor */
{
  esc("8");
}

void
decreqtparm(int pn)  /* Request Terminal Parameters */
{
  brc(pn,'x');
}

void
decrqss(char *pn) /* Request Status-String */
{
  do_dcs("$q%s", pn);
}

void
decsc(void)  /* Save Cursor */
{
  esc("7");
}

void
decsca(int pn1) /* VT200 select character attribute (protect) */
{
  do_csi("%d\"q", pn1);
}

void
decsed(int pn1) /* VT200 selective erase in display */
{
  do_csi("?%dJ", pn1);
}

void
decsel(int pn1) /* VT200 selective erase in line */
{
  do_csi("?%dK", pn1);
}

void
decstbm(int pn1, int pn2)  /* Set Top and Bottom Margins */
{
  if (pn1 || pn2) brc2(pn1, pn2, 'r');
  else            esc("[r");
  /* Good for >24-line terminals */
}

void
decswl(void)  /* Single Width Line */
{
  esc("#5");
}

void
dectst(int pn)  /* Invoke Confidence Test */
{
  brc2(2, pn, 'y');
#ifdef UNIX
  fflush(stdout);
#endif
}

void
dsr(int pn)  /* Device Status Report */
{
  brc(pn, 'n');
}

void
ed(int pn)  /* Erase in Display */
{
  brc(pn, 'J');
}

void
el(int pn)  /* Erase in Line */
{
  brc(pn,'K');
}

void
ech(int pn) /* Erase character(s) */
{
  brc(pn,'X');
}

void
hpa(int pn) /* Character Position Absolute */
{
  brc(pn, '`');
}

void
hts(void)  /* Horizontal Tabulation Set */
{
  esc("H");
}

void
hvp(int pn1, int pn2)  /* Horizontal and Vertical Position */
{
  brc2(pn1, pn2, 'f');
}

void
ind(void)  /* Index */
{
  esc("D");
}

void
nel(void)  /* Next Line */
{
  esc("E");
}

void
ri(void)  /* Reverse Index */
{
  esc("M");
}

void
ris(void) /*  Reset to Initial State */
{
  esc("c");
#ifdef UNIX
  fflush(stdout);
#endif
}

void
rm(char *ps)  /* Reset Mode */
{
  do_csi("%sl", ps);
}

void s8c1t(int flag) /* Tell terminal to respond with 7-bit or 8-bit controls */
{
  if ((input_8bits = flag) != 0)
    esc(" G"); /* select 8-bit controls */
  else
    esc(" F"); /* select 7-bit controls */
  fflush(stdout);
  zleep(300);
}

void
scs(int g, int c)  /* Select character Set */
{
  printf("%c%c%c%c%c%c%c", ESC, g ? ')' : '(', c,
                           ESC, g ? '(' : ')', 'B',
			   g ? 14 : 15);
}

void
sd_dec(int pn)  /* Scroll Down */
{
  brc(pn, 'T');
}

void
sd_iso(int pn)  /* Scroll Down */
{
  brc(pn, '^');
}

void
sgr(char *ps)  /* Select Graphic Rendition */
{
  do_csi("%sm", ps);
}

void
sl(int pn)  /* Scroll Left */
{
  do_csi("%d @", pn);
}

void
sm(char *ps)  /* Set Mode */
{
  do_csi("%sh", ps);
}

void
sr(int pn)  /* Scroll Right */
{
  do_csi("%d A", pn);
}

void
su(int pn)  /* Scroll Up */
{
  brc(pn, 'S');
}

void
tbc(int pn)  /* Tabulation Clear */
{
  brc(pn, 'g');
}

void
dch(int pn) /* Delete character */
{
  brc(pn, 'P');
}

void
ich(int pn) /* Insert character -- not in VT102 */
{
  brc(pn, '@');
}

void
dl(int pn) /* Delete line */
{
  brc(pn, 'M');
}

void
il(int pn) /* Insert line */
{
  brc(pn, 'L');
}

void
vpa(int pn) /* Line Position Absolute */
{
  brc(pn, 'd');
}

void
vt52cup(int l, int c)
{
  printf("%cY%c%c", ESC, l + 31, c + 31);
}

char
inchar(void)
{

  /*
   *   Wait until a character is typed on the terminal
   *   then read it, without waiting for CR.
   */

#ifdef UNIX
  int lval; static int val; char ch;

  fflush(stdout);
  lval = val;
  brkrd = FALSE;
  reading = TRUE;
#if HAVE_ALARM
  alarm(60);	/* timeout after 1 minute, in case user's keyboard is hung */
#endif
  read(0,&ch,1);
#if HAVE_ALARM
  alarm(0);
#endif
  reading = FALSE;
#ifdef DEBUG
  {
    FILE *fp = fopen("ttymodes.log","a");
    if (fp != 0) {
      fprintf(fp, "%d>%#x\n", brkrd, ch);
      fclose(fp);
    }
  }
#endif
  if (brkrd)
    val = 0177;
  else
    val = ch;
  if ((val==0177) && (val==lval))
    kill(getpid(), (int) SIGTERM);
#endif
  return(val);
}

char *instr(void)
{

  /*
   *   Get an unfinished string from the terminal:
   *   wait until a character is typed on the terminal,
   *   then read it, and all other available characters.
   *   Return a pointer to that string.
   */
  int i; long l1;
  static char result[80];

  i = 0;
  result[i++] = inchar();
/* Wait 0.1 seconds (1 second in vanilla UNIX) */
  zleep(100);
#ifdef UNIX
  fflush(stdout);
#if HAVE_RDCHK
  while(rdchk(0)) {
    read(0,result+i,1);
    if (i++ == 78) break;
  }
#else
#if USE_FCNTL
  while(read(2,result+i,1) == 1)
    if (i++ == 78) break;
#else
  while(ioctl(0,FIONREAD,&l1), l1 > 0L) {
    while(l1-- > 0L) {
      read(0,result+i,1);
      if (i++ == 78) goto out1;
    }
  }
out1:
#endif
#endif
#endif
  result[i] = '\0';
  return(result);
}

void
inputline(char *s)
{
  scanf("%s",s);
}

void
inflush(void)
{

  /*
   *   Flush input buffer, make sure no pending input character
   */

  int val;

#ifdef UNIX
#if HAVE_RDCHK
  while(rdchk(0))
    read(0,&val,1);
#else
#if USE_FCNTL
  while(read(2,&val,1) > 0)
    ;
#else
  long l1;
  ioctl (0, FIONREAD, &l1);
  while(l1-- > 0L)
    read(0,&val,1);
#endif
#endif
#endif
}

void
holdit(void)
{
  inflush();
  printf("Push <RETURN>");
  readnl();
}

void
readnl(void)
{
#ifdef UNIX
  char ch;
  fflush(stdout);
  brkrd = FALSE;
  reading = TRUE;
  do { read(0,&ch,1); } while(ch != '\n' && !brkrd);
  if (brkrd)
    kill(getpid(), SIGTERM);
  reading = FALSE;
#endif
}

void
zleep(int t)
{

/*
 *    Sleep and do nothing (don't waste CPU) for t milliseconds
 */

#ifdef UNIX
#if HAVE_USLEEP
  unsigned msecs = t * 1000;
  usleep(msecs);
#else
  unsigned secs = t / 1000;
  if (secs == 0) secs = 1;
  sleep(secs);	/* UNIX can only sleep whole seconds */
#endif
#endif
}

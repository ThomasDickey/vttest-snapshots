/* $Id: esc.c,v 1.12 1996/08/03 19:03:10 tom Exp $ */

#include <vttest.h>
#include <esc.h>

#if defined(SARG10) || defined(SARG20)
static int ttymode;
#endif

void
println(char *s)
{
  printf("%s\n", s);
}

void
esc(char *s)
{
  printf("%c%s", 27, s);
}

void
brcstr(char *ps, int c)
{
  printf("%c[%s%c", 27, ps, c);
}

void
brc(int pn, int c)
{
  printf("%c[%d%c", 27, pn, c);
}

void
brc2(int pn1, int pn2, int c)
{
  printf("%c[%d;%d%c", 27, pn1, pn2, c);
}

void
brc3(int pn1, int pn2, int pn3, int c)
{
  printf("%c[%d;%d;%d%c", 27, pn1, pn2, pn3, c);
}

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
  brcstr(ps, 'q');
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
decsc(void)  /* Save Cursor */
{
  esc("7");
}

void
decsca(int pn1) /* VT200 select character attribute (protect) */
{
  char temp[80];
  sprintf(temp, "%d\"p", pn1);
  esc(temp);
}

void
decsed(int pn1) /* VT200 selective erase in display */
{
  char temp[80];
  sprintf(temp, "?%dJ", pn1);
  esc(temp);
}

void
decsel(int pn1) /* VT200 selective erase in line */
{
  char temp[80];
  sprintf(temp, "?%dK", pn1);
  esc(temp);
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
}

void
rm(char *ps)  /* Reset Mode */
{
  brcstr(ps, 'l');
}

void
scs(int g, int c)  /* Select character Set */
{
  printf("%c%c%c%c%c%c%c", 27, g ? ')' : '(', c,
                           27, g ? '(' : ')', 'B',
			   g ? 14 : 15);
}

void
sgr(char *ps)  /* Select Graphic Rendition */
{
  brcstr(ps, 'm');
}

void
sl(int pn)  /* Scroll Left */
{
  char temp[80];
  sprintf(temp, "%d ", pn);
  brcstr(temp, '@');
}

void
sm(char *ps)  /* Set Mode */
{
  brcstr(ps, 'h');
}

void
sr(int pn)  /* Scroll Right */
{
  char temp[80];
  sprintf(temp, "%d ", pn);
  brcstr(temp, 'A');
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
  printf("%cY%c%c", 27, l + 31, c + 31);
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
#ifdef SARG10
  int val, waittime;

  waittime = 0;
  while(!uuo(051,2,&val)) {		/* TTCALL 2, (INCHRS)	*/
    zleep(100);				/* Wait 0.1 seconds	*/
    if ((waittime += ttymode) > 600)	/* Time-out, in case	*/
      return('\177');			/* of hung in ttybin(1)	*/
  }
#endif
#ifdef SARG20	/* try to fix a time-out function */
  int val, waittime;

  waittime = 0;
  while(jsys(SIBE,2,_PRIIN) == 0) {	/* Is input empty? */
    zleep(100);
    if ((waittime += ttymode) > 600)
      return('\177');
  }
  ejsys(BIN,_PRIIN);
  val = jsac[2];
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
#ifdef SARG10
  int val, crflag;
#endif

  int i; long l1;
  static char result[80];

  i = 0;
  result[i++] = inchar();
/* Wait 0.1 seconds (1 second in vanilla UNIX) */
#ifdef SARG10
  if (trmop(01031,0) < 5) zleep(500); /* wait longer if low speed */
  else                    zleep(100);
#else
  zleep(100);
#endif
#ifdef UNIX
  fflush(stdout);
#ifdef XENIX
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
#ifdef SARG10
  while(uuo(051,2,&val)) {	/* TTCALL 2, (INCHRS)  */
    if (!(val == '\012' && crflag))	/* TOPS-10 adds LF to CR */
      result[i++] = val;
    crflag = val == '\015';
    if (i == 79) break;
    zleep(50);          /* Wait 0.05 seconds */
  }
#endif
#ifdef SARG20
  while(jsys(SIBE,2,_PRIIN) != 0) {	/* read input until buffer is empty */
    ejsys(BIN,_PRIIN);
    result[i++] = jsac[2];
    if (i == 79) break;
    zleep(50);		/* Wait 0.05 seconds */
  }
#endif
  result[i] = '\0';
  return(result);
}

void
ttybin(int bin)
{
#ifdef SARG10
  #define OPEN 050
  #define IO_MOD 0000017
  #define _IOPIM 2
  #define _IOASC 0
  #define _TOPAG 01021
  #define _TOSET 01000

  int v;
  static int arglst[] = {
    _IOPIM,
    `TTY`,
    0    
  };
  arglst[0] = bin ? _IOPIM : _IOASC;
  v = uuo(OPEN, 1, &arglst[0]);
  if (!v) { printf("OPEN failed"); exit(); }
  trmop(_TOPAG + _TOSET, bin ? 0 : 1);
  ttymode = bin;
#endif
#ifdef SARG20
  /*	TTYBIN will set the line in BINARY/ASCII mode
   *	BINARY mode is needed to send control characters
   *	Bit 28 must be 0 (we don't flip it).
   *	Bit 29 is used for the mode change.
   */

  #define _TTASC 0000100
  #define _MOXOF 0000043

  int v;

  ejsys(RFMOD,_CTTRM);
  v = ejsys(SFMOD,_CTTRM, bin ? (~_TTASC & jsac[2]) : (_TTASC | jsac[2]));
  if (v) { printf("SFMOD failed"); exit(); }
  v = ejsys(MTOPR,_CTTRM,_MOXOF,0);
  if (v) { printf("MTOPR failed"); exit(); }
#endif
}

#ifdef SARG20
/*
 *	SUPERBIN turns off/on all input character interrupts
 *	This affects ^C, ^O, ^T
 *	Beware where and how you use it !!!!!!!
 */

superbin(int bin)
{
  int v;

  v = ejsys(STIW,(0//-5), bin ? 0 : -1);
  if (v) { printf("STIW superbinary setting failed"); exit(); }
  ttymode = bin;
}

/*
 *	PAGE affects the ^S/^Q handshake.
 *	Set bit 34 to turn it on. Clear it for off.
 */

page(bin) int bin;
{
  int v;

  #define TT_PGM 0000002

  ejsys(RFMOD,_CTTRM);	/* Get the current terminal status */
  v = ejsys(STPAR,_CTTRM, bin ? (TT_PGM | jsac[2]) : (~TT_PGM & jsac[2]));
  if (v) { printf("STPAR failed"); exit(); }
}
#endif

void
trmop(int fc, int arg)
{
#ifdef SARG10
  int retvalp;
  int arglst[3];

  /* TRMOP is a TOPS-10 monitor call that does things to the terminal. */

  /* Find out TTY nbr (PA1050 barfs if TRMOP get -1 instead of udx)    */
  /* A TRMNO monitor call returns the udx (Universal Device Index)     */

  arglst[0] = fc;		/* function code	*/
  arglst[1] = calli(0115, -1);	/* udx, TRMNO. UUO	*/
  arglst[2] = arg;		/* Optional argument	*/

  if (calli(0116, 3 // &arglst[0], &retvalp))           /* TRMOP. UUO */
  return (retvalp);
  else {
    printf("?Error return in TRMOP.");
    exit();
  }
#endif
}

void
inputline(char *s)
{
  scanf("%s",s);
#ifdef SARG10
  readnl();
#endif
#ifdef SARG20
  readnl();
#endif
}

void
inflush(void)
{

  /*
   *   Flush input buffer, make sure no pending input character
   */

  int val;

#ifdef UNIX
#ifdef XENIX
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
#ifdef SARG10
  while(uuo(051,2,&val))	/* TTCALL 2, (INCHRS)  */
    ;
#endif
#ifdef SARG20
  ejsys(CFIBF,_PRIIN);		/* Clear input buffer */
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
#ifdef SARG10
 while (getchar() != '\n')
   ;
#endif
#ifdef SARG20
 while (getchar() != '\n')
   ;
#endif
}

void
zleep(int t)
{

/*
 *    Sleep and do nothing (don't waste CPU) for t milliseconds
 */

#ifdef SARG10
  calli(072,t);		/* (HIBER) t milliseconds */
#endif
#ifdef SARG20
  ejsys(DISMS,t);	/* DISMISS for t milliseconds */
#endif
#ifdef UNIX
  t = t / 1000;
  if (t <= 0) t = 1;
  sleep((unsigned)t);	/* UNIX can only sleep whole seconds */
#endif
}

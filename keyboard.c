/* $Id: keyboard.c,v 1.8 1996/08/19 00:08:27 tom Exp $ */

#include <vttest.h>
#include <ttymodes.h>
#include <esc.h>

/* Test of:
     - DECLL   (Load LEDs)
     - Keyboard return messages
     - SM RM   (Set/Reset Mode) - Cursor Keys
                                - Auto repeat
     - DECKPAM (Keypad Application Mode)
     - DECKPNM (Keypad Numeric Mode)

The standard VT100 keyboard layout:
 
                                                        UP   DN   LE  RI

ESC   1!   2@   3#   4$   5%   6^   7&   8*   9(   0)   -_   =+   `~  BS

TAB*    qQ   wW   eE   rR   tT   yY   uU   iI   oO   pP   [{   ]}      DEL

**   **   aA   sS   dD   fF   gG   hH   jJ   kK   lL   ;:   ,"   RETN  \|

**   ****   zZ   xX   cC   vV   bB   nN   mM   ,<   .>   /?   ****   LF

             ****************SPACE BAR****************

                                                           PF1 PF2 PF3 PF4

                                                           *7* *8* *9* *-*

                                                           *4* *5* *6* *,*

                                                           *1* *2* *3*

                                                           ***0*** *.* ENT
*/

static struct key {
    char c;
    int  row;
    int  col;
    char *symbol;
} keytab [] = {
    { ESC, 1,  0, "ESC" },
    { '1', 1,  6, "1" },    { '!', 1,  7, "!" },
    { '2', 1, 11, "2" },    { '@', 1, 12, "@" },
    { '3', 1, 16, "3" },    { '#', 1, 17, "#" },
    { '4', 1, 21, "4" },    { '$', 1, 22, "$" },
    { '5', 1, 26, "5" },    { '%', 1, 27, "%" },
    { '6', 1, 31, "6" },    { '^', 1, 32, "^" },
    { '7', 1, 36, "7" },    { '&', 1, 37, "&" },
    { '8', 1, 41, "8" },    { '*', 1, 42, "*" },
    { '9', 1, 46, "9" },    { '(', 1, 47, "(" },
    { '0', 1, 51, "0" },    { ')', 1, 52, ")" },
    { '-', 1, 56, "-" },    { '_', 1, 57, "_" },
    { '=', 1, 61, "=" },    { '+', 1, 62, "+" },
    { '`', 1, 66, "`" },    { '~', 1, 67, "~" },
    {   8, 1, 70, "BS" },
    {   9, 2,  0, " TAB " },
    { 'q', 2,  8, "q" },    { 'Q', 2,  9, "Q" },
    { 'w', 2, 13, "w" },    { 'W', 2, 14, "W" },
    { 'e', 2, 18, "e" },    { 'E', 2, 19, "E" },
    { 'r', 2, 23, "r" },    { 'R', 2, 24, "R" },
    { 't', 2, 28, "t" },    { 'T', 2, 29, "T" },
    { 'y', 2, 33, "y" },    { 'Y', 2, 34, "Y" },
    { 'u', 2, 38, "u" },    { 'U', 2, 39, "U" },
    { 'i', 2, 43, "i" },    { 'I', 2, 44, "I" },
    { 'o', 2, 48, "o" },    { 'O', 2, 49, "O" },
    { 'p', 2, 53, "p" },    { 'P', 2, 54, "P" },
    { '[', 2, 58, "[" },    { '{', 2, 59, "{" },
    { ']', 2, 63, "]" },    { '}', 2, 64, "}" },
    { 127, 2, 71, "DEL" },
    { 'a', 3, 10, "a" },    { 'A', 3, 11, "A" },
    { 's', 3, 15, "s" },    { 'S', 3, 16, "S" },
    { 'd', 3, 20, "d" },    { 'D', 3, 21, "D" },
    { 'f', 3, 25, "f" },    { 'F', 3, 26, "F" },
    { 'g', 3, 30, "g" },    { 'G', 3, 31, "G" },
    { 'h', 3, 35, "h" },    { 'H', 3, 36, "H" },
    { 'j', 3, 40, "j" },    { 'J', 3, 41, "J" },
    { 'k', 3, 45, "k" },    { 'K', 3, 46, "K" },
    { 'l', 3, 50, "l" },    { 'L', 3, 51, "L" },
    { ';', 3, 55, ";" },    { ':', 3, 56, ":" },
    {'\'', 3, 60, "'" },    { '"', 3, 61,"\"" },
    {  13, 3, 65, "RETN"},
    {'\\', 3, 71,"\\" },    { '|', 3, 72, "|" },
    { 'z', 4, 12, "z" },    { 'Z', 4, 13, "Z" },
    { 'x', 4, 17, "x" },    { 'X', 4, 18, "X" },
    { 'c', 4, 22, "c" },    { 'C', 4, 23, "C" },
    { 'v', 4, 27, "v" },    { 'V', 4, 28, "V" },
    { 'b', 4, 32, "b" },    { 'B', 4, 33, "B" },
    { 'n', 4, 37, "n" },    { 'N', 4, 38, "N" },
    { 'm', 4, 42, "m" },    { 'M', 4, 43, "M" },
    { ',', 4, 47, "," },    { '<', 4, 48, "<" },
    { '.', 4, 52, "." },    { '>', 4, 53, ">" },
    { '/', 4, 57, "/" },    { '?', 4, 58, "?" },
    {  10, 4, 69, "LF" },
    { ' ', 5, 13, "                SPACE BAR                "},
    {'\0', 0,  0, ""  }
  };

static struct curkey {
    char *curkeymsg[3];
    int  curkeyrow;
    int  curkeycol;
    char *curkeysymbol;
    char *curkeyname;
} curkeytab [] = {

    /* A Reset, A Set,  VT52  */

    {{"\033[A","\033OA","\033A"}, 0, 56, "UP",  "Up arrow"   },
    {{"\033[B","\033OB","\033B"}, 0, 61, "DN",  "Down arrow" },
    {{"\033[D","\033OD","\033D"}, 0, 66, "LT",  "Left arrow" },
    {{"\033[C","\033OC","\033C"}, 0, 71, "RT",  "Right arrow"},
    {{"",      "",       ""     }, 0,  0, "",    "" }
};

static struct fnkey {
    char *fnkeymsg[4];
    int  fnkeyrow;
    int  fnkeycol;
    char *fnkeysymbol;
    char *fnkeyname;
} fnkeytab [] = {

    /* ANSI-num,ANSI-app,VT52-nu,VT52-ap,  r, c,  symb   name         */

    {{"\033OP","\033OP","\033P","\033P" }, 6, 59, "PF1", "PF1"        },
    {{"\033OQ","\033OQ","\033Q","\033Q" }, 6, 63, "PF2", "PF2"        },
    {{"\033OR","\033OR","\033R","\033R" }, 6, 67, "PF3", "PF3"        },
    {{"\033OS","\033OS","\033S","\033S" }, 6, 71, "PF4", "PF4"        },
    {{"7",     "\033Ow","7",    "\033?w"}, 7, 59, " 7 ", "Numeric 7"  },
    {{"8",     "\033Ox","8",    "\033?x"}, 7, 63, " 8 ", "Numeric 8"  },
    {{"9",     "\033Oy","9",    "\033?y"}, 7, 67, " 9 ", "Numeric 9"  },
    {{"-",     "\033Om","-",    "\033?m"}, 7, 71, " - ", "Minus"      },
    {{"4",     "\033Ot","4",    "\033?t"}, 8, 59, " 4 ", "Numeric 4"  },
    {{"5",     "\033Ou","5",    "\033?u"}, 8, 63, " 5 ", "Numeric 5"  },
    {{"6",     "\033Ov","6",    "\033?v"}, 8, 67, " 6 ", "Numeric 6"  },
    {{",",     "\033Ol",",",    "\033?l"}, 8, 71, " , ", "Comma"      },
    {{"1",     "\033Oq","1",    "\033?q"}, 9, 59, " 1 ", "Numeric 1"  },
    {{"2",     "\033Or","2",    "\033?r"}, 9, 63, " 2 ", "Numeric 2"  },
    {{"3",     "\033Os","3",    "\033?s"}, 9, 67, " 3 ", "Numeric 3"  },
    {{"0",     "\033Op","0",    "\033?p"},10, 59,"   O   ","Numeric 0"},
    {{".",     "\033On",".",    "\033?n"},10, 67, " . ", "Point"      },
    {{"\015",  "\033OM","\015", "\033?M"},10, 71, "ENT", "ENTER"      },
    {{"","","",""},       0,  0, "",    ""           }
  };

struct natkey {
    char natc;
    int  natrow;
    int  natcol;
    char *natsymbol;
};

static void
set_keyboard_layout(struct natkey *table)
{
  int i, j;

  for (j = 0; table[j].natc != '\0'; j++) {
    for (i = 0; keytab[i].c != '\0'; i++) {
      if (keytab[i].row == table[j].natrow &&
	  keytab[i].col == table[j].natcol) {
	keytab[i].c = table[j].natc;
	keytab[i].symbol = table[j].natsymbol;
	break;
      }
    }
  }
}

static int
default_layout(MENU_ARGS)
{
  /* FIXME */
  return MENU_NOHOLD;
}

static int
set_D47_layout(MENU_ARGS)
{
  static struct natkey table[] =
  {
    { '"', 1, 12, "\""},
    { '&', 1, 32, "&" },
    { '/', 1, 37, "/" },
    { '(', 1, 42, "(" },
    { ')', 1, 47, ")" },
    { '=', 1, 52, "=" },
    { '+', 1, 56, "+" },    { '?', 1, 57, "?" },
    { '`', 1, 61, "`" },    { '@', 1, 62, "@" },
    { '<', 1, 66, "<" },    { '>', 1, 67, ">" },
    { '}', 2, 58, "}" },    { ']', 2, 59, "]" },
    { '^', 2, 63, "^" },    { '~', 2, 64, "~" },
    { '|', 3, 55, "|" },    {'\\', 3, 56,"\\" },
    { '{', 3, 60, "{" },    { '[', 3, 61, "[" },
    {'\'', 3, 71, "'" },    { '*', 3, 72, "*" },
    { ',', 4, 47, "," },    { ';', 4, 48, ";" },
    { '.', 4, 52, "." },    { ':', 4, 53, ":" },
    { '-', 4, 57, "-" },    { '_', 4, 58, "_" },
    {'\0', 0,  0, ""  }
  };

  set_keyboard_layout(table);
  return MENU_NOHOLD;
}

static int
set_E47_layout(MENU_ARGS)
{
  static struct natkey table[] =
  {
    { '"', 1, 12, "\""},
    { '&', 1, 32, "&" },
    { '/', 1, 37, "/" },
    { '(', 1, 42, "(" },
    { ')', 1, 47, ")" },
    { '=', 1, 52, "=" },
    { '+', 1, 56, "+" },    { '?', 1, 57, "?" },
    { '`', 1, 61, "`" },    { '@', 1, 62, "@" },
    { '<', 1, 66, "<" },    { '>', 1, 67, ">" },
    { '}', 2, 58, "}" },    { ']', 2, 59, "]" },
    { '~', 2, 63, "~" },    { '^', 2, 64, "^" },
    { '|', 3, 55, "|" },    {'\\', 3, 56,"\\" },
    { '{', 3, 60, "{" },    { '[', 3, 61, "[" },
    {'\'', 3, 71, "'" },    { '*', 3, 72, "*" },
    { ',', 4, 47, "," },    { ';', 4, 48, ";" },
    { '.', 4, 52, "." },    { ':', 4, 53, ":" },
    { '-', 4, 57, "-" },    { '_', 4, 58, "_" },
    {'\0', 0,  0, ""  }
  };

  set_keyboard_layout(table);
  return MENU_NOHOLD;
}

static void
show_cursor_keys(int flag)
{
  int i;

  for (i = 0; curkeytab[i].curkeysymbol[0] != '\0'; i++) {
    cup(1 + 2 * curkeytab[i].curkeyrow, 1 + curkeytab[i].curkeycol);
    if (flag) sgr("7");
    printf("%s", curkeytab[i].curkeysymbol);
    if (flag) sgr("");
  }
}

static void
show_keypad(int flag)
{
  int i;

  for (i = 0; fnkeytab[i].fnkeysymbol[0] != '\0'; i++) {
    cup(1 + 2 * fnkeytab[i].fnkeyrow, 1 + fnkeytab[i].fnkeycol);
    if (flag) sgr("7");
    printf("%s", fnkeytab[i].fnkeysymbol);
    if (flag) sgr("");
  }
}

static void
show_keyboard(int flag)
{
  int i;

  for (i = 0; keytab[i].c != '\0'; i++) {
    cup(1 + 2 * keytab[i].row, 1 + keytab[i].col);
    if (flag) sgr("7");
    printf("%s", keytab[i].symbol);
    if (flag) sgr("");
  }
}

/******************************************************************************/

static int
tst_AnswerBack(MENU_ARGS)
{
  char *abmstr;

  set_tty_crmod(TRUE);
  ed(2);
  cup(5,1);
  println("Finally, a check of the ANSWERBACK MESSAGE, which can be sent");
  println("by pressing CTRL-BREAK. The answerback message can be loaded");
  println("in SET-UP B by pressing SHIFT-A and typing e.g.");
  println("");
  println("         \" H e l l o , w o r l d Return \"");
  println("");
  println("(the double-quote characters included).  Do that, and then try");
  println("to send an answerback message with CTRL-BREAK.  If it works,");
  println("the answerback message should be displayed in reverse mode.");
  println("Finish with a single RETURN.");

  set_tty_crmod(FALSE);
  do {
    cup(17,1);
    inflush();
    abmstr = instr();
    cup(17,1);
    el(0);
    chrprint(abmstr);
  } while (strcmp(abmstr,"\r"));
  return MENU_NOHOLD;
}

static int
tst_AutoRepeat(MENU_ARGS)
{
  char arptstring[500];

  ed(2);
  cup(10,1);
  println("Test of the AUTO REPEAT feature");

  println("");
  println("Hold down an alphanumeric key for a while, then push RETURN.");
  printf("%s", "Auto Repeat OFF: ");
  rm("?8"); /* DECARM */
  inputline(arptstring);
  if (strlen(arptstring) == 0)      println("No characters read!??");
  else if (strlen(arptstring) == 1) println("OK.");
  else                              println("Too many characters read.");
  println("");

  println("Hold down an alphanumeric key for a while, then push RETURN.");
  printf("%s", "Auto Repeat ON: ");
  sm("?8"); /* DECARM */
  inputline(arptstring);
  if (strlen(arptstring) == 0)      println("No characters read!??");
  else if (strlen(arptstring) == 1) println("Not enough characters read.");
  else                              println("OK.");
  println("");

  return MENU_HOLD;
}

static int
tst_ControlKeys(MENU_ARGS)
{
  int  i, okflag;
  int  kbdc;
  char temp[80];
  char *kbds = strcpy(temp, " ");

  static struct ckey {
      int  ccount;
      char *csymbol;
  } ckeytab [] = {
      { 0, "NUL (CTRL-@ or CTRL-Space)" },
      { 0, "SOH (CTRL-A)" },
      { 0, "STX (CTRL-B)" },
      { 0, "ETX (CTRL-C)" },
      { 0, "EOT (CTRL-D)" },
      { 0, "ENQ (CTRL-E)" },
      { 0, "ACK (CTRL-F)" },
      { 0, "BEL (CTRL-G)" },
      { 0, "BS  (CTRL-H) (BACK SPACE)" },
      { 0, "HT  (CTRL-I) (TAB)" },
      { 0, "LF  (CTRL-J) (LINE FEED)" },
      { 0, "VT  (CTRL-K)" },
      { 0, "FF  (CTRL-L)" },
      { 0, "CR  (CTRL-M) (RETURN)" },
      { 0, "SO  (CTRL-N)" },
      { 0, "SI  (CTRL-O)" },
      { 0, "DLE (CTRL-P)" },
      { 0, "DC1 (CTRL-Q) (X-On)" },
      { 0, "DC2 (CTRL-R)" },
      { 0, "DC3 (CTRL-S) (X-Off)" },
      { 0, "DC4 (CTRL-T)" },
      { 0, "NAK (CTRL-U)" },
      { 0, "SYN (CTRL-V)" },
      { 0, "ETB (CTRL-W)" },
      { 0, "CAN (CTRL-X)" },
      { 0, "EM  (CTRL-Y)" },
      { 0, "SUB (CTRL-Z)" },
      { 0, "ESC (CTRL-[) (ESCAPE)" },
      { 0, "FS  (CTRL-\\ or CTRL-? or CTRL-_)" },
      { 0, "GS  (CTRL-])" },
      { 0, "RS  (CTRL-^ or CTRL-~ or CTRL-`)" },
      { 0, "US  (CTRL-_ or CTRL-?)" }
  };

  ed(2);
  for (i = 0; i < 32; i++) {
    cup(1 + (i % 16), 1 + 40 * (i / 16));
    sgr("7");
    printf("%s", ckeytab[i].csymbol);
    sgr("0");
  }
  cup(19,1);
  set_tty_crmod(TRUE);
  println(
  "Push each CTRL-key TWICE. Note that you should be able to send *all*");
  println(
  "CTRL-codes twice, including CTRL-S (X-Off) and CTRL-Q (X-Off)!");
  println(
  "Finish with DEL (also called DELETE or RUB OUT), or wait 1 minute.");
  set_tty_raw(TRUE);
  do {
    cup(max_lines-1,1); kbdc = inchar();
    cup(max_lines-1,1); el(0);
    if (kbdc < 32) printf("  %s", ckeytab[kbdc].csymbol);
    else {
      sprintf(kbds, "%c", kbdc);
      chrprint(kbds);
      printf("%s", " -- not a CTRL key");
    }
    if (kbdc < 32) ckeytab[kbdc].ccount++;
    if (ckeytab[kbdc].ccount == 2) {
      cup(1 + (kbdc % 16), 1 + 40 * (kbdc / 16));
      printf("%s", ckeytab[kbdc].csymbol);
    }
  } while (kbdc != '\177');

  restore_ttymodes();
  cup(max_lines,1);
  okflag = 1;
  for (i = 0; i < 32; i++) if (ckeytab[i].ccount < 2) okflag = 0;
  if (okflag) printf("%s", "OK. ");
  else        printf("%s", "You have not been able to send all CTRL keys! ");
  return MENU_HOLD;
}

static int
tst_CursorKeys(MENU_ARGS)
{
  int  i;
  int  ckeymode;
  char *curkeystr;

  static char *curkeymodes[3] = {
      "ANSI / Cursor key mode RESET",
      "ANSI / Cursor key mode SET",
      "VT52 Mode"
  };

  ed(2);
  show_keyboard(0);
  show_keypad(0);
  cup(max_lines-2,1);

  set_tty_crmod(FALSE);
  set_tty_echo(FALSE);

  for (ckeymode = 0; ckeymode <= 2; ckeymode++) {
    if (ckeymode) sm("?1"); /* DECCKM */
    else          rm("?1");

    show_cursor_keys(1);
    cup(20,1); printf("<%s>%20s", curkeymodes[ckeymode], "");
    cup(max_lines-2,1); el(0);
    cup(max_lines-2,1); printf("%s", "Press each cursor key. Finish with TAB.");
    for(;;) {
      cup(max_lines-1,1);
      if (ckeymode == 2) rm("?2"); /* VT52 mode */
      curkeystr = instr();
      esc("<");                      /* ANSI mode */
      cup(max_lines-1,1); el(0);
      cup(max_lines-1,1); chrprint(curkeystr);
      if (!strcmp(curkeystr,"\t")) break;
      for (i = 0; curkeytab[i].curkeysymbol[0] != '\0'; i++) {
	if (!strcmp(curkeystr,curkeytab[i].curkeymsg[ckeymode])) {
	  sgr("7");
	  printf(" (%s key) ", curkeytab[i].curkeyname);
	  sgr("");
	  cup(1 + 2 * curkeytab[i].curkeyrow,
	      1 + curkeytab[i].curkeycol);
	  printf("%s", curkeytab[i].curkeysymbol);
	  break;
	}
      }
      if (i == sizeof(curkeytab) / sizeof(struct curkey) - 1) {
	sgr("7");
	printf("%s", " (Unknown cursor key) ");
	sgr("");
      }
    }
  }

  cup(max_lines-1,1); el(0);
  restore_ttymodes();
  return MENU_MERGE;
}

static int
tst_FunctionKeys(MENU_ARGS)
{
  int  i;
  int  fkeymode;
  char *fnkeystr;

  static char *fnkeymodes[4] = {
      "ANSI Numeric mode",
      "ANSI Application mode",
      "VT52 Numeric mode",
      "VT52 Application mode"
  };

  ed(2);
  show_keyboard(0);
  show_cursor_keys(0);
  cup(max_lines-2,1);

  set_tty_crmod(FALSE);
  set_tty_echo(FALSE);

  for (fkeymode = 0; fkeymode <= 3; fkeymode++) {
    show_keypad(1);
    cup(20,1); printf("<%s>%20s", fnkeymodes[fkeymode], "");
    cup(max_lines-2,1); el(0);
    cup(max_lines-2,1); printf("%s", "Press each function key. Finish with TAB.");
    for(;;) {
      cup(max_lines-1,1);
      if (fkeymode >= 2)  rm("?2");    /* VT52 mode */
      if (fkeymode % 2)   deckpam();   /* Application mode */
      else                deckpnm();   /* Numeric mode     */
      fnkeystr = instr();
      esc("<");				/* ANSI mode */
      cup(max_lines-1,1); el(0);
      cup(max_lines-1,1); chrprint(fnkeystr);
      if (!strcmp(fnkeystr,"\t")) break;
      for (i = 0; fnkeytab[i].fnkeysymbol[0] != '\0'; i++) {
	if (!strcmp(fnkeystr,fnkeytab[i].fnkeymsg[fkeymode])) {
	  sgr("7");
	  printf(" (%s key) ", fnkeytab[i].fnkeyname);
	  sgr("");
	  cup(1 + 2 * fnkeytab[i].fnkeyrow, 1 + fnkeytab[i].fnkeycol);
	  printf("%s", fnkeytab[i].fnkeysymbol);
	  break;
	}
      }
      if (i == sizeof(fnkeytab) / sizeof(struct fnkey) - 1) {
	sgr("7");
	printf("%s", " (Unknown function key) ");
	sgr("");
      }
    }
  }

  cup(max_lines-1,1); el(0);
  restore_ttymodes();
  return MENU_MERGE;
}

static int
tst_KeyBoardLayout(MENU_ARGS)
{
  int  i;
  int  kbdc;
  char temp[80];
  char *kbds = strcpy(temp, " ");

  static MENU keyboardmenu[] = {
      { "Standard American ASCII layout",                    default_layout },
      { "Swedish national layout D47",                       set_D47_layout },
      { "Swedish national layout E47",                       set_E47_layout },
        /* add new keyboard layouts here */
      { "",                                                  0 }
    };

  ed(2);
  title(0); println("Choose keyboard layout:");
  (void) menu(keyboardmenu);

  ed(2);
  show_keyboard(1);
  show_cursor_keys(0);
  show_keypad(0);
  cup(max_lines-2,1);

  set_tty_crmod(FALSE);
  set_tty_echo(FALSE);

  inflush();
  printf("Press each key, both shifted and unshifted. Finish with RETURN:");
  do { /* while (kbdc != 13) */
    cup(max_lines-1,1); kbdc = inchar();
    cup(max_lines-1,1); el(0);
    sprintf(kbds, "%c", kbdc);
    chrprint(kbds);
    for (i = 0; keytab[i].c != '\0'; i++) {
      if (keytab[i].c == kbdc) {
        cup(1 + 2 * keytab[i].row, 1 + keytab[i].col);
	printf("%s", keytab[i].symbol);
	break;
      }
    }
  } while (kbdc != 13);

  cup(max_lines-1,1); el(0);
  restore_ttymodes();
  return MENU_MERGE;
}

static int
tst_LED_Lights(MENU_ARGS)
{
  int  i;
  char *ledmsg[6], *ledseq[6];

  ledmsg[0] = "L1 L2 L3 L4"; ledseq[0] = "1;2;3;4";
  ledmsg[1] = "   L2 L3 L4"; ledseq[1] = "1;0;4;3;2";
  ledmsg[2] = "   L2 L3";    ledseq[2] = "1;4;;2;3";
  ledmsg[3] = "L1 L2";       ledseq[3] = ";;2;1";
  ledmsg[4] = "L1";          ledseq[4] = "1";
  ledmsg[5] = "";            ledseq[5] = "";

#ifdef UNIX
  fflush(stdout);
#endif
  ed(2);
  cup(10,1);
  println("These LEDs (\"lamps\") on the keyboard should be on:");
  for (i = 0; i <= 5; i++) {
    cup(10,52); el(0); printf("%s", ledmsg[i]);
    decll("0");
    decll(ledseq[i]);
    cup(12,1); holdit();
  }
  return MENU_NOHOLD;
}

/******************************************************************************/
int
tst_keyboard(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "LED Lights",                                        tst_LED_Lights },
      { "Auto Repeat",                                       tst_AutoRepeat },
      { "KeyBoard Layout",                                   tst_KeyBoardLayout },
      { "Cursor Keys",                                       tst_CursorKeys },
      { "Function Keys",                                     tst_FunctionKeys },
      { "AnswerBack",                                        tst_AnswerBack },
      { "Control Keys",                                      tst_ControlKeys },
      { "", 0 }
    };

  do {
    ed(2);
    title(0); printf("Keyboard Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

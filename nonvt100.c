/* $Id: nonvt100.c,v 1.6 1996/08/07 10:51:34 tom Exp $ */

/*
 * The list of non-VT320 codes was compiled using the list of non-VT320 codes
 * described in the Kermit 3.13 documentation, combined with the ISO-6429
 * (ECMA-48) spec.
 */
#include <vttest.h>
#include <color.h>
#include <xterm.h>
#include <ttymodes.h>
#include <nonvt100.h>
#include <esc.h>

static void report_ok(char *ref, char *tst)
{
  printf(" (%s)", !strcmp(ref, tst) ? "success" : "fail");
}

/* Kermit 3.13 and dtterm implement this; it's probably for VT320 */
static void tst_C8C1T(void)
{
  static char *table[] = {
    "\2331;1R",
    "\033[1;1R",
  };
  char *report;

  ed(2);
  cup(5,1);
  println("This tests the VT5xx control sequence to direct the terminal to emit 8-bit");
  println("control-sequences instead of <esc> sequences.");

  set_tty_raw(TRUE);
  c8c1t(1); fflush(stdout);
  cup(1,1); dsr(6);
  report = instr();
  cup(10,1);
  printf("8-bit controls enabled: ");
  chrprint(report);
  report_ok(table[0], report);

  c8c1t(0); fflush(stdout);
  cup(1,1); dsr(6);
  report = instr();
  cup(12,1);
  printf("8-bit controls disabled: ");
  chrprint(report);
  report_ok(table[1], report);

  set_tty_raw(FALSE);
  set_tty_crmod(TRUE);
  cup(max_lines-1,1);
  holdit();
}

static void tst_CBT(void)
{
  int n;
  int last = (min_cols + 7) / 8;

  ed(2);
  for (n = 1; n <= last; n++) {
    cup(1,min_cols);
    cbt(n);
    printf("%d", last + 1 - n);
  }
  cup(max_lines-3,1);
  ed(0);
  println("If your terminal supports CBT, the tab-stops are numbered starting at 1.");
  holdit();
}

/* Note: CHA and HPA have identical descriptions in ECMA-48 */
/* dtterm implements this (does VT320?) */
static void tst_CHA(void)
{
  int n;

  ed(2);
  for (n = 1; n < max_lines-3; n++) {
    cup(n, min_cols - n);
    cha(n);
    printf("+");
  }
  cup(max_lines-3, 1);
  for (n = 1; n <= min_cols; n++)
    printf("%c", n == max_lines-3 ? '+' : '*');
  cup(max_lines-2, 1);
  println("There should be a diagonal of +'s down to the row of *'s above this message");
  holdit();
}

/* Kermit's documentation refers to this as CHI, ECMA-48 as CHT */
static void tst_CHT(void)
{
  int n;
  int last = (min_cols * 2 + 7) / 8;

  ed(2);
  cup(1,1);
  println("CHT with param == 1:");
  for (n = 0; n < last; n++) {
    cht(1);
    printf("*");
  }

  cup(4,1);
  println("CHT with param != 1:");
  for (n = 0; n < last; n++) {
    cup(5,1);
    cht(n);
    printf("+");
  }

  cup(7,1);
  println("Normal tabs:");
  for (n = 0; n < last; n++) {
    printf("\t*");
  }

  cup(max_lines-3, 1);
  println("If your terminal supports CHT, the lines with *'s above will look the same.");
  println("The lines are designed to wrap-around once.");
  holdit();
}

static void tst_CNL(void)
{
  int n;

  ed(2);
  cup(1, 1);
  printf("1.");
  for (n = 1; n <= max_lines - 3; n++) {
    cup(1, min_cols);
    cnl(n-1);
    printf("%d.", n);
  }
  cup(max_lines-2, 1);
  ed(0);
  println("The lines above this should be numbered in sequence, from 1.");
  holdit();

#if 0
  decstbm(1, max_lines-3);
  cup(max_lines-3, 1);
  cnl(5);
  decstbm(0, 0); /* No scroll region */

  cup(max_lines-2, 1);
  ed(0);
  println("The numbered lines (if any) may be shifted up by 5.");
  holdit();
#endif
}

/*
 * There's a comment in the MS-DOS Kermit 3.13 documentation that implies CPL
 * is used to replace RI (reverse-index).  ECMA-48 doesn't specify scrolling
 * regions, DEC terminals do apparently, so for CPL and CNL we'll test this.
 */
static void tst_CPL(void)
{
  int i;

  ed(2);
  cup(max_lines, 1);
  for (i = max_lines-1; i > 0; i--) {
    cpl(1);
    printf("%d.", i);
  }
  cup(max_lines-2, 1);
  ed(0);
  println("If your terminal supports CPL, the lines above this are numbered.");
  holdit();

#if 0
  decstbm(1, max_lines-3);
  cup(1, 1);
  cpl(5);
  decstbm(0, 0); /* No scroll region */

  cup(max_lines-2, 1);
  ed(0);
  println("The numbered lines (if any) may be shifted down by 5.");
  holdit();
#endif
}

/*
 * Test DEC's selective-erase (set-protected area)
 */
static void tst_DECSCA(void)
{
  int i, j, pass;

  ed(2);

  for (pass = 0; pass < 2; pass++) {
    if (pass == 0)
      decsca(1);
    cup(5, 20);
    for (i = 5; i <= max_lines - 6; i++) {
      cup(i, 20);
      for (j = 20; j < min_cols - 20; j++) {
        printf("*");
      }
    }
    if (pass == 0) {
      decsca(0);

      cup(max_lines/2, min_cols/2);
      decsed(0); /* after the cursor */
      decsed(1); /* before the cursor */
      decsed(2); /* the whole display */

      decsel(0); /* after the cursor */
      decsel(1); /* before the cursor */
      decsel(2); /* the whole line */

      cup(max_lines-3, 1);
      println("If your terminal supports DEC protected areas (DECSCA, DECSED, DECSEL),");
      println("there will be an solid box made of *'s in the middle of the screen.");
      holdit();
    }
  }
}

/*
 * Test if the terminal can make the cursor invisible
 */
static void tst_DECTCEM(void)
{
  ed(2);
  rm("?25");
  println("The cursor should be invisible");
  holdit();
  sm("?25");
  println("The cursor should be visible again");
  holdit();
}

/*
 * VT200 and higher
 *
 * Test to ensure that 'ech' (erase character) is honored, with no parameter,
 * explicit parameter, and longer than the screen width (to ensure that the
 * terminal doesn't try to wrap-around the erasure).
 */
static void tst_ECH(void)
{
  int i;

  ed(2);
  decaln();
  for (i = 1; i <= max_lines; i++) {
    cup(i, min_cols - i - 2);
    brcstr("", 'X'); /* make sure default-parameter works */
    cup(i, min_cols - i - 1);
    printf("*");
    ech(min_cols);
    printf("*"); /* this should be adjacent, in the upper-right corner */
  }

  cup(max_lines-4, 1);
  ed(0);

  cup(max_lines-4, min_cols - (max_lines + 6));
  println("diagonal: ^^ (clear)");
  println("ECH test: there should be E's with a gap before diagonal of **'s");
  println("The lower-right diagonal region should be cleared.  Nothing else.");
  holdit();
}

static void tst_HPA(void)
{
  int n;

  ed(2);
  for (n = 1; n < max_lines-3; n++) {
    cup(n, min_cols - n);
    hpa(n);
    printf("+");
  }
  cup(max_lines-3, 1);
  for (n = 1; n <= min_cols; n++)
    printf("%c", n == max_lines-3 ? '+' : '*');
  cup(max_lines-2, 1);
  println("There should be a diagonal of +'s down to the row of *'s above this message");
  holdit();
}

/*
 * Test the SD (scroll-down) by forcing characters written in a diagonal into
 * a horizontal row.
 *
 * dtterm uses the incorrect escape sequence (ending with 'T' instead of '^'),
 * apparently someone misread 05/14 as 05/04.
 */
static void tst_SD(void)
{
  int n;
  int last = max_lines - 3;

  ed(2);
  for (n = 1; n <= last; n++) {
    cup(n, n);
    printf("*");
    sd(1); 
  }
  cup(last+1,1);
  ed(0);
  println("If your terminal supports SD, there is a horizontal row of *'s above.");
  holdit();
}

/*
 * Test the SL (scroll-left) by forcing characters written in a diagonal into
 * a vertical line.
 */
static void tst_SL(void)
{
  int n;
  int last = max_lines - 3;

  ed(2);
  for (n = 1; n < last; n++) {
    cup(n, min_cols/2 + last - n);
    printf("*");
    sl(1); 
  }
  cup(last,1);
  ed(0);
  println("If your terminal supports SL, there is a vertical column of *'s centered above.");
  holdit();
}

/*
 * Test the SR (scroll-right) by forcing characters written in a diagonal into
 * a vertical line.
 */
static void tst_SR(void)
{
  int n;
  int last = max_lines - 3;

  ed(2);
  for (n = 1; n < last; n++) {
    cup(n, min_cols/2 - last + n);
    printf("*");
    sr(1); 
  }
  cup(last,1);
  ed(0);
  println("If your terminal supports SR, there is a vertical column of *'s centered above.");
  holdit();
}

/*
 * Test the SU (scroll-up) by forcing characters written in a diagonal into
 * a horizontal row.
 */
static void tst_SU(void)
{
  int n;
  int last = max_lines - 3;

  ed(2);
  for (n = 1; n < last; n++) {
    cup(last + 1 - n, n);
    printf("*");
    su(1); 
  }
  cup(last+1,1);
  ed(0);
  println("If your terminal supports SU, there is a horizontal row of *'s above.");
  holdit();
}

/*
 * Test SPA (set-protected area)
 */
static void tst_SPA(void)
{
  int i, j, pass;

  ed(2);

  for (pass = 0; pass < 2; pass++) {
    if (pass == 0) {
      esc("V"); /* SPA */
    }
    /* make two passes so we can paint over the protected-chars in the second */
    cup(5, 20);
    for (i = 5; i <= max_lines - 6; i++) {
      cup(i, 20);
      for (j = 20; j < min_cols - 20; j++) {
        printf("*");
      }
    }
    if (pass == 0) {
      esc("W"); /* EPA */

      cup(max_lines/2, min_cols/2);
      ed(0); /* after the cursor */
      ed(1); /* before the cursor */
      ed(2); /* the whole display */

      el(0); /* after the cursor */
      el(1); /* before the cursor */
      el(2); /* the whole line */

      ech(min_cols);

      cup(max_lines-3, 1);
      println("If your terminal supports protected areas, there will be an solid box made of");
      println("*'s in the middle of the screen.");
      holdit();
    }
  }
}

/*
 * Kermit's documentation refers to this as CVA, ECMA-48 as VPA.
 * Move the cursor in the current column to the specified line.
 */
static void tst_VPA(void)
{
  int n;

  ed(2);
  cup(5, 20);
  for (n = 20; n <= min_cols - 20; n++)
    printf("*");
  for (n = 5; n < max_lines - 6; n++) {
    vpa(n);
    printf("*\b");
  }
  for (n = min_cols - 20; n >= 20; n--)
    printf("\b*\b");
  for (n = 5; n < max_lines - 6; n++) {
    vpa(n);
    printf("*\b");
  }

  cup(max_lines-3, 1);
  println("There should be a box-outline made of *'s in the middle of the screen.");
  holdit();
}

/******************************************************************************/

static
void tst_vt220(void)
{
  int menuchoice;

  static char *my_menu[] = {
      "Exit",
      "Test Erase Char",
      "Test Previous-Line (CPL)",
      "Test Protected-Areas (DECSCA)",
      "Test Protected-Areas (SPA)",
      "Test Visible/Invisible Cursor (DECTCEM)",
      "Test 8-bit controls (C7C1T/C8C1T)",
      ""
    };

  do {
    ed(2);
    cup(5, 10); printf("VT220/VT320 Tests");
    cup(7, 10); println("Choose test type:");
    menuchoice = menu(my_menu);
    switch (menuchoice) {
      case 1: tst_ECH();         break;
      case 2: tst_CPL();         break;
      case 3: tst_DECSCA();      break;
      case 4: tst_SPA();         break;
      case 5: tst_DECTCEM();     break;
      case 6: tst_C8C1T();       break;
    }
  } while (menuchoice);
}

/******************************************************************************/

static
void tst_ecma48_curs(void)
{
  int menuchoice;

  static char *my_menu[] = {
      "Exit",
      "Test Character-Position-Absolute (HPA)",
      "Test Cursor-Back-Tab (CBT)",
      "Test Cursor-Character-Absolute (CHA)",
      "Test Cursor-Horizontal-Index (CHT)",
      "Test Line-Position-Absolute (VPA)",
      "Test Next-Line (CNL)",
      ""
    };

  do {
    ed(2);
    cup(5, 10); printf("ISO-6429 (ECMA-48) Cursor-Movement");
    cup(7, 10); println("Choose test type:");
    menuchoice = menu(my_menu);
    switch (menuchoice) {
      case  1: tst_HPA();         break;
      case  2: tst_CBT();         break;
      case  3: tst_CHA();         break;
      case  4: tst_CHT();         break;
      case  5: tst_VPA();         break;
      case  6: tst_CNL();         break;
    }
  } while (menuchoice);
}

static
void tst_ecma48_misc(void)
{
  int menuchoice;

  static char *my_menu[] = {
      "Exit",
      "Test Scroll-Down (SD)",
      "Test Scroll-Left (SL)",
      "Test Scroll-Right (SR)",
      "Test Scroll-Up (SU)",
      ""
    };

  do {
    ed(2);
    cup(5, 10); printf("Miscellaneous ISO-6429 (ECMA-48) Tests");
    cup(7, 10); println("Choose test type:");
    menuchoice = menu(my_menu);
    switch (menuchoice) {
      case 1: tst_SD();          break;
      case 2: tst_SL();          break;
      case 3: tst_SR();          break;
      case 4: tst_SU();          break;
    }
  } while (menuchoice);
}

/******************************************************************************/
void tst_nonvt100(void)
{
  int menuchoice;

  static char *my_menu[] = {
      "Exit",
      "Test of VT220/VT320 features (Erase Char, etc.)",
      "Test ISO-6429 cursor-movement",
      "Test ISO-6429 colors",
      "Test other ISO-6429 features",
      "Test XTERM special features",
      ""
    };

  do {
    ed(2);
    cup(5, 10); printf("Non-VT100 Tests");
    cup(7, 10); println("Choose test type:");
    menuchoice = menu(my_menu);
    switch (menuchoice) {
      case 1: tst_vt220();       break;
      case 2: tst_ecma48_curs(); break;
      case 3: tst_colors();      break;
      case 4: tst_ecma48_misc(); break;
      case 5: tst_xterm();       break;
    }
  } while (menuchoice);
}

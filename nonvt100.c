/* $Id: nonvt100.c,v 1.21 1996/08/19 00:23:12 tom Exp $ */

/*
 * The list of non-VT320 codes was compiled using the list of non-VT320 codes
 * described in the Kermit 3.13 documentation, combined with the ISO-6429
 * (ECMA-48) spec.
 */
#include <vttest.h>
#include <ttymodes.h>
#include <esc.h>

int
not_impl(MENU_ARGS)
{
  cup(1,1);
  printf("Sorry, test not implemented:\r\n\r\n  %s", the_title);
  cup(max_lines-1,1);
  return MENU_HOLD;
}

static void
report_ok(char *ref, char *tst)
{
  if ((tst = skip_prefix(csi_input(), tst)) == 0)
    tst = "?";
  printf(" (%s)", !strcmp(ref, tst) ? "success" : "fail");
}

/* Kermit 3.13 and dtterm implement this; it's probably for VT220 */
static int
tst_C8C1T(MENU_ARGS)
{
  char *report;

  ed(2);
  cup(5,1);
  println("This tests the VT200+ control sequence to direct the terminal to emit 8-bit");
  println("control-sequences instead of <esc> sequences.");

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  s8c1t(1);
  cup(1,1); dsr(6);
  report = instr();
  cup(10,1);
  printf("8-bit controls enabled: ");
  chrprint(report);
  report_ok("1;1R", report);

  s8c1t(0);
  cup(1,1); dsr(6);
  report = instr();
  cup(12,1);
  printf("8-bit controls disabled: ");
  chrprint(report);
  report_ok("1;1R", report);

  restore_ttymodes();
  cup(max_lines-1,1);
  return MENU_HOLD;
}

/* VT420 doesn't do this */
static int
tst_CBT(MENU_ARGS)
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
  return MENU_HOLD;
}

/* Note: CHA and HPA have identical descriptions in ECMA-48 */
/* dtterm implements this (VT400 doesn't) */
static int
tst_CHA(MENU_ARGS)
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
  return MENU_HOLD;
}

/*
 * Kermit's documentation refers to this as CHI, ECMA-48 as CHT.
 *
 * VT420 doesn't do this
 */
static int
tst_CHT(MENU_ARGS)
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
  return MENU_HOLD;
}

/* VT420 doesn't do this */
static int
tst_CNL(MENU_ARGS)
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
  return MENU_HOLD;
}

/*
 * There's a comment in the MS-DOS Kermit 3.13 documentation that implies CPL
 * is used to replace RI (reverse-index).  ECMA-48 doesn't specify scrolling
 * regions, DEC terminals do apparently, so for CPL and CNL we'll test this.
 */
static int
tst_CPL(MENU_ARGS)
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
  return MENU_HOLD;
}

/*
 * Test DEC's selective-erase (set-protected area) by drawing a box of
 * *'s that will remain, and a big X of *'s that gets cleared..
 */
static int
tst_DECSCA(MENU_ARGS)
{
  int i, j, pass;
  int tmar = 5;
  int bmar = max_lines - 8;
  int lmar = 20;
  int rmar = min_cols - lmar;

  ed(2);

  for (pass = 0; pass < 2; pass++) {
    if (pass == 0)
      decsca(1);
    for (i = tmar; i <= bmar; i++) {
      cup(i, lmar);
      for (j = lmar; j <= rmar; j++) {
        printf("*");
      }
    }
    if (pass == 0) {
      decsca(0);

      for (j = 0; j <= 2; j++) {
        for (i = 1; i < tmar; i++) {
          cup(i, lmar - tmar + (i+j)); printf("*");
          cup(i, rmar + tmar - (i+j)); printf("*");
        }
        for (i = bmar + 1; i < max_lines; i++) {
          cup(i, lmar + bmar - i + j); printf("*");
          cup(i, rmar - bmar + i - j); printf("*");
        }
        cup(max_lines/2, min_cols/2);
        decsed(j);
      }

      for (i = rmar+1; i <= min_cols; i++) {
        cup(tmar, i);        printf("*");
        cup(max_lines/2, i); printf("*");
      }
      cup(max_lines/2, min_cols/2);
      decsel(0); /* after the cursor */

      for (i = 1; i < lmar; i++) {
        cup(tmar, i);        printf("*");
        cup(max_lines/2, i); printf("*");
      }
      cup(max_lines/2, min_cols/2);
      decsel(1); /* before the cursor */

      cup(tmar, min_cols/2);
      decsel(2); /* the whole line */

      cup(max_lines-3, 1);
      ed(0);
      println("If your terminal supports DEC protected areas (DECSCA, DECSED, DECSEL),");
      println("there will be an solid box made of *'s in the middle of the screen.");
      holdit();
    }
  }
  return MENU_NOHOLD;
}

/*
 * Test if the terminal can make the cursor invisible
 */
static int
tst_DECTCEM(MENU_ARGS)
{
  ed(2);
  cup(1,1);
  rm("?25");
  println("The cursor should be invisible");
  holdit();
  sm("?25");
  println("The cursor should be visible again");
  return MENU_HOLD;
}

/*
 * VT200 and higher
 *
 * Test to ensure that 'ech' (erase character) is honored, with no parameter,
 * explicit parameter, and longer than the screen width (to ensure that the
 * terminal doesn't try to wrap-around the erasure).
 */
int
tst_ECH(MENU_ARGS)
{
  int i;

  ed(2);
  decaln();
  for (i = 1; i <= max_lines; i++) {
    cup(i, min_cols - i - 2);
    do_csi("X"); /* make sure default-parameter works */
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
  return MENU_HOLD;
}

/* VT420 doesn't do this */
static int
tst_HPA(MENU_ARGS)
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
  return MENU_HOLD;
}

/*
 * Test the SD (scroll-down) by forcing characters written in a diagonal into
 * a horizontal row.
 *
 * vt400 and dtterm use the (incorrect?) escape sequence (ending with 'T'
 * instead of '^'), apparently someone misread 05/14 as 05/04 or vice versa.
 */
int
tst_SD_DEC(MENU_ARGS)
{
  int n;
  int last = max_lines - 3;

  ed(2);
  for (n = 1; n < last; n++) {
    cup(n, n);
    printf("*");
    sd_dec(1);
  }
  cup(last+1,1);
  ed(0);
  println("If your terminal supports SD, there is a horizontal row of *'s above.");
  return MENU_HOLD;
}

static int
tst_SD_ISO(MENU_ARGS)
{
  int n;
  int last = max_lines - 3;

  ed(2);
  for (n = 1; n < last; n++) {
    cup(n, n);
    printf("*");
    sd_iso(1);
  }
  cup(last+1,1);
  ed(0);
  println("If your terminal supports SD, there is a horizontal row of *'s above.");
  return MENU_HOLD;
}

/*
 * Test the SL (scroll-left) by forcing characters written in a diagonal into
 * a vertical line.
 */
static int
tst_SL(MENU_ARGS)
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
  return MENU_HOLD;
}

/*
 * Test the SR (scroll-right) by forcing characters written in a diagonal into
 * a vertical line.
 */
static int
tst_SR(MENU_ARGS)
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
  return MENU_HOLD;
}

/*
 * Test the SU (scroll-up) by forcing characters written in a diagonal into
 * a horizontal row.
 */
int
tst_SU(MENU_ARGS)
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
  return MENU_HOLD;
}

/*
 * Test SPA (set-protected area)
 */
static int
tst_SPA(MENU_ARGS)
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
      println("*'s in the middle of the screen.  (VT420 does not implement this)");
      holdit();
    }
  }
  return MENU_NOHOLD;
}

/*
 * Kermit's documentation refers to this as CVA, ECMA-48 as VPA.
 * Move the cursor in the current column to the specified line.
 *
 * VT420 doesn't do this
 */
static int
tst_VPA(MENU_ARGS)
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
  return MENU_HOLD;
}

/******************************************************************************/

static int
tst_Editing(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Delete Column (DECDC)",                        not_impl },
      { "Test Erase Char (ECH)",                             tst_ECH },
      { "Test Insert Column (DECIC)",                        not_impl },
      { "Test Protected-Areas (DECSCA)",                     tst_DECSCA },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("Editing Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
tst_PageFormat(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test set columns per page (DECSCPP)",               not_impl },
      { "Test columns mode (DECCOLM)",                       not_impl },
      { "Test set lines per page (DECSLPP)",                 not_impl },
      { "Test set left and right margins (DECSLRM)",         not_impl },
      { "Test set vertical split-screen (DECVSSM)",          not_impl },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("Page Format Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
tst_PageMovement(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Next Page (NP)",                               not_impl },
      { "Test Preceding Page (PP)",                          not_impl },
      { "Test Page Position Absolute (PPA)",                 not_impl },
      { "Test Page Position Backward (PPB)",                 not_impl },
      { "Test Page Position Relative (PPR)",                 not_impl },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("Page Format Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/* FIXME: some of the VT420 stuff should be here... */

static int
tst_vt220(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test page-format controls",                         tst_PageFormat },
      { "Test page-movement controls",                       tst_PageMovement },
      { "Test editing controls",                             tst_Editing },
      { "Test Previous-Line (CPL)",                          tst_CPL },
      { "Test Visible/Invisible Cursor (DECTCEM)",           tst_DECTCEM },
      { "Test 8-bit controls (C7C1T/C8C1T)",                 tst_C8C1T },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT220/VT320 Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
tst_ecma48_curs(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Character-Position-Absolute (HPA)",            tst_HPA },
      { "Test Cursor-Back-Tab (CBT)",                        tst_CBT },
      { "Test Cursor-Character-Absolute (CHA)",              tst_CHA },
      { "Test Cursor-Horizontal-Index (CHT)",                tst_CHT },
      { "Test Line-Position-Absolute (VPA)",                 tst_VPA },
      { "Test Next-Line (CNL)",                              tst_CNL },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("ISO-6429 (ECMA-48) Cursor-Movement");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

static int
tst_ecma48_misc(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Protected-Areas (SPA)",                        tst_SPA },
      { "Test Scroll-Down (SD)",                             tst_SD_ISO },
      { "Test Scroll-Left (SL)",                             tst_SL },
      { "Test Scroll-Right (SR)",                            tst_SR },
      { "Test Scroll-Up (SU)",                               tst_SU },
      { "",                                                  0 },
    };

  do {
    ed(2);
    title(0); printf("Miscellaneous ISO-6429 (ECMA-48) Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/
int
tst_nonvt100(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test of VT220/VT320 features",                      tst_vt220 },
      { "Test of VT420 features",                            tst_vt420 },
      { "Test ISO-6429 cursor-movement",                     tst_ecma48_curs },
      { "Test ISO-6429 colors",                              tst_colors },
      { "Test other ISO-6429 features",                      tst_ecma48_misc },
      { "Test XTERM special features",                       tst_xterm },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("Non-VT100 Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

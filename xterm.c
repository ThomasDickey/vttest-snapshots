/* $Id: xterm.c,v 1.3 1996/07/22 00:06:59 tom Exp $ */

#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>
#include <xterm.h>

static void
test_modify_ops(void)
{
  int n;
  int wide, high;
  char temp[100];

  ed(2);

  cup(1,1);
  println("Test of Window modifying.");

  brc(2, 't');	/* iconify window */
  println("Iconify");
  fflush(stdout); sleep(2);

  brc(1, 't');	/* de-iconify window */
  println("De-Iconify");
  fflush(stdout); sleep(1);

  ed(2);
  for (n = 0; n <= 200; n += 5) {
    sprintf(temp, "Position (%d,%d)", n, n*2);
    println(temp);
    esc("K");		/* Erase to end of line	*/
    brc3(3, n, n*2, 't');
    fflush(stdout);
  }
  holdit();

  ed(2);
  brc3(3, 0, 0, 't');

  for (n = 0; n <= 200; n += 10) {
    wide = n+20;
    high = n+50;
    brc3(4, high, wide, 't');
    sprintf(temp, "%d x %d pixels", high, wide);
    println(temp);
    fflush(stdout);
  }
  holdit();

  ed(2);
  for (n = 0; n <= 200; n += 10) {
    high = n+50;
    brc3(4, high, 0, 't');
    sprintf(temp, "%d x (screen-width) pixels", high);
    println(temp);
    fflush(stdout);
  }
  holdit();

  ed(2);
  for (n = 0; n <= 300; n += 10) {
    wide = n+50;
    brc3(4, 0, wide, 't');
    sprintf(temp, "(screen-height) x %d pixels", wide);
    println(temp);
    fflush(stdout);
  }
  holdit();

  while (n >= 200) {
    wide = n+50;
    high = 500 - n;
    brc3(4, high, wide, 't');
    sprintf(temp, "%d x %d pixels", high, wide);
    println(temp);
    fflush(stdout);
    n -= 5;
  }
  holdit();

  while (n <= 300) {
    wide = n+50;
    high = 500 - n;
    brc3(4, high, wide, 't');
    sprintf(temp, "%d x %d pixels", high, wide);
    println(temp);
    fflush(stdout);
    n += 5;
  }
  holdit();

  ed(2);
  for (n = 5; n <= 20; n++) {
    wide = n*4;
    high = n+5;
    brc3(8, high, wide, 't');
    sprintf(temp, "%d x %d chars", high, wide);
    while (strlen(temp) < wide - 1)
    	strcat(temp, ".");
    println(temp);
    fflush(stdout);
  }
  holdit();

  ed(2);
  for (n = 5; n <= 24; n++) {
    high = n;
    brc3(8, high, 0, 't');
    sprintf(temp, "%d x (screen-width) chars", high);
    println(temp);
    fflush(stdout);
  }
  holdit();

  ed(2);
  for (n = 5; n <= 80; n++) {
    wide = n;
    brc3(8, 0, wide, 't');
    sprintf(temp, "(screen-height) x %d chars", wide);
    println(temp);
    fflush(stdout);
  }
  holdit();

  brc3(3, 200, 200, 't');
  brc3(8, 24, 80, 't');
  println("Reset to 24 x 80");

  ed(2);
  println("Lower");
  brc(6, 't');
  holdit();

  ed(2);
  println("Raise");
  brc(5, 't');
  holdit();
}

static void
test_report_ops(void)
{
  char *report;

  ed(2);

  cup(1,1);
  println("Test of Window reporting.");
  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  cup(3,1);
  println("Report icon label:");
  cup(4,10);
  brc(20, 't');	/* report icon label */
  report = instr();
  chrprint(report);

  cup(5,1);
  println("Report window label:");
  cup(6,10);
  brc(21, 't');	/* report window label */
  report = instr();
  chrprint(report);

  cup(7,1);
  println("Report size of window (chars):");
  cup(8,10);
  brc(18, 't');	/* report window's text-size */
  report = instr();
  chrprint(report);

  cup(9,1);
  println("Report size of window (pixels):");
  cup(10,10);
  brc(14, 't');	/* report window's pixel-size */
  report = instr();
  chrprint(report);

  cup(11,1);
  println("Report position of window (pixels):");
  cup(12,10);
  brc(13, 't');	/* report window's pixel-size */
  report = instr();
  chrprint(report);

  cup(13,1);
  println("Report state of window (normal/iconified):");
  cup(14,10);
  brc(11, 't');	/* report window's pixel-size */
  report = instr();
  chrprint(report);

  set_tty_raw(FALSE);
  set_tty_echo(TRUE);

  cup(20,1);
  holdit();
}

/*
 * xterm (and derived programs such as hpterm, dtterm, rxvt) are the most
 * widely used vt100 near-compatible terminal emulators (other than modem
 * programs).
 */
void
tst_xterm(void)
{
  int menuchoice;
  static char *colormenu[] = {
    "Return to main menu",
    "Window modify-operations",
    "Window report-operations",
    ""
  };

  do {
    ed(2);
    cup(7,10); println("Choose test type:");
    menuchoice = menu(colormenu);
    switch (menuchoice) {
    case 1: test_modify_ops();   break;
    case 2: test_report_ops();   break;
    }
  } while (menuchoice);
}

/* $Id: xterm.c,v 1.16 1997/04/19 01:00:58 tom Exp $ */

#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>

#define MCHR(c) (((c) - ' ') & 0xff)

#define isQuit(c) (((c) == 'q') || ((c) == 'Q'))

static void
show_click(int y, int x, int c)
{
  cup(y,x);
  putchar(c);
  vt_move(y,x);
  fflush(stdout);
}

static int
test_modify_ops(MENU_ARGS)
{
  int n;
  int wide, high;
  char temp[100];

  vt_move(1,1);
  println("Test of Window modifying.");

  brc(2, 't'); /* iconify window */
  println("Iconify");
  fflush(stdout); sleep(2);

  brc(1, 't'); /* de-iconify window */
  println("De-Iconify");
  fflush(stdout); sleep(1);

  ed(2);
  for (n = 0; n <= 200; n += 5) {
    sprintf(temp, "Position (%d,%d)", n, n*2);
    println(temp);
    esc("K"); /* Erase to end of line */
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
  return MENU_HOLD;
}

/* Mouse Highlight Tracking */
static int
test_mouse_hilite(MENU_ARGS)
{
  int y = 0, x = 0;

  vt_move(1,1);
  println(the_title);
  println("Press 'q' to quit.  Mouse events will be marked with the button number.");

  sm("?1001");
  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  for(;;) {
    char *report = instr();
    if (isQuit(*report))
      break;
    vt_move(3,10); vt_el(2); chrprint(report);
    if ((report = skip_csi(report)) != 0) {
      if (*report == 'M'
       && strlen(report) == 4) {
        int b = MCHR(report[1]);
        b &= 3;
        x = MCHR(report[2]);
        y = MCHR(report[3]);
        if (b != 3) {
          /* send the xterm the highlighting range (it MUST be done first) */
          do_csi("1;%d;%d;%d;%d;T", x, y, 10, 20);
          /* now, show the mouse-click */
          show_click(y, x, b + 1 + '0');
        }
        /* interpret the event */
        vt_move(4,10); vt_el(2);
        show_result("tracking: code 0x%x (%d,%d)", MCHR(report[1]), y, x);
        fflush(stdout);
      } else if (*report == 'T' && strlen(report) == 7) {
        /* interpret the event */
        vt_move(4,10); vt_el(2);
        show_result("done: start(%d,%d), end(%d,%d), mouse(%d,%d)",
          MCHR(report[2]), MCHR(report[1]),
          MCHR(report[4]), MCHR(report[3]),
          MCHR(report[6]), MCHR(report[5]));
        if (MCHR(report[2]) != y
         || MCHR(report[1]) != x)
          show_click(MCHR(report[2]), MCHR(report[1]), 's');
        if (MCHR(report[4]) != y
         || MCHR(report[3]) != x)
          show_click(MCHR(report[4]), MCHR(report[3]), 'e');
        if (MCHR(report[6]) != y
         || MCHR(report[5]) != x)
          show_click(MCHR(report[6]), MCHR(report[5]), 'm');
      } else if (*report == 't' && strlen(report) == 3) {
        /* interpret the event */
        vt_move(4,10); vt_el(2);
        show_result("done: end(%d,%d)",
          MCHR(report[2]), MCHR(report[1]));
        if (MCHR(report[2]) != y
         || MCHR(report[1]) != x)
          show_click(MCHR(report[2]), MCHR(report[1]), 'e');
      }
    }
  }

  rm("?1001");
  restore_ttymodes();

  vt_move(max_lines-2,1);
  return MENU_HOLD;
}

/* Normal Mouse Tracking */
static int
test_mouse_normal(MENU_ARGS)
{
  int y = 0, x = 0;

  vt_move(1,1);
  println(the_title);
  println("Press 'q' to quit.  Mouse events will be marked with the button number.");

  sm("?1000");
  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  for(;;) {
    char *report = instr();
    if (isQuit(*report))
      break;
    vt_move(3,10); vt_el(2); chrprint(report);
    if ((report = skip_csi(report)) != 0
     && *report == 'M'
     && strlen(report) == 4) {
      int b = MCHR(report[1]);
      vt_move(4,10); vt_el(2);
      show_result("code 0x%x (%d,%d)", b, MCHR(report[3]), MCHR(report[2]));
      b &= 3;
      if (b != 3)
        show_click(MCHR(report[3]), MCHR(report[2]), b + 1 + '0');
      else if (MCHR(report[2]) != x || MCHR(report[3]) != y)
        show_click(MCHR(report[3]), MCHR(report[2]), '*');
      x = MCHR(report[2]);
      y = MCHR(report[3]);
    }
  }

  rm("?1000");
  restore_ttymodes();

  vt_move(max_lines-2,1);
  return MENU_HOLD;
}

static int
test_report_ops(MENU_ARGS)
{
  char *report;

  vt_move(1,1);
  println("Test of Window reporting.");
  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  vt_move(3,1);
  println("Report icon label:");
  vt_move(4,10);
  brc(20, 't'); /* report icon label */
  report = instr();
  chrprint(report);

  vt_move(5,1);
  println("Report window label:");
  vt_move(6,10);
  brc(21, 't'); /* report window label */
  report = instr();
  chrprint(report);

  vt_move(7,1);
  println("Report size of window (chars):");
  vt_move(8,10);
  brc(18, 't'); /* report window's text-size */
  report = instr();
  chrprint(report);

  vt_move(9,1);
  println("Report size of window (pixels):");
  vt_move(10,10);
  brc(14, 't'); /* report window's pixel-size */
  report = instr();
  chrprint(report);

  vt_move(11,1);
  println("Report position of window (pixels):");
  vt_move(12,10);
  brc(13, 't'); /* report window's pixel-size */
  report = instr();
  chrprint(report);

  vt_move(13,1);
  println("Report state of window (normal/iconified):");
  vt_move(14,10);
  brc(11, 't'); /* report window's pixel-size */
  report = instr();
  chrprint(report);

  vt_move(20,1);
  restore_ttymodes();
  return MENU_HOLD;
}

/* Set window title */
static int
test_window_name(MENU_ARGS)
{
  char temp[BUFSIZ];

  vt_move(1,1);
  println("Please enter the new window name.  Newer xterms may beep when setting the title.");
  inputline(temp);
  do_osc("0;%s%c", temp, BEL);
  return MENU_NOHOLD;
}

/* X10 Mouse Compatibility */
static int
test_X10_mouse(MENU_ARGS)
{
  vt_move(1,1);
  println(the_title);
  println("Press 'q' to quit.  Mouse events will be marked with the button number.");

  sm("?9");
  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  for(;;) {
    char *report = instr();
    if (isQuit(*report))
      break;
    vt_move(3,10); vt_el(2); chrprint(report);
    if ((report = skip_csi(report)) != 0
     && *report == 'M'
     && strlen(report) == 4) {
      int x = report[2] - ' ';
      int y = report[3] - ' ';
      cup(y,x);
      printf("%d", report[1] - ' ' + 1);
      vt_move(y,x);
      fflush(stdout);
    }
  }

  rm("?9");
  restore_ttymodes();

  vt_move(max_lines-2,1);
  return MENU_HOLD;
}

/*
 * xterm (and derived programs such as hpterm, dtterm, rxvt) are the most
 * widely used vt100 near-compatible terminal emulators (other than modem
 * programs).
 */
int
tst_xterm(MENU_ARGS)
{
  static MENU my_menu[] = {
    { "Return to main menu",                                 0 },
    { "Set window title",                                    test_window_name },
    { "X10 Mouse Compatibility",                             test_X10_mouse },
    { "Normal Mouse Tracking",                               test_mouse_normal },
    { "Mouse Highlight Tracking",                            test_mouse_hilite },
    { "Window modify-operations",                            test_modify_ops },
    { "Window report-operations",                            test_report_ops },
    { "",                                                    0 }
  };

  do {
    vt_clear(2);
    title(0); println("XTERM special features");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/* $Id: xterm.c,v 1.44 2010/05/28 00:41:33 tom Exp $ */

#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>

#define Pause(secs) fflush(stdout); sleep(secs)

static void
check_rc(int row, int col)
{
  char *report;
  char *params;
  char expected[80];

  sprintf(expected, "%d;%dR", row, col);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);
  do_csi("6n");
  report = get_reply();
  restore_ttymodes();

  vt_move(row, 1);
  el(2);
  if ((params = skip_csi(report)) == 0
      || strcmp(params, expected) != 0) {
    printf("cursor save/restore %s, got \"%s\", expected \"%s\"",
           SHOW_FAILURE, params, expected);
  } else {
    printf("cursor save/restore %s", SHOW_SUCCESS);
  }
}

static int
test_altscrn_47(MENU_ARGS)
{
  vt_move(1, 1);
  println(the_title);
  vt_move(3, 1);
  println("Test private setmode 47 (to/from alternate screen)");
  vt_move(4, 1);
  println("The next screen will be filled with E's down to the prompt.");
  vt_move(7, 5);
  decsc();
  vt_move(max_lines - 2, 1);
  holdit();

  sm("?47");
  decaln();     /* fill the screen */
  vt_move(15, 7);
  decsc();
  vt_move(max_lines - 2, 1);
  ed(0);
  holdit();

  rm("?47");
  decrc();
  check_rc(7, 5);
  vt_move(4, 1);
  el(2);
  println("The original screen should be restored except for this line.");
  vt_move(max_lines - 2, 1);
  return MENU_HOLD;
}

static int
test_altscrn_1047(MENU_ARGS)
{
  vt_move(1, 1);
  println(the_title);
  vt_move(3, 1);
  println("Test private setmode 1047 (to/from alternate screen)");
  vt_move(4, 1);
  println("The next screen will be filled with E's down to the prompt");
  vt_move(5, 1);
  println("unless titeInhibit resource is set, or alternate-screen is disabled.");
  vt_move(7, 5);
  decsc();
  vt_move(9, 7);  /* move away from the place we saved with DECSC */
  sm("?1048");  /* this saves the cursor position */
  vt_move(max_lines - 2, 1);
  holdit();

  sm("?1047");
  decaln();     /* fill the screen */
  vt_move(15, 7);
  decsc();
  vt_move(max_lines - 2, 1);
  ed(0);
  holdit();

  rm("?1047");
  decrc();
  rm("?1048");
  check_rc(9, 7);
  vt_move(4, 1);
  el(2);
  println("The original screen should be restored except for this line");
  vt_move(max_lines - 2, 1);
  return MENU_HOLD;
}

static int
test_altscrn_1049(MENU_ARGS)
{
  vt_move(1, 1);
  println(the_title);
  vt_move(3, 1);
  println("Test private setmode 1049 (to/from alternate screen)");
  vt_move(4, 1);
  println("The next screen will be filled with E's down to the prompt.");
  vt_move(5, 1);
  println("unless titeInhibit resource is set, or alternate-screen is disabled.");
  vt_move(7, 5);
  decsc();
  vt_move(max_lines - 2, 1);
  holdit();     /* cursor location will be one line down */

  sm("?1049");  /* this saves the cursor location */
  decaln();     /* fill the screen */
  vt_move(max_lines - 2, 1);
  ed(0);
  holdit();

  rm("?1049");
  decrc();
  check_rc(max_lines - 1, 1);
  vt_move(4, 1);
  el(2);
  println("The original screen should be restored except for this line");
  vt_move(max_lines - 2, 1);
  return MENU_HOLD;
}

/*
 * Xterm implements an alternate screen, which is used to save the command-line
 * screen to restore it after running a full-screen application.
 *
 * The original scheme used separate save/restore-cursor and clear-screen
 * operations in conjunction with a toggle to/from the alternate screen
 * (private setmode 47).  Since not all users want the feature, xterm also
 * implements the titeInhibit resource to make it populate the $TERMCAP
 * variable without the ti/te (smcup/rmcup) strings which hold those sequences.
 * The limitation of titeInhibit is that it cannot work for terminfo, because
 * that information is read from a file rather than the environment.  I
 * implemented a corresponding set of strings for private setmode 1047 and 1048
 * to model the termcap behavior in terminfo.
 *
 * The behavior of the save/restore cursor operations still proved
 * unsatisfactory since users would (even in the original private setmode 47)
 * occasionally run shell programs from within their full-screen application
 * which would do a save-cursor to a different location, causing the final
 * restore-cursor to place the cursor in an unexpected location.  The private
 * setmode 1049 works around this by using a separate memory location to store
 * its version of the cursor location.
 */
static int
tst_altscrn(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
    { "Exit",                                                0 },
    { "Switch to/from alternate screen (xterm)",             test_altscrn_47 },
    { "Improved alternate screen (XFree86 xterm mode 1047)", test_altscrn_1047 },
    { "Better alternate screen (XFree86 xterm mode 1049)",   test_altscrn_1049 },
    { "",                                                    0 }
  };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), println("XTERM Alternate-Screen features"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

#define NUMFONTS 7

static int
tst_modify_font(MENU_ARGS)
{
  char temp[BUFSIZ];

  vt_move(1, 1);
  println("Please enter the font name.");
  println("You can use '#' number to set fonts relative (with +/- sign) and");
  println("absolute (with a number) based on the current font's position in");
  println("the font-menu.  Examples:");
  println("    \"fixed\" to set the current font to \"fixed\"");
  println("    \"#fixed\" to set the current font to \"fixed\"");
  println("    \"#-fixed\" to set the previous font to \"fixed\"");
  println("    \"#-2 fixed\" to set the next-previous font to \"fixed\"");
  println("    \"#+fixed\" to set the following font to \"fixed\"");
  vt_move(11, 1);
  printf(">");
  inputline(temp);
  do_osc("50;%s%c", temp, BEL);
  return MENU_HOLD;
}

static int
tst_report_font(MENU_ARGS)
{
  int n;
  char *report;
  int row = 1;
  int col = 1;

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  vt_move(row, col);
  println("Current font:");
  ++row;

  vt_move(row, col + 6);
  do_osc("50;?%c", BEL);
  report = instr();
  row = chrprint2(report, row, col);

  ++row;
  vt_move(row, col);
  println("Absolute fonts:");
  ++row;

  for (n = 0; n < NUMFONTS; ++n) {
    vt_move(row, col);
    do_osc("50;?%d%c", n, BEL);
    report = instr();
    if (strchr(report, ';') != 0) {
      printf("  %2d: ", n);
      row = chrprint2(report, row, col);
    }
  }

  ++row;

  vt_move(row, col);
  println("Relative fonts (bell may sound):");
  ++row;

  for (n = -NUMFONTS; n < NUMFONTS; ++n) {
    vt_move(row, col);
    do_osc("50;?%c%d%c", n >= 0 ? '+' : '-', n >= 0 ? n : -n, BEL);
    report = instr();
    if (strchr(report, ';') != 0) {
      printf("  %2d: ", n);
      row = chrprint2(report, row, col);
    } else if (n >= 0) {
      break;
    }
  }

  restore_ttymodes();
  return MENU_HOLD;
}

static int
tst_font(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
    { "Exit",                                                0 },
    { "Modify font",                                         tst_modify_font },
    { "Report font(s)",                                      tst_report_font },
    { "",                                                    0 }
  };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), println("XTERM Font features"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

static int
test_modify_ops(MENU_ARGS)
{
  int n;
  int wide, high;
  char temp[100];

  vt_move(1, 1);
  println("Test of Window modifying.");

  brc(2, 't');  /* iconify window */
  println("Iconify");
  Pause(2);

  brc(1, 't');  /* de-iconify window */
  println("De-Iconify");
  Pause(1);

  ed(2);
  for (n = 0; n <= 200; n += 5) {
    sprintf(temp, "Position (%d,%d)", n, n * 2);
    println(temp);
    esc("K");   /* Erase to end of line */
    brc3(3, n, n * 2, 't');
    fflush(stdout);
  }
  holdit();

  ed(2);
  brc3(3, 0, 0, 't');

  for (n = 0; n <= 200; n += 10) {
    wide = n + 20;
    high = n + 50;
    brc3(4, high, wide, 't');
    sprintf(temp, "%d x %d pixels", high, wide);
    println(temp);
    fflush(stdout);
  }
  holdit();

  ed(2);
  for (n = 0; n <= 200; n += 10) {
    high = n + 50;
    brc3(4, high, 0, 't');
    sprintf(temp, "%d x (screen-width) pixels", high);
    println(temp);
    fflush(stdout);
  }
  holdit();

  ed(2);
  for (n = 0; n <= 300; n += 10) {
    wide = n + 50;
    brc3(4, 0, wide, 't');
    sprintf(temp, "(screen-height) x %d pixels", wide);
    println(temp);
    fflush(stdout);
  }
  holdit();

  while (n >= 200) {
    wide = n + 50;
    high = 500 - n;
    brc3(4, high, wide, 't');
    sprintf(temp, "%d x %d pixels", high, wide);
    println(temp);
    fflush(stdout);
    n -= 5;
  }
  holdit();

  while (n <= 300) {
    wide = n + 50;
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
    wide = n * 4;
    high = n + 5;
    brc3(8, high, wide, 't');
    sprintf(temp, "%d x %d chars", high, wide);
    while ((int) strlen(temp) < wide - 1)
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

static int
test_report_ops(MENU_ARGS)
{
  char *report;
  int row = 3;
  int col = 10;

  vt_move(1, 1);
  println("Test of Window reporting.");
  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  vt_move(row++, 1);
  println("Report icon label:");
  vt_move(row, col);
  brc(20, 't'); /* report icon label */
  report = instr();
  row = chrprint2(report, row, col);

  vt_move(row++, 1);
  println("Report window label:");
  vt_move(row, col);
  brc(21, 't'); /* report window label */
  report = instr();
  row = chrprint2(report, row, col);

  vt_move(row++, 1);
  println("Report size of window (chars):");
  vt_move(row++, col);
  brc(18, 't'); /* report window's text-size */
  report = instr();
  chrprint(report);

  vt_move(row++, 1);
  println("Report size of window (pixels):");
  vt_move(row++, col);
  brc(14, 't'); /* report window's pixel-size */
  report = instr();
  chrprint(report);

  vt_move(row++, 1);
  println("Report position of window (pixels):");
  vt_move(row++, col);
  brc(13, 't'); /* report window's pixel-size */
  report = instr();
  chrprint(report);

  vt_move(row++, 1);
  println("Report state of window (normal/iconified):");
  vt_move(row, col);
  brc(11, 't'); /* report window's pixel-size */
  report = instr();
  chrprint(report);

  vt_move(20, 1);
  restore_ttymodes();
  return MENU_HOLD;
}

/* Set window title */
static int
test_window_name(MENU_ARGS)
{
  char temp[BUFSIZ];

  vt_move(1, 1);
  println("Please enter the new window name.  Newer xterms may beep when setting the title.");
  inputline(temp);
  do_osc("0;%s%c", temp, BEL);
  return MENU_NOHOLD;
}

/*
 * xterm (and derived programs such as hpterm, dtterm, rxvt) are the most
 * widely used vt100 near-compatible terminal emulators (other than modem
 * programs).  dtterm emulates a vt220, as does XFree86 xterm.  DECterm
 * emulates a vt320.
 */
int
tst_xterm(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
    { "Exit",                                                0 },
    { "Set window title",                                    test_window_name },
    { "Font features",                                       tst_font },
    { "Mouse features",                                      tst_mouse },
    { "Tektronix 4014 features",                             tst_tek4014 },
    { "Alternate-Screen features (xterm)",                   tst_altscrn },
    { "Window modify-operations (dtterm)",                   test_modify_ops },
    { "Window report-operations (dtterm)",                   test_report_ops },
    { "",                                                    0 }
  };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), println("XTERM special features"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

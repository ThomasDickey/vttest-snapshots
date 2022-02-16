/* $Id: status.c,v 1.11 2022/02/16 01:28:19 tom Exp $ */

#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>

static void
restore_status(void)
{
  decsasd(0);   /* main display */
  decssdt(1);   /* indicator (default) */
  restore_ttymodes();
}

static int
simple_statusline(MENU_ARGS)
{
  static char text[] = "TEXT IN THE STATUS LINE";

  vt_move(1, 1);
  println("This is a simple test of the status-line");
  println("");

  decssdt(2);
  decsasd(1);
  tprintf("%s", text);
  decsasd(0);
  printxx("There should be %s\r\n", text);
  holdit();

  decssdt(0);
  println("There should be no status line");
  holdit();

  decssdt(1);
  println("The status line should be normal (i.e., indicator)");
  holdit();

  restore_status();
  return MENU_NOHOLD;
}

static int
SGR_statusline(MENU_ARGS)
{
  vt_move(1, 1);
  println("This test writes SGR controls to the status-line");
  holdit();

  decssdt(2);
  decsasd(1);

  el(2);
  cup(1, 1);
  sgr("1");
  tprintf("BOLD ");
  sgr("0;4");
  tprintf("Underlined ");
  sgr("0;7");
  tprintf("Reverse ");
  sgr("0;5");
  tprintf("Blink ");
  sgr("0");
  tprintf("NORMAL text ");

  decsasd(0);
  holdit();

  restore_status();
  holdit();

  restore_status();
  return MENU_NOHOLD;
}

static int
CUP_statusline(MENU_ARGS)
{
  static struct {
    const char *codes;
    const char *value;
  } table[] = {
    {
      "0;1", "First "
    },
    {
      "0;4", "Second "
    },
    {
      "0;7", "Third "
    },
    {
      "0;5", "Fourth "
    },
    {
      "0", "Last word"
    },
  };
  size_t last;

  vt_move(1, 1);
  println("This test demonstrates cursor-movement in the status-line");
  holdit();

  decssdt(2);
  decsasd(1);

  /*
   * Write the words in reverse-order, using cursor-addressing.
   * We could also do something with backspacing, insert/delete, but this
   * suffices to illustrate.
   */
  el(2);
  for (last = TABLESIZE(table); last != 0; --last) {
    size_t j;
    int cols = 0;
    for (j = 0; j + 1 < last; ++j) {
      cols += (int) strlen(table[j].value);
    }
    hpa(1 + cols);
    sgr(table[last - 1].codes);
    tprintf("%s", table[last - 1].value);
  }

  decsasd(0);
  holdit();

  restore_status();
  holdit();

  restore_status();
  return MENU_NOHOLD;
}

/* VT200 & up
 *
 *  CSI Ps $ }     DECSASD         Select active status display
 *         Ps = 0 select main display
 *         Ps = 1 select status line
 *         Moves cursor to selected display area.  This command will be ignored
 *         unless the status line has been enabled by CSI 2 $ ~.  When the
 *         status line has been selected cursor remains there until the main
 *         display is reselected by CSI 0 $ }.
 *
 *  CSI Ps $ ~     DECSSDT         Select Status Line Type
 *                         Ps      meaning
 *                         0       no status line (empty)
 *                         1       indicator line
 *                         2       host-writable line
 */
int
tst_statusline(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Simple Status line Test",                           simple_statusline },
      { "Test Graphic-Rendition in Status line",             SGR_statusline },
      { "Test Cursor-movement in Status line",               CUP_statusline },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), println(the_title));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  decssdt(0);
  return MENU_NOHOLD;
}

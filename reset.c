/* $Id: reset.c,v 1.10 2024/12/05 00:37:39 tom Exp $ */

#include <vttest.h>
#include <esc.h>

static int did_reset = FALSE;

int
tst_DECSTR(MENU_ARGS)
{
  vt_move(1, 1);
  println(the_title);
  println("(VT220 & up)");
  println("");
  println("The terminal will now soft-reset");
  holdit();
  decstr();
  return MENU_HOLD;
}

static int
tst_DECTST(MENU_ARGS)
{
  vt_move(1, 1);
  println(the_title);
  println("");

  if (did_reset)
    println("The terminal is now RESET.  Next, the built-in confidence test");
  else
    printxx("The built-in confidence test ");
  printxx("will be invoked. ");
  holdit();

  vt_clear(2);
  dectst(1);
  zleep(5000);  /* Wait 5.0 seconds */
  vt_move(10, 1);
  println("If the built-in confidence test found any errors, a code");
  printxx("%s", "is visible above. ");

  did_reset = FALSE;
  return MENU_HOLD;
}

static int
tst_RIS(MENU_ARGS)
{
  vt_move(1, 1);
  println(the_title);
  println("(VT100 & up, not recommended)");
  println("");
  printxx("The terminal will now be RESET. ");
  holdit();
  ris();
  zleep(5000);  /* Wait 5.0 seconds */

  did_reset = TRUE;
  reset_level();
  input_8bits = FALSE;
  output_8bits = FALSE;
  parse_7bits = FALSE;
  return MENU_HOLD;
}

int
tst_rst(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              NULL },
      { "Reset to Initial State (RIS)",                      tst_RIS },
      { "Invoke Terminal Test (DECTST)",                     tst_DECTST },
      { "Soft Terminal Reset (DECSTR)",                      tst_DECSTR },
      { "",                                                  NULL }
    };
  /* *INDENT-ON* */

  did_reset = FALSE;

  do {
    vt_clear(2);
    __(title(0), printxx("%s", the_title));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

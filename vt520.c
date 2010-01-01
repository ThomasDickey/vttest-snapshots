/* $Id: vt520.c,v 1.2 2010/01/01 16:20:10 tom Exp $ */

/*
 * Reference:  VT520/VT525 Video Terminal Programmer Information
 *             (EK-VT520-RM.A01)
 */
#include <vttest.h>
#include <draw.h>
#include <esc.h>
#include <ttymodes.h>

/******************************************************************************/

static void
decscusr(int parm)
{
  do_csi("%d q", parm);
}

/*
 * VT500 & up
 *
 * Test if the terminal can change the cursor's appearance
 */
static int
tst_DECSCUSR(MENU_ARGS)
{
  vt_move(1, 1);
  decscusr(1);
  println("The cursor should be a blinking rectangle");
  holdit();
  decscusr(0);
  println("The cursor should be a blinking rectangle");
  holdit();
  decscusr(2);
  println("The cursor should be a nonblinking rectangle");
  holdit();
  decscusr(3);
  println("The cursor should be a blinking underline");
  holdit();
  decscusr(4);
  println("The cursor should be a nonblinking underline");
  holdit();
  decscusr(2);
  println("The cursor should be a rectangle again");
  return MENU_HOLD;
}

static int
tst_vt520_screen(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Set Cursor Style (DECSCUSR)",                  tst_DECSCUSR },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT520 Screen-Display Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * These apply only to VT500's & above.
 */
int
tst_vt520(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT420 features",                               tst_vt420 },
      { "Test cursor-movement",                              not_impl },
      { "Test editing sequences",                            not_impl },
      { "Test keyboard-control",                             not_impl },
      { "Test reporting functions",                          not_impl },
      { "Test screen-display functions",                     tst_vt520_screen },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT520 Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

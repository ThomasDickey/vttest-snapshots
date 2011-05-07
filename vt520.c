/* $Id: vt520.c,v 1.3 2011/05/07 12:23:15 tom Exp $ */

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

/******************************************************************************/

static int
rpt_DECCRTST(MENU_ARGS)
{
  return any_decrqss(the_title, "-q");
}

static int
rpt_DECDLDA(MENU_ARGS)
{
  return any_decrqss(the_title, ",z");
}

static int
rpt_DECSEST(MENU_ARGS)
{
  return any_decrqss(the_title, "-z");
}

static int
rpt_DECSCP(MENU_ARGS)
{
  return any_decrqss(the_title, "*u");
}

static int
rpt_DECSCS(MENU_ARGS)
{
  return any_decrqss(the_title, "*r");
}

static int
rpt_DECSDPT(MENU_ARGS)
{
  return any_decrqss(the_title, "(p");
}

static int
rpt_DECSDDT(MENU_ARGS)
{
  return any_decrqss(the_title, "$p");
}

static int
rpt_DECSFC(MENU_ARGS)
{
  return any_decrqss(the_title, "*s");
}

static int
rpt_DECSPRTT(MENU_ARGS)
{
  return any_decrqss(the_title, "$s");
}

static int
rpt_DECSPPCS(MENU_ARGS)
{
  return any_decrqss(the_title, "*p");
}

static int
rpt_DECSSL(MENU_ARGS)
{
  return any_decrqss(the_title, "p");
}

static int
rpt_DECSPMA(MENU_ARGS)
{
  return any_decrqss(the_title, ",x");
}

static int
rpt_DECSPP(MENU_ARGS)
{
  return any_decrqss(the_title, "+w");
}

static int
rpt_DECSSCLS(MENU_ARGS)
{
  return any_decrqss(the_title, " p");
}

static int
rpt_DECSTRL(MENU_ARGS)
{
  return any_decrqss(the_title, "\"u");
}

/*
 * This list is separated from the main one, to keep the menu size fitting on
 * a 24x80 screen.
 */
static int
tst_VT520_hardware_DECRQSS(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT420 features",                               tst_vt420_DECRQSS },
      { "CRT Saver Timing",                                  rpt_DECCRTST },
      { "Down Line Load Allocation",                         rpt_DECDLDA },
      { "Energy Save Timing",                                rpt_DECSEST },
      { "Select Communication Port",                         rpt_DECSCP },
      { "Select Communication Speed",                        rpt_DECSCS },
      { "Select Digital Printed Data Type",                  rpt_DECSDPT },
      { "Select Disconnect Delay Time",                      rpt_DECSDDT },
      { "Select Flow Control Type",                          rpt_DECSFC },
      { "Select Printer Type",                               rpt_DECSPRTT },
      { "Select ProPrinter Character Set",                   rpt_DECSPPCS },
      { "Select Set-Up Language",                            rpt_DECSSL },
      { "Session Page Memory Allocation",                    rpt_DECSPMA },
      { "Set Port Parameter",                                rpt_DECSPP },
      { "Set Scroll Speed",                                  rpt_DECSSCLS },
      { "Set Transmit Rate Limit",                           rpt_DECSTRL },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT520 Status-Strings Reports (Hardware-oriented)"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
rpt_DECATR(MENU_ARGS)
{
  return any_decrqss(the_title, ",}");
}

static int
rpt_DECAC(MENU_ARGS)
{
  return any_decrqss(the_title, ",|");
}

static int
rpt_DECARR(MENU_ARGS)
{
  return any_decrqss(the_title, "-p");
}

static int
rpt_DECSTGLT(MENU_ARGS)
{
  return any_decrqss(the_title, "){");
}

static int
rpt_DECSZS(MENU_ARGS)
{
  return any_decrqss(the_title, ",{");
}

static int
rpt_DECSCUSR(MENU_ARGS)
{
  return any_decrqss(the_title, " p");
}

static int
rpt_DECSKCV(MENU_ARGS)
{
  return any_decrqss(the_title, " r");
}

static int
rpt_DECSLCK(MENU_ARGS)
{
  return any_decrqss(the_title, " v");
}

static int
rpt_DECSMBV(MENU_ARGS)
{
  return any_decrqss(the_title, " u");
}

static int
rpt_DECSWBV(MENU_ARGS)
{
  return any_decrqss(the_title, " t");
}

static int
rpt_DECTME(MENU_ARGS)
{
  return any_decrqss(the_title, " ~");
}

static int
tst_VT520_DECRQSS(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT420 features",                               tst_vt420_DECRQSS },
      { "Test Hardware-oriented features",                   tst_VT520_hardware_DECRQSS },
      { "Alternate Text Color",                              rpt_DECATR },
      { "Assign Color",                                      rpt_DECAC },
      { "Select Auto Repeat Rate",                           rpt_DECARR },
      { "Select Color Lookup Table",                         rpt_DECSTGLT },
      { "Select Zero Symbol",                                rpt_DECSZS },
      { "Set Cursor Style",                                  rpt_DECSCUSR },
      { "Set Key Click Volume",                              rpt_DECSKCV },
      { "Set Lock Key Style",                                rpt_DECSLCK },
      { "Set Margin Bell Volume",                            rpt_DECSMBV },
      { "Set Warning Bell Volume",                           rpt_DECSWBV },
      { "Terminal Mode Emulation",                           rpt_DECTME },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT520 Status-Strings Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
tst_VT520_report_presentation(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT420 features",                               tst_vt420_report_presentation },
      { "Request Mode (DECRQM)/Report Mode (DECRPM)",        tst_DECRPM },
      { "Status-String Report (DECRQSS)",                    tst_VT520_DECRQSS },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  int old_DECRPM = set_DECRPM(5);

  do {
    vt_clear(2);
    __(title(0), printf("VT520 Presentation State Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  set_DECRPM(old_DECRPM);
  return MENU_NOHOLD;
}

int
tst_vt520_reports(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT420 features",                               tst_vt420_reports },
      { "Test Presentation State Reports",                   tst_VT520_report_presentation },
      { "Test Device Status Reports (DSR)",                  tst_vt420_device_status },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT520 Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

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
      { "Test reporting functions",                          tst_vt520_reports },
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

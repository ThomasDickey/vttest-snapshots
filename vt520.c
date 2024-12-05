/* $Id: vt520.c,v 1.28 2024/12/05 00:43:14 tom Exp $ */

/*
 * Reference:  VT520/VT525 Video Terminal Programmer Information
 *             (EK-VT520-RM.A01)
 */
#include <vttest.h>
#include <draw.h>
#include <esc.h>
#include <ttymodes.h>

/******************************************************************************/

static struct {
  int code;
  const char *text;
} tbl_decscusr[] = {

  {
    1, "The cursor should be a blinking rectangle"
  },
  {
    0, "The cursor should be a blinking rectangle"
  },
  {
    2, "The cursor should be a nonblinking rectangle"
  },
  {
    3, "The cursor should be a blinking underline"
  },
  {
    4, "The cursor should be a nonblinking underline"
  },
  {
    5, "xterm: The cursor should be a blinking vertical bar"
  },
  {
    6, "xterm: The cursor should be a nonblinking vertical bar"
  },
  {
    2, "The cursor should be a rectangle again"
  }
};

/******************************************************************************/
static int
tst_vt520_cursor(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              NULL },
      { "Test VT420 features",                               tst_vt420_cursor },
      { origin_mode_mesg,                                    toggle_DECOM },
      { lrmm_mesg,                                           toggle_LRMM },
      { tb_marg_mesg,                                        toggle_STBM },
      { lr_marg_mesg,                                        toggle_SLRM },
      { txt_override_color,                                  toggle_color_mode, },
      { "Test Character-Position-Absolute (HPA)",            tst_HPA },
      { "Test Cursor-Back-Tab (CBT)",                        tst_CBT },
      { "Test Cursor-Character-Absolute (CHA)",              tst_CHA },
      { "Test Cursor-Horizontal-Index (CHT)",                tst_CHT },
      { "Test Horizontal-Position-Relative (HPR)",           tst_HPR },
      { "Test Line-Position-Absolute (VPA)",                 tst_VPA },
      { "Test Next-Line (CNL)",                              tst_CNL },
      { "Test Previous-Line (CPL)",                          tst_CPL },
      { "Test Vertical-Position-Relative (VPR)",             tst_VPR },
      { "",                                                  NULL }
    };
  /* *INDENT-ON* */

  setup_vt420_cursor(PASS_ARGS);

  do {
    vt_clear(2);
    __(title(0), printxx("VT520 Cursor-Movement"));
    __(title(2), println("Choose test type:"));
    menus_vt420_cursor();
  } while (menu(my_menu));

  finish_vt420_cursor(PASS_ARGS);

  return MENU_NOHOLD;
}

/******************************************************************************/
/*
 * VT500 & up
 *
 * Test if terminal can control whether the screen is cleared when changing
 * DECCOLM.
 */
static int
tst_DECNCSM(MENU_ARGS)
{
  int last = max_lines - 4;
  char temp[80];

  decaln();
  deccolm(FALSE);
  vt_move(last, 1);
  ruler(last, min_cols);
  vt_clear(0);
  sprintf(temp, "Screen should be cleared (%d-columns)", min_cols);
  println(temp);
  holdit();

  decaln();
  deccolm(TRUE);
  vt_move(last, 1);
  ruler(last, max_cols);
  vt_clear(0);
  sprintf(temp, "Screen should be cleared (%d-columns)", max_cols);
  println(temp);
  holdit();

  decncsm(TRUE);

  decaln();
  deccolm(FALSE);
  vt_move(last, 1);
  ruler(last, min_cols);
  vt_clear(0);
  sprintf(temp, "Screen should be filled (%d-columns)", min_cols);
  println(temp);
  holdit();

  decaln();
  deccolm(TRUE);
  vt_move(last, 1);
  ruler(last, max_cols);
  vt_clear(0);
  sprintf(temp, "Screen should be filled (%d of %d-columns)", min_cols, max_cols);
  println(temp);
  holdit();

  decncsm(FALSE);
  deccolm(FALSE);
  vt_move(last, 1);
  ruler(last, min_cols);
  vt_clear(0);
  sprintf(temp, "Screen should be cleared (%d-columns)", min_cols);
  println(temp);

  return MENU_HOLD;
}

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
  size_t n;

  vt_move(1, 1);
  for (n = 0; n < TABLESIZE(tbl_decscusr); ++n) {
    if (n != 0)
      holdit();
    decscusr(tbl_decscusr[n].code);
    println(tbl_decscusr[n].text);
  }
  return MENU_HOLD;
}

/******************************************************************************/

#define A_BOLD      (1 << 1)
#define A_UNDERLINE (1 << 4)
#define A_BLINK     (1 << 5)
#define A_REVERSE   (1 << 7)

/*
 * VT525 display alternate text colors
 */
static int
tst_DECATC(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static struct {
    int attr;
  } table[] = {
    { 0, },
    { A_BOLD, },
    {          A_REVERSE, },
    {                      A_UNDERLINE, },
    {                                    A_BLINK, },
    { A_BOLD | A_REVERSE, },
    { A_BOLD |             A_UNDERLINE, },
    { A_BOLD |                           A_BLINK, },
    {          A_REVERSE | A_UNDERLINE, },
    {          A_REVERSE |               A_BLINK, },
    {                      A_UNDERLINE | A_BLINK, },
    { A_BOLD | A_REVERSE | A_UNDERLINE, },
    { A_BOLD | A_REVERSE |               A_BLINK, },
    { A_BOLD |             A_UNDERLINE | A_BLINK, },
    {          A_REVERSE | A_UNDERLINE | A_BLINK, },
    { A_BOLD | A_REVERSE | A_UNDERLINE | A_BLINK, },
  };
  /* *INDENT-ON* */
  size_t n;

  vt_move(1, 1);
  println(the_title);
  /* this test assumes that an application has used DECATC, etc., to set up
   * suitable colors, and simply displays what has been set up.
   */
  for (n = 0; n < TABLESIZE(table); ++n) {
    char buffer[80];

    vt_move((int) n + 3, 10);
    decstglt(1);
    printxx("%2d ", (int) n);
    if (table[n].attr & A_BOLD)
      sgr("1");
    if (table[n].attr & A_REVERSE)
      sgr("7");
    if (table[n].attr & A_UNDERLINE)
      sgr("4");
    if (table[n].attr & A_BLINK)
      sgr("5");
    printxx(" Testing ");
    sgr("0");
    decstglt(3);
    *buffer = '\0';
    if (table[n].attr & A_BOLD)
      strcat(buffer, " bold");
    if (table[n].attr & A_REVERSE)
      strcat(buffer, " reverse");
    if (table[n].attr & A_UNDERLINE)
      strcat(buffer, " underline");
    if (table[n].attr & A_BLINK)
      strcat(buffer, " blink");
    if (buffer[0] == 0)
      strcpy(buffer, " normal text");
    buffer[1] = (char) toupper(CharOf(buffer[1]));
    printxx("%s", buffer);
  }
  vt_move(max_lines - 1, 1);
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
  return any_decrqss(the_title, "-r");
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
  return any_decrqss(the_title, ")p");
}

static int
rpt_DECSDDT(MENU_ARGS)
{
  return any_decrqss(the_title, "$q");
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
rpt_DECSRFR(MENU_ARGS)
{
  return any_decrqss(the_title, "\"t");
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

/******************************************************************************/

static int
rpt_DECATC(MENU_ARGS)
{
  static const char *attributes[] =
  {
    "normal",
    "bold",
    "reverse",
    "underline",
    "blink",
    "bold reverse",
    "bold underline",
    "bold blink",
    "reverse underline",
    "reverse blink",
    "underline blink",
    "bold reverse underline",
    "bold reverse blink",
    "bold underline blink",
    "reverse underline blink",
    "bold reverse underline blink"
  };

  int ps1;
  int row, col;
  const char *show;
  const char *suffix = ",}";

  vt_move(1, 1);
  printxx("Testing DECRQSS: %s\n", the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  reset_decstbm();
  reset_decslrm();

  row = 3;
  col = 10;

  for (ps1 = 0; ps1 < 16; ++ps1) {
    char func[80];
    char *report;
    int fail = 1;
    int qps, qfg, qbg;

    sprintf(func, "%d%s", ps1, suffix);

    decrqss(func);
    report = get_reply();

    vt_move(row, col);
    chrprint2(report, row++, col);

    switch (parse_decrqss(report, suffix)) {
    case 1:
      show = "ok (valid request)";
      if (sscanf(report, "%d;%d;%d", &qps, &qfg, &qbg) == 3
          && qps == ps1
          && qfg >= 0 && qfg < 16
          && qbg >= 0 && qbg < 16) {
        fail = 0;
        show = attributes[ps1];
      }
      break;
    case 0:
      show = "invalid request";
      break;
    default:
      show = SHOW_FAILURE;
      break;
    }
    show_result("%s", show);
    if (fail)
      break;
  }

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

static int
rpt_DECAC(MENU_ARGS)
{
  static const char *attributes[] =
  {
    "normal text",
    "window frame",
  };

  int ps1;
  int row, col;
  const char *show;
  const char *suffix = ",|";

  vt_move(1, 1);
  printxx("Testing DECRQSS: %s\n", the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  reset_decstbm();
  reset_decslrm();

  row = 3;
  col = 10;

  for (ps1 = 1; ps1 <= 2; ++ps1) {
    char func[80];
    char *report;
    int fail = 1;
    int qps, qfg, qbg;

    sprintf(func, "%d%s", ps1, suffix);

    decrqss(func);
    report = get_reply();

    vt_move(row, col);
    chrprint2(report, row++, col);

    switch (parse_decrqss(report, suffix)) {
    case 1:
      show = "ok (valid request)";
      if (sscanf(report, "%d;%d;%d", &qps, &qfg, &qbg) == 3
          && qps == ps1
          && qfg >= 0 && qfg < 16
          && qbg >= 0 && qbg < 16) {
        fail = 0;
        show = attributes[ps1 - 1];
      }
      break;
    case 0:
      show = "invalid request";
      break;
    default:
      show = SHOW_FAILURE;
      break;
    }
    show_result("%s", show);
    if (fail)
      break;
  }

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
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
  size_t n;

  vt_move(1, 1);
  for (n = 0; n < TABLESIZE(tbl_decscusr); ++n) {
    char expected[10];
    if (n != 0)
      holdit();
    vt_clear(2);
    decscusr(tbl_decscusr[n].code);
    sprintf(expected, "%d", tbl_decscusr[n].code ? tbl_decscusr[n].code : 1);
    (void) any_decrqss2(tbl_decscusr[n].text, " q", expected);
  }
  return MENU_HOLD;
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
rpt_DECUS(MENU_ARGS)
{
  return any_decrqss(the_title, ",y");
}

static int
tst_VT510_DECRQSS(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              NULL },
      { "Test VT420 features (DECRQSS)",                     tst_vt420_DECRQSS },
      { "Select Communication Port (DECSCP)",                rpt_DECSCP },
      { "Select Communication Speed (DECSCS)",               rpt_DECSCS },
      { "Select Digital Printed Data Type (DECSDPT)",        rpt_DECSDPT },
      { "Select Disconnect Delay Time (DECSDDT)",            rpt_DECSDDT },
      { "Select Flow Control Type (DECSFC)",                 rpt_DECSFC },
      { "Select Printer Type (DECSPRTT)",                    rpt_DECSPRTT },
      { "Select ProPrinter Character Set (DECSPPCS)",        rpt_DECSPPCS },
      { "Select Refresh Rate Selection (DECSRFR)",           rpt_DECSRFR },
      { "Select Set-Up Language (DECSSL)",                   rpt_DECSSL },
      { "Set Cursor Style (DECSCUSR)",                       rpt_DECSCUSR },
      { "Set Key Click Volume (DECSKCV)",                    rpt_DECSKCV },
      { "Set Lock Key Style (DECSLCK)",                      rpt_DECSLCK },
      { "Set Margin Bell Volume (DECSMBV)",                  rpt_DECSMBV },
      { "Set Port Parameter (DECSPP)",                       rpt_DECSPP },
      { "Set Scroll Speed (DECSSCLS)",                       rpt_DECSSCLS },
      { "Set Transmit Rate Limit (DECSTRL)",                 rpt_DECSTRL },
      { "Set Warning Bell Volume (DECSWBV)",                 rpt_DECSWBV },
      { "",                                                  NULL }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printxx("VT510 Status-Strings Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

int
tst_vt520_DECRQSS(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              NULL },
      { "Test VT510 features (DECRQSS)",                     tst_VT510_DECRQSS },
      { "Alternate Text Color (DECATC)",                     rpt_DECATC },
      { "Assign Color (DECAC)",                              rpt_DECAC },
      { "CRT Saver Timing (DECCRTST)",                       rpt_DECCRTST },
      { "Down Line Load Allocation (DECDLDA)",               rpt_DECDLDA },
      { "Energy Save Timing (DECSEST)",                      rpt_DECSEST },
      { "Select Auto Repeat Rate (DECARR)",                  rpt_DECARR },
      { "Select Color Lookup Table (DECSTGLT)",              rpt_DECSTGLT },
      { "Select Zero Symbol (DECSZS)",                       rpt_DECSZS },
      { "Session Page Memory Allocation (DECSPMA)",          rpt_DECSPMA },
      { "Terminal Mode Emulation (DECTME)",                  rpt_DECTME },
      { "Update Session (DECUS)",                            rpt_DECUS },
      { "",                                                  NULL }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printxx("VT520 Status-Strings Reports"));
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
      { "Exit",                                              NULL },
      { "Test VT420 features",                               tst_vt420_report_presentation },
      { "Request Mode (DECRQM)/Report Mode (DECRPM)",        tst_DECRPM },
      { "Status-String Report (DECRQSS)",                    tst_vt520_DECRQSS },
      { "",                                                  NULL }
    };
  /* *INDENT-ON* */

  int old_DECRPM = set_DECRPM(5);

  do {
    vt_clear(2);
    __(title(0), printxx("VT520 Presentation State Reports"));
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
      { "Exit",                                              NULL },
      { "Test VT420 features",                               tst_vt420_reports },
      { "Test Presentation State Reports",                   tst_VT520_report_presentation },
      { "Test Device Status Reports (DSR)",                  tst_vt420_device_status },
      { "",                                                  NULL }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printxx("VT520 Reports"));
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
      { "Exit",                                              NULL },
      { "Test No Clear on Column Change (DECNCSM)",          tst_DECNCSM },
      { "Test Set Cursor Style (DECSCUSR)",                  tst_DECSCUSR },
      { "Test Alternate Text Color (DECATC)",                tst_DECATC },
      { "",                                                  NULL }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printxx("VT520 Screen-Display Tests"));
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
      { "Exit",                                              NULL },
      { "Test VT420 features",                               tst_vt420 },
      { "Test cursor-movement",                              tst_vt520_cursor },
      { "Test editing sequences",                            not_impl },
      { "Test keyboard-control",                             not_impl },
      { "Test reporting functions",                          tst_vt520_reports },
      { "Test screen-display functions",                     tst_vt520_screen },
      { "",                                                  NULL }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printxx("VT520 Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

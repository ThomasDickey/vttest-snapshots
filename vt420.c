/* $Id: vt420.c,v 1.111 2012/04/15 22:28:13 tom Exp $ */

/*
 * Reference:  Installing and Using the VT420 Video Terminal (North American
 *             Model (EK-VT420-UG.002)
 */
#include <vttest.h>
#include <draw.h>
#include <esc.h>
#include <ttymodes.h>

#define WHITE_ON_BLUE   "0;37;44"
#define WHITE_ON_GREEN  "0;37;42"
#define YELLOW_ON_BLACK "0;33;40"
#define BLINK_REVERSE   "0;5;7"

typedef struct {
  int mode;
  char *name;
} MODES;

typedef enum {
  marNone = -1,
  marReset = 0,
  marFirst = 1,
  marLast = 2,
  marMiddle = 3,
  marEnd
} MARS;

static int do_lines = FALSE;
static int do_colors = FALSE;

/******************************************************************************/

static int lrmm_flag;

static MARS lr_marg_flag;
static int lr_marg1, lr_marg2;
static MARS tb_marg_flag;
static int tb_marg1, tb_marg2;

static char lrmm_mesg[80];
static char lr_marg_mesg[80];
static char tb_marg_mesg[80];

static int
toggle_LRMM(MENU_ARGS)
{
  lrmm_flag = !lrmm_flag;
  if (lrmm_flag)
    sm("?69");
  else
    rm("?69");
  return MENU_NOHOLD;
}

/*
 * The message tells what margins will be used in the test, not what their
 * value is while drawing the menu (since actually setting margins would
 * interfere with the menu).
 */
static int
toggle_STBM(MENU_ARGS)
{
  switch (++tb_marg_flag) {
  default:
    tb_marg_flag = marReset;
    tb_marg1 = 0;
    tb_marg2 = 0;
    strcpy(tb_marg_mesg, "Top/Bottom margins are reset");
    break;
  case marFirst:
    tb_marg1 = 1;
    tb_marg2 = max_lines / 2;
    strcpy(tb_marg_mesg, "Top/Bottom margins are set to top half of screen");
    break;
  case marLast:
    tb_marg1 = max_lines / 2;
    tb_marg2 = max_lines;
    strcpy(tb_marg_mesg,
           "Top/Bottom margins are set to bottom half of screen");
    break;
  case marMiddle:
    tb_marg1 = max_lines / 4;
    tb_marg2 = (3 * max_lines) / 4;
    strcpy(tb_marg_mesg,
           "Top/Bottom margins are set to middle half of screen");
    break;
  }
  return MENU_NOHOLD;
}

static int
toggle_SLRM(MENU_ARGS)
{
  switch (++lr_marg_flag) {
  default:
    lr_marg_flag = marReset;
    lr_marg1 = 0;
    lr_marg2 = 0;
    strcpy(lr_marg_mesg, "Left/right margins are reset");
    break;
  case marFirst:
    lr_marg1 = 1;
    lr_marg2 = min_cols / 2;
    strcpy(lr_marg_mesg, "Left/right margins are set to left half of screen");
    break;
  case marLast:
    lr_marg1 = min_cols / 2;
    lr_marg2 = min_cols;
    strcpy(lr_marg_mesg,
           "Left/right margins are set to right half of screen");
    break;
  case marMiddle:
    lr_marg1 = min_cols / 4;
    lr_marg2 = (3 * min_cols) / 4;
    strcpy(lr_marg_mesg,
           "Left/right margins are set to middle half of screen");
    break;
  }
  return MENU_NOHOLD;
}

/*
 * Fill area outside margins with given character, to help show that changes
 * are limited to the area within margins.
 */
static void
fill_outside(int ch)
{
  int row, col;

  for (row = 1; row <= max_lines; ++row) {
    if (row < tb_marg1 ||
        row > tb_marg2 ||
        lr_marg1 > 1 ||
        lr_marg2 < min_cols) {
      int first = 1;
      int next = 0;

      for (col = 1; col <= min_cols; ++col) {
        if ((lrmm_flag && lr_marg1 && col < lr_marg1) ||
            (lrmm_flag && lr_marg2 && col > lr_marg2) ||
            (tb_marg1 != 0 && row < tb_marg1) ||
            (tb_marg2 != 0 && row > tb_marg2)) {
          if (first || (next != col)) {
            cup(row, col);
            first = 0;
            next = col + 1;
          }
          putchar(ch);
          ++next;
        }
      }
    }
  }
}

static void
test_with_margins(int enable)
{
  if (enable) {
    fill_outside('.');
    decstbm(tb_marg1, tb_marg2);
    decslrm(lr_marg1, lr_marg2);
  } else {
    decstbm(0, 0);
    decslrm(0, 0);
  }
}

static void
ruler(int row)
{
  int col;
  vt_move(row, 1);
  for (col = 1; col <= min_cols; ++col) {
    int ch = (((col % 10) == 0)
              ? ('0' + (col / 10) % 10)
              : (((col % 5) == 0)
                 ? '+'
                 : '-'));
    putchar(ch);
  }
  putchar('\n');
}

/*
 * Fill the area within margins with a test pattern.  The top line is numbers,
 * the bottom line is alphas.  In between, use asterisks.
 */
static void
fill_margins(void)
{
  int top = tb_marg1 ? tb_marg1 : 1;
  int bot = tb_marg2 ? tb_marg2 : max_lines;
  int lft = (lrmm_flag && lr_marg1) ? lr_marg1 : 1;
  int rgt = (lrmm_flag && lr_marg2) ? lr_marg2 : min_cols;
  int row, col;

  decawm(FALSE);  /* do this to allow writing in lower-right */
  for (row = top; row <= bot; ++row) {
    cup(row, lft);
    for (col = lft; col <= rgt; ++col) {
      if (row == top) {
        putchar((col - lft) % 10 + '0');
      } else if (row == bot) {
        putchar((col - lft) % 26 + 'a');
      } else {
        putchar('*');
      }
    }
  }
  decawm(TRUE);
}

#define DATA(name,level) { name, #name, level }

static int
show_DECLRMM(MENU_ARGS)
{
  /* *INDENT-OFF* */
  RQM_DATA dec_modes[] = { /* this list is sorted by code, not name */
    DATA( DECLRMM, 4 /* left/right margin mode */),
  };
  /* *INDENT-ON* */

  int code;
  int old_DECRPM = set_DECRPM(4);

  code = any_RQM(PASS_ARGS, dec_modes, TABLESIZE(dec_modes), 1);
  set_DECRPM(old_DECRPM);
  return code;
}

#undef DATA

/*
 * Allow user to test the same screens with/without lines.
 */
static int
toggle_lines_mode(MENU_ARGS)
{
  do_lines = !do_lines;
  return MENU_NOHOLD;
}

/*
 * Allow user to test the same screens with/without colors.
 * VT420's do not support ANSI colors; this is for testing xterm.
 */
static int
toggle_color_mode(MENU_ARGS)
{
  do_colors = !do_colors;
  return MENU_NOHOLD;
}

/*
 * DECALN does not set attributes; we want a colored screen for some tests.
 */
static void
fill_screen(void)
{
  int y, x;

  if (do_colors) {
    sgr(WHITE_ON_BLUE);
    for (y = 0; y < max_lines - 4; ++y) {
      cup(y + 1, 1);
      for (x = 0; x < min_cols; ++x)
        putchar('E');
    }
    /* make this a different color to show fill versus erase diffs */
    sgr(WHITE_ON_GREEN);
  } else {
    decaln();   /* fill the screen */
  }
}

/******************************************************************************/

static int
rpt_DECSACE(MENU_ARGS)
{
  return any_decrqss(the_title, "*x");
}

static int
rpt_DECSNLS(MENU_ARGS)
{
  return any_decrqss(the_title, "*|");
}

static int
rpt_DECSLRM(MENU_ARGS)
{
  return any_decrqss(the_title, "s");
}

static int
rpt_DECELF(MENU_ARGS)
{
  return any_decrqss(the_title, "+q");
}

/*
 * VT420 manual shows "=}", but the terminal returns an error.  VT510 sequences
 * show "*}".
 */
static int
rpt_DECLFKC(MENU_ARGS)
{
  return any_decrqss(the_title, "*}");
}

static int
rpt_DECSMKR(MENU_ARGS)
{
  return any_decrqss(the_title, "+r");
}

/******************************************************************************/

static void
show_DataIntegrity(char *report)
{
  int pos = 0;
  int code = scanto(report, &pos, 'n');
  const char *show;

  switch (code) {
  case 70:
    show = "No communication errors";
    break;
  case 71:
    show = "Communication errors";
    break;
  case 73:
    show = "Not reported since last power-up or RIS";
    break;
  default:
    show = SHOW_FAILURE;
  }
  show_result("%s", show);
}

static void
show_keypress(int row, int col)
{
  char *report;
  char last[BUFSIZ];

  last[0] = '\0';
  vt_move(row++, 1);
  println("When you are done, press any key twice to quit.");
  vt_move(row, col);
  fflush(stdout);
  while (strcmp(report = instr(), last)) {
    vt_move(row, col);
    vt_clear(0);
    chrprint(report);
    strcpy(last, report);
  }
}

static void
show_MultisessionStatus(char *report)
{
  int pos = 0;
  int Ps1 = scan_any(report, &pos, 'n');
  int Ps2 = scanto(report, &pos, 'n');
  const char *show;

  switch (Ps1) {
  case 80:
    show = "SSU sessions enabled (%d max)";
    break;
  case 81:
    show = "SSU sessions available but pending (%d max)";
    break;
  case 83:
    show = "SSU sessions not ready";
    break;
  case 87:
    show = "Sessions on separate lines";
    break;
  default:
    show = SHOW_FAILURE;
  }
  show_result(show, Ps2);
}

/******************************************************************************/

/*
 * VT400 & up.
 * DECBI - Back Index
 * This control function moves the cursor backward one column.  If the cursor
 * is at the left margin, then all screen data within the margin moves one
 * column to the right.  The column that shifted past the right margin is lost.
 *
 * Format:  ESC 6
 * Description:
 * DECBI adds a new column at the left margin with no visual attributes.  DECBI
 * is not affected by the margins.  If the cursor is at the left border of the
 * page when the terminal received DECBI, then the terminal ignores DECBI.
 */
static int
tst_DECBI(MENU_ARGS)
{
  int n, m;
  int last = max_lines - 4;
  int final;
  int top;
  int lft;
  int rgt;

  test_with_margins(1);

  top = tb_marg1 ? tb_marg1 : 1;
  lft = (lrmm_flag && lr_marg1) ? lr_marg1 : 1;
  rgt = (lrmm_flag && lr_marg2) ? lr_marg2 : min_cols;

  final = (rgt - lft + 1) / 4;

  for (n = final; n > 0; n--) {
    cup(top, lft);
    if (n != final) {
      for (m = 0; m < 4; m++)
        decbi();
    }
    printf("%3d", n);
  }
  test_with_margins(0);

  vt_move(last, 1);
  vt_clear(0);

  println(the_title);
  println("If your terminal supports DECBI (backward index), then the top row");
  printf("should be numbered 1 through %d.\n", final);
  return MENU_HOLD;
}

static int
tst_DECBKM(MENU_ARGS)
{
  char *report;

  vt_move(1, 1);
  println(the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  reset_inchar();
  decbkm(TRUE);
  println("Press the backspace key");
  vt_move(3, 10);
  report = instr();
  chrprint(report);
  show_result(!strcmp(report, "\010") ? SHOW_SUCCESS : SHOW_FAILURE);

  reset_inchar();
  vt_move(5, 1);
  decbkm(FALSE);
  println("Press the backspace key again");
  vt_move(6, 10);
  report = instr();
  chrprint(report);
  show_result(!strcmp(report, "\177") ? SHOW_SUCCESS : SHOW_FAILURE);

  vt_move(max_lines - 1, 1);
  restore_ttymodes();
  return MENU_HOLD;
}

/*
 * VT400 & up
 * Change Attributes in Rectangular Area
 */
static int
tst_DECCARA(MENU_ARGS)
{
  int last = max_lines - 4;
  int top = 5;
  int left = 5;
  int right = 45;
  int bottom = max_lines - 10;

  if (do_colors)
    sgr(WHITE_ON_BLUE);

  decsace(TRUE);
  fill_screen();
  deccara(top, left, bottom, right, 7);   /* invert a rectangle) */
  deccara(top + 1, left + 1, bottom - 1, right - 1, 0);   /* invert a rectangle) */

  sgr("0");
  vt_move(last, 1);
  vt_clear(0);

  println(the_title);
  println("There should be an open rectangle formed by reverse-video E's");
  holdit();

  decsace(FALSE);
  fill_screen();
  deccara(top, left, bottom, right, 7);   /* invert a rectangle) */
  deccara(top + 1, left + 1, bottom - 1, right - 1, 0);   /* invert a rectangle) */

  sgr("0");
  vt_move(last, 1);
  vt_clear(0);

  println(the_title);
  println("There should be an open rectangle formed by reverse-video E's");
  println("combined with wrapping at the margins.");
  return MENU_HOLD;
}

static int
tst_DECCKSR(MENU_ARGS, int Pid, const char *the_csi)
{
  char *report;
  int pos = 0;

  vt_move(1, 1);
  printf("Testing DECCKSR: %s\n", the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("%s", the_csi);
  report = get_reply();
  vt_move(3, 10);
  chrprint(report);
  if ((report = skip_dcs(report)) != 0
      && strip_terminator(report)
      && strlen(report) > 1
      && scanto(report, &pos, '!') == Pid
      && report[pos++] == '~'
      && (report = skip_digits(report + pos + 1)) != 0
      && *report == '\0') {
    show_result(SHOW_SUCCESS);
  } else {
    show_result(SHOW_FAILURE);
  }

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

/*
 * VT400 & up.
 * Copy Rectangular area
 */
static int
tst_DECCRA(MENU_ARGS)
{
#define adj_y 3
#define adj_x 4
#define adj_DECCRA " (down %d, right %d)\r\n", box.bottom + 1 - box.top, box.right + 1 - box.left, adj_y, adj_x
#define msg_DECCRA(msg) "The %dx%d box " msg adj_DECCRA
  BOX box;

  if (make_box_params(&box, 10, 30) == 0) {
    box.top = 5;
    box.left = 5;

    if (do_colors) {
      sgr(WHITE_ON_BLUE);
    } else {
      sgr(BLINK_REVERSE);
    }
    draw_box_outline(&box, do_lines ? -1 : '*');
    sgr("0");

    vt_move(max_lines - 3, 1);
    println(the_title);
    tprintf(msg_DECCRA("will be copied"));
    holdit();

    deccra(box.top, box.left, box.bottom, box.right, 1,
           box.top + adj_y, box.left + adj_x, 1);

    vt_move(max_lines - 2, 1);
    vt_clear(0);

    tprintf(msg_DECCRA("should be copied, overlapping"));
    holdit();

    ed(2);

    make_box_params(&box, 10, 30);
    box.top = 5;
    box.left = 5;

    if (do_colors) {
      sgr(YELLOW_ON_BLACK);
    } else {
      sgr("0;7");   /* fill the box in reverse */
    }
    draw_box_filled(&box, -1);

    if (do_colors) {
      sgr(WHITE_ON_BLUE);
    } else {
      sgr(BLINK_REVERSE);
    }
    draw_box_outline(&box, do_lines ? -1 : '*');
    sgr("0");

    vt_move(max_lines - 3, 1);
    println(the_title);
    tprintf(msg_DECCRA("will be copied"));
    holdit();

    sgr("0;4"); /* set underline, to check if that leaks through */
    deccra(box.top, box.left, box.bottom, box.right, 1,
           box.top + adj_y, box.left + adj_x, 1);
    sgr("0");

    vt_move(max_lines - 2, 1);
    vt_clear(0);

    tprintf(msg_DECCRA("should be copied, overlapping"));
  }
  return MENU_HOLD;
}

static int
marker_of(int n)
{
  return (n - 1) % 26 + 'a';
}

/*
 * VT400 & up.
 * Delete column.
 */
static int
tst_DECDC(MENU_ARGS)
{
  int n;
  int last = max_lines - 3;
  int base_row;
  int base_col;
  int left_col;
  int top;
  int bot;
  int lft;
  int final_dc;
  char mark_1st = 0;
  char mark_2nd = 0;

  test_with_margins(1);
  top = tb_marg1 ? tb_marg1 : 1;
  bot = tb_marg2 ? tb_marg2 : (last - 1);
  lft = (lrmm_flag && lr_marg1) ? lr_marg1 : 1;

  /*
   * Adjustments so that most of the initial line (before shifting) passes
   * through the area within margins.
   */
  switch (lr_marg_flag) {
  default:
    base_col = (2 * last);
    left_col = 1;
    break;
  case marFirst:
    base_col = (min_cols / 2);
    left_col = 1;
    break;
  case marMiddle:
    base_col = (3 * min_cols) / 4;
    left_col = min_cols / 4;
    break;
  case marLast:
    base_col = min_cols + 0;
    left_col = min_cols / 2;
    break;
  }

  final_dc = base_col - 1;

  if (tb_marg_flag == marLast) {
    base_row = max_lines / 2;
  } else {
    base_row = 0;
  }

  for (n = 1; n < last; n++) {
    int row = base_row + n;
    int col = base_col - n;

    if (row <= last) {
      int mark = marker_of(n);

      if (row >= top && row <= bot && row < last) {
        mark_2nd = (char) mark;
        if (mark_1st == 0) {
          mark_1st = (char) mark;
        }
      }

      __(cup(row, col), putchar(mark));
      if (top > 1 || (lrmm_flag && lft > 1)) {
        __(cup(1, 1), decdc(1));  /* outside margins, should be ignored */
        __(cup(row, col), putchar(mark));
      }
      if (final_dc-- > 0)
        __(cup(top, lft), decdc(1));
    }
  }
  if (final_dc > left_col)
    __(cup(top, lft), decdc(final_dc - left_col));
  test_with_margins(0);

  ruler(last);
  vt_move(last + 1, 1);
  vt_clear(0);

  if (lrmm_flag)
    tprintf("If your terminal supports DECDC, letters %c-%c are on column %d\n",
            mark_1st, mark_2nd, lft);
  else
    println("There should be a diagonal of letters from left near bottom to middle at top");
  return MENU_HOLD;
}

/*
 * VT400 & up
 * Erase Rectangular area
 */
static int
tst_DECERA(MENU_ARGS)
{
  fill_screen();
  decera(5, 5, max_lines - 10, min_cols - 5);

  sgr("0");
  vt_move(max_lines - 3, 1);
  vt_clear(0);

  println(the_title);
  println("There should be a rectangle cleared in the middle of the screen.");
  return MENU_HOLD;
}

/*
 * This is two tests:  IND (index) and RI (reverse index).  For each test, we
 * start by filling the area inside (including) the margins with a test
 * pattern, and then after the user presses "return", update the screen so that
 * only one line of the test-pattern should remain visible.
 */
static int
tst_IND_RI(MENU_ARGS)
{
  int hold_row, hold_col;       /* where to put "Push RETURN" */
  int row;
  int top = tb_marg1 ? tb_marg1 : 1;
  int bot = tb_marg2 ? tb_marg2 : max_lines;
  int lft = (lrmm_flag && lr_marg1) ? lr_marg1 : 1;
  int rgt = (lrmm_flag && lr_marg2) ? lr_marg2 : min_cols;

  switch (tb_marg_flag) {
  default:
    hold_row = max_lines / 2;
    break;
  case marFirst:
    hold_row = bot + 1;
    break;
  case marLast:
    hold_row = 1;
    break;
  }

  hold_col = 1;
  if (lrmm_flag) {
    switch (lr_marg_flag) {
    default:
      break;
    case marFirst:
      hold_col = rgt + 1;
      break;
    case marMiddle:
      hold_col = lft;
      break;
    }
  }

  test_with_margins(1);

  fill_margins();

  vt_move(hold_row, hold_col);
  holdit();

  cup(bot, (lft + rgt) / 2);
  for (row = top; row < bot; ++row)
    ind();

  vt_move(hold_row, hold_col);
  printf("\"abcd...\" should be at top. ");
  vt_move(hold_row + 1, hold_col);
  holdit();

  fill_margins();
  fill_outside('.');

  vt_move(hold_row, hold_col);
  holdit();

  cup(top, (lft + rgt) / 2);
  for (row = top; row < bot; ++row)
    ri();

  vt_move(hold_row, hold_col);
  printf("\"0123...\" should be at bottom. ");
  vt_move(hold_row + 1, hold_col);
  holdit();

  test_with_margins(0);

  return MENU_NOHOLD;
}

/*
 * Check to see if ASCII formatting controls (BS, HT, CR) are affected by
 * left/right margins.  Do this by starting after the left-margin, and
 * backspacing "before" the left margin.  Then fill the margins with a usable
 * test pattern.  After that, use tabs to go to the right margin, adding
 * another usable test (+), and use carriage returns to go to the left margin,
 * adding another usable test (-).
 */
static int
tst_ASCII_format(MENU_ARGS)
{
  int last = max_lines - 4;
  int top;
  int bot;
  int lft;
  int rgt;
  int n;
  int tab;
  int size;

  test_with_margins(1);

  top = tb_marg1 ? tb_marg1 : 1;
  bot = tb_marg2 ? tb_marg2 : last - 1;
  lft = (lrmm_flag && lr_marg1) ? lr_marg1 : 1;
  rgt = (lrmm_flag && lr_marg2) ? lr_marg2 : min_cols;

  /*
   * This should stop at the left margin, and the result overwritten by a
   * fill-pattern.
   */
  cup(top, rgt);
  for (n = 0; n < rgt; ++n) {
    printf("*%c%c", BS, BS);
  }

  /*
   * Fill the margins with a repeating pattern.  Do it twice, to force it to
   * scroll up.
   */
  size = 2 * (rgt - lft + 1) * (bot - top + 1);
  for (n = 0; n < size; ++n) {
    int ch = ((n % 10) ? ((n % 10) + '0') : '_');
    putchar(ch);
  }

  /*
   * Mark the margins with '-' (left) and '+' (right).
   */
  cup(top, lft);
  for (n = top; n <= bot; ++n) {
    putchar('-');
    for (tab = 0; tab < (rgt - lft + 16) / 8; ++tab) {
      putchar(TAB);
    }
    putchar('+');
    if (n < bot) {
      putchar(CR);
      putchar(LF);
    }
  }

  test_with_margins(0);

  vt_move(last, 1);
  vt_clear(0);

  ruler(last);
  println(the_title);
  println("A repeating \"0123456789_\" pattern should fall within the -/+ margins");
  return MENU_HOLD;
}

/*
 * VT400 & up.
 *
 * DECFI - Forward Index
 * This control function moves the column forward one column.  If the cursor is
 * at the right margin, then all screen data within the margins moves one
 * column to the left.  The column shifted past the left margin is lost.
 *
 * Format: ESC 9
 * Description:
 * DECFI adds a new column at the right margin with no visual attributes.
 * DECFI is not affected by the margins.  If the cursor is at the right border
 * of the page when the terminal receives DECFI, then the terminal ignores
 * DECFI.
 */
static int
tst_DECFI(MENU_ARGS)
{
  int n, m;
  int last = max_lines - 4;
  int final;
  int top;
  int lft;
  int rgt;

  test_with_margins(1);

  top = tb_marg1 ? tb_marg1 : 1;
  lft = (lrmm_flag && lr_marg1) ? lr_marg1 : 1;
  rgt = (lrmm_flag && lr_marg2) ? lr_marg2 : min_cols;

  final = (rgt - lft + 1) / 4;

  for (n = 1; n <= final; n++) {
    cup(top, rgt - 3);
    printf("%3d", n);   /* leaves cursor in rightmost column */
    if (n != final) {
      for (m = 0; m < 4; m++)
        decfi();
    }
  }
  test_with_margins(0);

  vt_move(last, 1);
  vt_clear(0);

  println(the_title);
  println("If your terminal supports DECFI (forward index), then the top row");
  printf("should be numbered 1 through %d.\n", final);
  return MENU_HOLD;
}

/*
 * Demonstrate whether cursor movement is limited by margins.  VT420 manual
 * says that CUU/CUD will stop on the margins, but if outside the margins
 * will proceed to the page border.  So we can test this by
 *
 * a) moving to the margin, and cursor up/down toward the border, placing a
 * marker at the end of the cursor movement (to overwrite a prior marker placed
 * explicitly on the border).
 *
 * b) repeat the process, going from the border into the area within margins.
 *
 * c) Even for the no-margins case, this is useful, since it demonstrates
 * whether the cursor forces scrolling.
 */
static int
tst_cursor_margins(MENU_ARGS)
{
  BOX box;
  int last = max_lines - 4;
  int row;
  int col;

  test_with_margins(1);

  box.top = tb_marg1 ? tb_marg1 : 1;
  box.bottom = tb_marg2 ? tb_marg2 : max_lines;
  box.left = (lrmm_flag && lr_marg1 ? lr_marg1 : 1);
  box.right = (lrmm_flag && lr_marg2 ? lr_marg2 : min_cols);
  draw_box_outline(&box, '*');

  for (row = box.top; row <= box.bottom; ++row) {
    cup(row, box.left);
    for (col = box.left; col > 0; col--) {
      cub(1);
    }
    putchar('l');
  }

  for (row = box.top; row <= box.bottom; ++row) {
    cup(row, box.right);
    for (col = box.right; col <= min_cols; col++) {
      cuf(1);
    }
    putchar('r');
  }

  for (col = box.left; col <= box.right; ++col) {
    cup(box.top, col);
    for (row = box.top; row > 0; row--) {
      cuu(1);
    }
    putchar('u');
  }

  for (col = box.left; col <= box.right; ++col) {
    cup(box.bottom, col);
    for (row = box.bottom; row <= max_lines; row++) {
      cud(1);
    }
    putchar('d');
  }

  test_with_margins(0);
  vt_move(last, 1);
  vt_clear(0);

  println(the_title);
  println("A box of *'s should appear on screen border, overwritten for margins (u/d/l/r)");
  return MENU_HOLD;
}

/*
 * VT400 & up
 * Fill Rectangular area
 */
static int
tst_DECFRA(MENU_ARGS)
{
  if (do_colors) {
    sgr(WHITE_ON_BLUE);
    vt_clear(2);
    sgr(WHITE_ON_GREEN);
  }
  decfra('*', 5, 5, max_lines - 10, min_cols - 5);

  vt_move(max_lines - 3, 1);
  if (do_colors) {
    sgr("0");
  }
  vt_clear(0);

  println(the_title);
  println("There should be a rectangle of *'s in the middle of the screen.");
  holdit();

  if (do_colors) {
    sgr(WHITE_ON_BLUE);
  }
  decfra(' ', 5, 5, max_lines - 10, min_cols - 5);
  sgr("0");

  vt_move(max_lines - 3, 1);
  vt_clear(0);

  println(the_title);
  println("The rectangle of *'s should be gone.");
  return MENU_HOLD;
}

/*
 * VT400 & up.
 * Insert column.
 */
static int
tst_DECIC(MENU_ARGS)
{
  int n;
  int last = max_lines - 3;
  int base_row;
  int base_col;
  int last_col;
  int top;
  int bot;
  int lft;
  int rgt;
  int final_ic;
  char mark_1st = 0;
  char mark_2nd = 0;

  test_with_margins(1);
  top = tb_marg1 ? tb_marg1 : 1;
  bot = tb_marg2 ? tb_marg2 : (last - 1);
  lft = (lrmm_flag && lr_marg1) ? lr_marg1 : 1;
  rgt = (lrmm_flag && lr_marg2) ? lr_marg2 : min_cols;

  /*
   * Adjustments so that most of the initial line (before shifting) passes
   * through the area within margins.
   */
  switch (lr_marg_flag) {
  default:
    base_col = (2 * last);
    last_col = min_cols - 1;
    break;
  case marFirst:
    base_col = 0;
    last_col = min_cols / 2 - 1;
    break;
  case marMiddle:
    base_col = min_cols / 4;
    last_col = (3 * min_cols) / 4 - 1;
    break;
  case marLast:
    base_col = (min_cols / 2);
    last_col = min_cols - 1;
    break;
  }

  final_ic = base_col;

  if (tb_marg_flag == marLast) {
    base_row = max_lines / 2;
  } else {
    base_row = 0;
  }

  for (n = 1; n < last; n++) {
    int row = base_row + n;
    int col = base_col + n;

    if (row < last) {
      int mark = marker_of(n);

      if (row >= top && row <= bot && row < last) {
        mark_2nd = (char) mark;
        if (mark_1st == 0) {
          mark_1st = (char) mark;
        }
      }

      __(cup(row, col), putchar(mark));
      if (top > 1 || (lrmm_flag && lft > 1)) {
        __(cup(1, 1), decic(1));  /* outside margins, should be ignored */
        __(cup(row, col), putchar(mark));
      }
      if (final_ic++ <= last_col)
        __(cup(top, lft), decic(1));
    }
  }
  if (final_ic <= last_col)
    decic(last_col - final_ic);
  test_with_margins(0);

  ruler(last);
  vt_move(last + 1, 1);
  vt_clear(0);

  if (lrmm_flag)
    tprintf("If your terminal supports DECIC, letters %c-%c are on column %d\n",
            mark_1st, mark_2nd, rgt);
  else
    println("There should be a diagonal of letters from left near top to middle at bottom");
  return MENU_HOLD;
}

static int
tst_DECKBUM(MENU_ARGS)
{
  vt_move(1, 1);
  println(the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  deckbum(TRUE);
  println("The keyboard is set for data processing.");
  show_keypress(3, 10);

  vt_move(10, 1);
  deckbum(FALSE);
  println("The keyboard is set for normal (typewriter) processing.");
  show_keypress(11, 10);

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

static int
tst_DECKPM(MENU_ARGS)
{
  vt_move(1, 1);
  println(the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  deckpm(TRUE);
  println("The keyboard is set for position reports.");
  show_keypress(3, 10);

  vt_move(10, 1);
  deckpm(FALSE);
  println("The keyboard is set for character codes.");
  show_keypress(11, 10);

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

static int
tst_DECNKM(MENU_ARGS)
{
  vt_move(1, 1);
  println(the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  decnkm(FALSE);
  println("Press one or more keys on the keypad.  They should generate numeric codes.");
  show_keypress(3, 10);

  vt_move(10, 1);
  decnkm(TRUE);
  println("Press one or more keys on the keypad.  They should generate control codes.");
  show_keypress(11, 10);

  decnkm(FALSE);
  vt_move(max_lines - 1, 1);
  restore_ttymodes();
  return MENU_HOLD;
}

/*
 * VT400 & up
 * Reverse Attributes in Rectangular Area
 */
static int
tst_DECRARA(MENU_ARGS)
{
  int last = max_lines - 4;
  int top = 5;
  int left = 5;
  int right = 45;
  int bottom = max_lines - 10;

  decsace(TRUE);
  fill_screen();
  decrara(top, left, bottom, right, 7);   /* invert a rectangle) */
  decrara(top + 1, left + 1, bottom - 1, right - 1, 7);   /* invert a rectangle) */

  sgr("0");
  vt_move(last, 1);
  vt_clear(0);

  println(the_title);
  println("There should be an open rectangle formed by reverse-video E's");
  holdit();

  decsace(FALSE);
  fill_screen();
  decrara(top, left, bottom, right, 7);   /* invert a rectangle) */
  decrara(top + 1, left + 1, bottom - 1, right - 1, 7);   /* invert a rectangle) */

  sgr("0");
  vt_move(last, 1);
  vt_clear(0);

  println(the_title);
  println("There should be an open rectangle formed by reverse-video E's");
  println("combined with wrapping at the margins.");
  return MENU_HOLD;
}

int
tst_vt420_DECRQSS(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT320 features (DECRQSS)",                     tst_vt320_DECRQSS },
      { "Select attribute change extent (DECSACE)",          rpt_DECSACE },
      { "Set number of lines per screen (DECSNLS)",          rpt_DECSNLS },
      { "Set left and right margins (DECSLRM)",              rpt_DECSLRM },
      { "Enable local functions (DECELF)",                   rpt_DECELF },
      { "Local function key control (DECLFKC)",              rpt_DECLFKC },
      { "Select modifier key reporting (DECSMKR)",           rpt_DECSMKR },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT420 Status-Strings Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/*
 * Selective-Erase Rectangular area
 */
static int
tst_DECSERA(MENU_ARGS)
{
  int top = 5;
  int left = 5;
  int right = min_cols - 5;
  int bottom = max_lines - 10;
  int last = max_lines - 3;

  fill_screen();

  /*
   * Fill an area slightly smaller than we will erase.
   *
   * That way (since the SGR color at this point differs from the color used to
   * fill the screen), we can see whether the colors are modified by the erase,
   * and if so, whether they come from the SGR color.
   */
  decsca(0);
  decfra('*', top + 1, left + 1, bottom - 1, right - 1);
  decsca(1);

  sgr("0");
  vt_move(last, 1);
  vt_clear(0);

  println(the_title);
  tprintf("Rectangle %d,%d - %d,%d was filled using DECFRA\n",
          top + 1,
          left + 1,
          bottom - 1,
          right - 1);
  holdit();

  sgr(WHITE_ON_GREEN);
  decsera(top, left, bottom, right);  /* erase the inside */
  decsca(0);

  sgr("0");
  vt_move(last, 1);
  vt_clear(0);

  println(the_title);
  tprintf("Rectangle %d,%d - %d,%d is cleared using DECSERA\n",
          top,
          left,
          bottom,
          right);
  return MENU_HOLD;
}

/* FIXME: use DECRQSS to get reports */
static int
tst_DECSNLS(MENU_ARGS)
{
  int rows;

  vt_move(1, 1);
  println("Testing Select Number of Lines per Screen (DECSNLS)");

  for (rows = 48; rows >= 24; rows -= 12) {
    set_tty_raw(TRUE);
    set_tty_echo(FALSE);

    printf("%d Lines/Screen: ", rows);
    decsnls(rows);
    decrqss("*|");
    chrprint(instr());
    println("");

    restore_ttymodes();
    holdit();
  }

  return MENU_NOHOLD;
}

static int
tst_DSR_area_sum(MENU_ARGS)
{
  return tst_DECCKSR(PASS_ARGS, 1, "1;1;10;10;20;20*y");
}

static int
tst_DSR_data_ok(MENU_ARGS)
{
  return any_DSR(PASS_ARGS, "?75n", show_DataIntegrity);
}

static int
tst_DSR_macrospace(MENU_ARGS)
{
  char *report;
  const char *show;

  vt_move(1, 1);
  printf("Testing DECMSR: %s\n", the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("?62n");
  report = instr();
  vt_move(3, 10);
  chrprint(report);
  if ((report = skip_csi(report)) != 0
      && (report = skip_digits(report)) != 0
      && !strcmp(report, "*{")) {
    show = SHOW_SUCCESS;
  } else {
    show = SHOW_FAILURE;
  }
  show_result("%s", show);

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

static int
tst_DSR_memory_sum(MENU_ARGS)
{
  return tst_DECCKSR(PASS_ARGS, 1, "?63;1n");
}

static int
tst_DSR_multisession(MENU_ARGS)
{
  return any_DSR(PASS_ARGS, "?85n", show_MultisessionStatus);
}

int
tst_SRM(MENU_ARGS)
{
  int oldc, newc;

  vt_move(1, 1);
  println(the_title);

  set_tty_raw(TRUE);

  set_tty_echo(FALSE);
  srm(FALSE);

  println("Local echo is enabled, remote echo disabled.  Press any keys, repeat to quit.");
  vt_move(3, 10);

  oldc = -1;
  while ((newc = inchar()) != oldc)
    oldc = newc;

  set_tty_echo(TRUE);
  srm(TRUE);

  vt_move(10, 1);
  println("Local echo is disabled, remote echo enabled.  Press any keys, repeat to quit.");
  vt_move(11, 10);

  oldc = -1;
  while ((newc = inchar()) != oldc)
    oldc = newc;

  vt_move(max_lines - 1, 1);
  restore_ttymodes();
  return MENU_HOLD;
}

/******************************************************************************/

static int
tst_PageFormat(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test set columns per page (DECSCPP)",               not_impl },
      { "Test columns mode (DECCOLM)",                       not_impl },
      { "Test set lines per page (DECSLPP)",                 not_impl },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("Page Format Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * The main vt100 module tests CUP, HVP, CUF, CUB, CUU, CUD
 */
static int
tst_VT420_cursor(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT320 features",                               tst_vt320_cursor },
      { lrmm_mesg,                                           toggle_LRMM },
      { tb_marg_mesg,                                        toggle_STBM },
      { lr_marg_mesg,                                        toggle_SLRM },
      { "Test Back Index (DECBI)",                           tst_DECBI },
      { "Test Forward Index (DECFI)",                        tst_DECFI },
      { "Test cursor movement within margins",               tst_cursor_margins },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  tb_marg_flag = marNone;
  toggle_STBM(PASS_ARGS);

  lr_marg_flag = marNone;
  toggle_SLRM(PASS_ARGS);

  do {
    vt_clear(2);
    __(title(0), printf("VT420 Cursor-Movement Tests"));
    __(title(2), println("Choose test type:"));
    sprintf(lrmm_mesg, "%s DECLRMM (left/right mode)", STR_ENABLE(lrmm_flag));
  } while (menu(my_menu));

  if (tb_marg_flag > marReset)
    decstbm(0, 0);

  if (lr_marg_flag > marReset) {
    if (!lrmm_flag)
      toggle_LRMM(PASS_ARGS);
    decslrm(0, 0);
  }

  if (lrmm_flag)
    toggle_LRMM(PASS_ARGS);

  return MENU_NOHOLD;
}

/******************************************************************************/

static int
show_DECSTBM(MENU_ARGS)
{
  int code;
  decstbm(tb_marg1, tb_marg2);
  code = rpt_DECSTBM(PASS_ARGS);
  return code;
}

static int
show_DECSLRM(MENU_ARGS)
{
  int code;
  decslrm(lr_marg1, lr_marg2);
  code = rpt_DECSLRM(PASS_ARGS);
  return code;
}

/*
 * The main vt100 module tests IRM, DL, IL, DCH, ICH, ED, EL
 * The vt220 module tests ECH and DECSCA
 */
static int
tst_VT420_editing(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { lrmm_mesg,                                           toggle_LRMM },
      { "Show DECRQM response for DECLRMM",                  show_DECLRMM },
      { "Show DECRQSS response for DECSTBM",                 show_DECSTBM },
      { "Show DECRQSS response for DECSLRM",                 show_DECSLRM },
      { tb_marg_mesg,                                        toggle_STBM },
      { lr_marg_mesg,                                        toggle_SLRM },
      { "Test Delete Column (DECDC)",                        tst_DECDC },
      { "Test Insert Column (DECIC)",                        tst_DECIC },
      { "Test vertical scrolling (IND, RI)",                 tst_IND_RI },
      { "Test ASCII formatting (BS, CR, TAB)",               tst_ASCII_format },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  tb_marg_flag = marNone;
  toggle_STBM(PASS_ARGS);

  lr_marg_flag = marNone;
  toggle_SLRM(PASS_ARGS);

  do {
    vt_clear(2);
    __(title(0), printf("VT420 Editing Sequence Tests"));
    __(title(2), println("Choose test type:"));
    sprintf(lrmm_mesg, "%s DECLRMM (left/right mode)", STR_ENABLE(lrmm_flag));
  } while (menu(my_menu));

  if (tb_marg_flag > marReset)
    decstbm(0, 0);

  if (lr_marg_flag > marReset) {
    if (!lrmm_flag)
      toggle_LRMM(PASS_ARGS);
    decslrm(0, 0);
  }

  if (lrmm_flag)
    toggle_LRMM(PASS_ARGS);

  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * The main vt100 module tests LNM, DECKPAM, DECARM, DECAWM
 */
static int
tst_VT420_keyboard_ctl(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Backarrow key (DECBKM)",                       tst_DECBKM },
      { "Test Numeric keypad (DECNKM)",                      tst_DECNKM },
      { "Test Keyboard usage (DECKBUM)",                     tst_DECKBUM },
      { "Test Key position (DECKPM)",                        tst_DECKPM },
      { "Test Enable Local Functions (DECELF)",              not_impl },
      { "Test Local Function-Key Control (DECLFKC)",         not_impl },
      { "Test Select Modifier-Key Reporting (DECSMKR)",      not_impl }, /* DECEKBD */
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT420 Keyboard-Control Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
tst_VT420_rectangle(MENU_ARGS)
{
  static char txt_override_lines[80];
  static char txt_override_color[80];
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { txt_override_lines,                                  toggle_lines_mode, },
      { txt_override_color,                                  toggle_color_mode, },
      { "Test Change-Attributes in Rectangular Area (DECCARA)", tst_DECCARA },
      { "Test Copy Rectangular area (DECCRA)",               tst_DECCRA },
      { "Test Erase Rectangular area (DECERA)",              tst_DECERA },
      { "Test Fill Rectangular area (DECFRA)",               tst_DECFRA },
      { "Test Reverse-Attributes in Rectangular Area (DECRARA)", tst_DECRARA },
      { "Test Selective-Erase Rectangular area (DECSERA)",   tst_DECSERA },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  /*
   * Avoid confusion - terminal must be in VT420 mode to use these tests.
   */
  if (terminal_id() < 400) {
    printf("Sorry, a real VT%d terminal doesn't implement rectangle operations\n",
           terminal_id());
    return MENU_HOLD;
  }

  do {
    vt_clear(2);
    sprintf(txt_override_lines, "%s line-drawing characters",
            do_lines ? "Use" : "Do not use");
    sprintf(txt_override_color, "%s background and rectangles (xterm)",
            do_colors ? "Color" : "Do not color");
    __(title(0), printf("VT420 Rectangular Area Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/* UDK and rectangle-checksum status are available only on VT400 */

int
tst_vt420_device_status(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT320 features",                               tst_vt320_device_status },
      { "Test Printer Status",                               tst_DSR_printer },
      { "Test UDK Status",                                   tst_DSR_userkeys },
      { "Test Keyboard Status",                              tst_DSR_keyboard },
      { "Test Locator Status",                               tst_DSR_locator },
      { "Test Macro Space",                                  tst_DSR_macrospace },
      { "Test Memory Checksum",                              tst_DSR_memory_sum },
      { "Test Data Integrity",                               tst_DSR_data_ok },
      { "Test Multiple Session Status",                      tst_DSR_multisession },
      { "Test Checksum of Rectangular Area",                 tst_DSR_area_sum },
      { "Test Extended Cursor-Position (DECXCPR)",           tst_DSR_cursor },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT420 Device Status Reports (DSR)"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

int
tst_vt420_report_presentation(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT320 features",                               tst_vt320_report_presentation },
      { "Request Mode (DECRQM)/Report Mode (DECRPM)",        tst_DECRPM },
      { "Status-String Report (DECRQSS)",                    tst_vt420_DECRQSS },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  int old_DECRPM = set_DECRPM(4);

  do {
    vt_clear(2);
    __(title(0), printf("VT420 Presentation State Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  set_DECRPM(old_DECRPM);
  return MENU_NOHOLD;
}

int
tst_vt420_reports(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT320 features",                               tst_vt320_reports },
      { "Test Presentation State Reports",                   tst_vt420_report_presentation },
      { "Test Device Status Reports (DSR)",                  tst_vt420_device_status },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT420 Reports"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
tst_VT420_screen(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT320 features",                               tst_vt320_screen },
      { "Test Select Number of Lines per Screen (DECSNLS)",  tst_DECSNLS },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT420 Screen-Display Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * These apply only to VT400's & above.
 */
int
tst_vt420(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test VT320 features",                               tst_vt320 },
      { "Test cursor-movement",                              tst_VT420_cursor },
      { "Test editing sequences",                            tst_VT420_editing },
      { "Test keyboard-control",                             tst_VT420_keyboard_ctl },
      { "Test macro-definition (DECDMAC)",                   not_impl },
      { "Test page-format controls",                         tst_PageFormat },
      { "Test rectangular area functions",                   tst_VT420_rectangle },
      { "Test reporting functions",                          tst_vt420_reports },
      { "Test screen-display functions",                     tst_VT420_screen },
      { "",                                                  0 }
    };
  /* *INDENT-ON* */

  do {
    vt_clear(2);
    __(title(0), printf("VT420 Tests"));
    __(title(2), println("Choose test type:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/* $Id: setup.c,v 1.43 2023/12/05 09:16:16 tom Exp $ */

#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>

static int cur_level = -1;      /* current operating level (VT100=1) */
static int max_level = -1;      /* maximum operating level */

static int
check_8bit_toggle(void)
{
  char *report;

  set_tty_raw(TRUE);

  cup(1, 1);
  dsr(6);
  padding(5);   /* may not be needed */
  report = get_reply();

  restore_ttymodes();

  if ((report = skip_csi(report)) != 0
      && !strcmp(report, "1;1R"))
    return TRUE;
  return FALSE;
}

/*
 * Determine the current and maximum operating levels of the terminal
 */
static void
find_levels(void)
{
  char *report;
  int actual_id = -1;
  int has_color = 0;

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  da();
  report = get_reply();

  cur_level =
    max_level = 1;  /* assume a VT100 */

  if (!strcmp(report, "\033/Z")) {
    actual_id = 52;
    cur_level =
      max_level = 0;  /* must be a VT52 */
  } else if ((report = skip_csi(report)) == 0 || strlen(report) < 3) {
    actual_id = 100;
  } else if (*report == '?') {
    int pos = 1;
    int check = scan_DA(report, &pos);
    switch (check) {
    default:
    case 1:
      actual_id = 100;
      break;
    case 2:
    case 6:
      actual_id = 102;
      break;
    case 4:
      actual_id = 132;
      break;
    case 7:
      actual_id = 131;
      break;
    case 12:
      actual_id = 125;
      break;
    case 62:
      actual_id = 220;
      break;
    case 63:
      actual_id = 320;
      break;
    case 64:
      actual_id = 420;
      break;
    case 65:
      actual_id = 520;
      break;
    }
    cur_level = max_level = (actual_id / 100);
  }

  if (max_level >= 4) {
    int pos = 1;                /* index of first digit */
    int check;
    /*
     * Check for color feature (see below) needed in initialization of VT525.
     * Other terminals may advertise this feature, but only VT525 will change
     * its response in DECRQCRA according to color.
     */
    while ((check = scan_DA(report, &pos)) >= 0) {
      if (check == 22) {
        has_color = 1;
        break;
      }
    }
    decrqss("\"p");
    report = get_reply();
    if ((report = skip_dcs(report)) != 0
        && isdigit(CharOf(*report++))   /* 0 or 1 (by observation, though 1 is an err) */
        &&*report++ == '$'
        && *report++ == 'r'
        && *report++ == '6'
        && isdigit(CharOf(*report)))
      cur_level = *report - '0';
  }

  if (max_level >= 2) {
    do_csi(">c");   /* or "CSI > 0 c" */
    report = get_reply();
    if ((report = skip_csi(report)) != 0 && *report == '>') {
      int pos = 1;              /* index of first digit */
      switch (scan_DA(report, &pos)) {
      case 1:
        actual_id = 220;
        break;
      case 2:
        actual_id = 240;
        break;
      case 18:
        actual_id = 330;
        break;
      case 19:
        actual_id = 340;
        break;
      case 24:
        actual_id = 320;
        break;
      case 28:
        actual_id = 330;
        break;
      case 32:
        actual_id = 382;
        break;
      case 48:
        actual_id = 382;
        break;
      case 41:
        actual_id = 420;
        break;
      case 61:
        actual_id = 510;
        break;
      case 64:
        actual_id = 520;
        break;
      case 65:
        actual_id = 525;
        break;
      }
    }
  }

  /*
   * If the terminal advertises color (a VT525 feature), initialize the
   * text color to the default values, to simplify the DECRQCRA test.
   * Defaults are documented in EK-VT520-RM, p 2-32.
   */
  if (has_color && actual_id == 525) {
    use_decac = 1;
    decac(1, 7, 0);   /* normal text: assign white-on-black */
  }

  if (LOG_ENABLED) {
    fprintf(log_fp, "Max Operating Level: %d\n", max_level);
    fprintf(log_fp, "Cur Operating Level: %d\n", cur_level);
    fprintf(log_fp, "Derived terminal-id: %d\n", actual_id);
  }

  restore_ttymodes();
}

static int
toggle_DECSCL(MENU_ARGS)
{
  int request = cur_level;

  if (max_level <= 1) {
    vt_move(1, 1);
    printxx("Sorry, terminal supports only VT%d", terminal_id());
    vt_move(max_lines - 1, 1);
    return MENU_HOLD;
  }

  if (++request > max_level)
    request = 1;
  set_level(request);

  restore_ttymodes();
  return MENU_NOHOLD;
}

static int
toggle_Logging(MENU_ARGS)
{
  if (log_fp == 0)
    enable_logging();
  else
    log_disabled = !log_disabled;
  return MENU_NOHOLD;
}

static int
toggle_Padding(MENU_ARGS)
{
  use_padding = !use_padding;
  return MENU_NOHOLD;
}

static int
toggle_Slowly(MENU_ARGS)
{
  slow_motion = !slow_motion;
  return MENU_NOHOLD;
}

static int
toggle_8bit_in(MENU_ARGS)
{
  int old = input_8bits;

  s8c1t(!old);
  fflush(stdout);
  if (!check_8bit_toggle()) {
    input_8bits = old;
    vt_clear(2);
    vt_move(1, 1);
    println("Sorry, this terminal does not support 8-bit input controls");
    return MENU_HOLD;
  }
  return MENU_NOHOLD;
}

/*
 * This changes the CSI code to/from an escape sequence.
 */
static int
toggle_8bit_out(MENU_ARGS)
{
  int old = output_8bits;

  fflush(stdout);
  output_8bits = !output_8bits;
  if (!check_8bit_toggle()) {
    output_8bits = old;
    vt_clear(2);
    vt_move(1, 1);
    println("Sorry, this terminal does not support 8-bit output controls");
    return MENU_HOLD;
  }
  return MENU_NOHOLD;
}

/******************************************************************************/

void
enable_logging(void)
{
  static char my_name[] = "vttest.log";
  log_fp = fopen(my_name, "w");
  if (log_fp == 0) {
    failed(my_name);
  }
}

void
reset_level(void)
{
  cur_level = max_level;
}

void
restore_level(VTLEVEL *save)
{
  set_level(save->cur_level);
  if (cur_level > 1
      && save->input_8bits != input_8bits)  /* just in case level didn't change */
    s8c1t(save->input_8bits);
  output_8bits = save->output_8bits;  /* in case we thought this was VT100 */
}

void
save_level(VTLEVEL *save)
{
  save->cur_level = cur_level;
  save->input_8bits = input_8bits;
  save->output_8bits = output_8bits;

  if (LOG_ENABLED)
    fprintf(log_fp, "save_level(%d) in=%d, out=%d\n", cur_level,
            input_8bits ? 8 : 7,
            output_8bits ? 8 : 7);
}

int
get_level(void)
{
  return cur_level;
}

int
set_level(int request)
{
  if (cur_level < 0)
    find_levels();

  if (LOG_ENABLED)
    fprintf(log_fp, "set_level(%d)\n", request);

  if (request > max_level) {
    printxx("Sorry, this terminal supports only VT%d\n", terminal_id());
    return FALSE;
  }

  if (request != cur_level) {
    if (request == 0) {
      rm("?2"); /* Reset ANSI (VT100) mode, Set VT52 mode  */
      input_8bits = FALSE;
      output_8bits = FALSE;
    } else {
      if (cur_level == 0) {
        esc("<");   /* Enter ANSI mode (VT100 mode) */
      }
      if (request == 1) {
        input_8bits = FALSE;
        output_8bits = FALSE;
      }
      if (max_level > 1) {
        if (request > 1)
          do_csi("6%d;%d\"p", request, !input_8bits);
        else
          do_csi("61\"p");
      }
    }
    padding(5); /* may not be needed */

    cur_level = request;
  }

  if (LOG_ENABLED)
    fprintf(log_fp, "...set_level(%d) in=%d, out=%d\n", cur_level,
            input_8bits ? 8 : 7,
            output_8bits ? 8 : 7);

  return TRUE;
}

/*
 * Set the terminal's operating level to the default (i.e., based on what the
 * terminal returns as a response to DA).
 */
void
default_level(void)
{
  if (max_level < 0)
    find_levels();
  set_level(max_level);
}

int
terminal_id(void)
{
  if (max_level >= 1)
    return max_level * 100;
  else if (max_level == 0)
    return 52;
  return 100;
}

int
tst_setup(MENU_ARGS)
{
  static char txt_output[80] = "send 7/8";
  static char txt_input8[80] = "receive 7/8";
  static char txt_DECSCL[80] = "DECSCL";
  static char txt_logging[80] = "logging";
  static char txt_padded[80] = "padding";
  static char txt_slowly[80];
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
    { "Exit",                                                0 },
    { "Setup terminal to original test-configuration",       setup_terminal },
    { txt_output,                                            toggle_8bit_out },
    { txt_input8,                                            toggle_8bit_in },
    { txt_DECSCL,                                            toggle_DECSCL },
    { txt_logging,                                           toggle_Logging },
    { txt_padded,                                            toggle_Padding },
    { txt_slowly,                                            toggle_Slowly },
    { "",                                                    0 }
  };
  /* *INDENT-ON* */

  if (cur_level < 0)
    find_levels();

  do {
    sprintf(txt_output, "Send %d-bit controls", output_8bits ? 8 : 7);
    sprintf(txt_input8, "Receive %d-bit controls", input_8bits ? 8 : 7);
    sprintf(txt_DECSCL, "Operating level %d (VT%d)",
            cur_level, cur_level ? cur_level * 100 : 52);
    sprintf(txt_logging, "Logging %s", STR_ENABLED(LOG_ENABLED));
    sprintf(txt_padded, "Padding %s", STR_ENABLED(use_padding));
    sprintf(txt_slowly, "Slow-movement/scrolling %s", STR_ENABLED(slow_motion));

    vt_clear(2);
    __(title(0), println("Modify test-parameters"));
    __(title(2), println("Select a number to modify it:"));
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

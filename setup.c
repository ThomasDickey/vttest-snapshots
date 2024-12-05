/* $Id: setup.c,v 1.55 2024/12/05 00:40:24 tom Exp $ */

#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>

#define PARSE_NAME(mode) \
        ((mode > 1) \
          ? "C2" \
          : (mode == 1 \
             ? "8-bit" \
             : "7-bit"))

static int cur_level = -1;      /* current operating level (VT100=1) */
static int max_level = -1;      /* maximum operating level */

static int
check_8bit_toggle(void)
{
  int result = FALSE;
  char *report;

  set_tty_raw(TRUE);

  pause_replay();
  cup(1, 1);
  dsr(6);
  padding(5);   /* may not be needed */
  report = get_reply();
  resume_replay();

  restore_ttymodes();

  if ((report = skip_csi(report)) != NULL
      && !strcmp(report, "1;1R"))
    result = TRUE;
  if (LOG_ENABLED) {
    fprintf(log_fp, NOTE_STR "%svalid response from DSR 6\n",
            result ? "" : "no ");
  }
  return result;
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
  } else if ((report = skip_csi(report)) == NULL || strlen(report) < 3) {
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
    if ((report = skip_dcs(report)) != NULL
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
    if ((report = skip_csi(report)) != NULL && *report == '>') {
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
    fprintf(log_fp, NOTE_STR "Max Operating Level: %d\n", max_level);
    fprintf(log_fp, NOTE_STR "Cur Operating Level: %d\n", cur_level);
    fprintf(log_fp, NOTE_STR "Derived terminal-id: %d\n", actual_id);
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
  if (log_fp == NULL)
    enable_logging(NULL);
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

static int
toggle_7bit_fsm(MENU_ARGS)
{
  int save_7bits = parse_7bits;

  /*
   * See comment in put_char().  There are two cases for the parse_7bits value.
   * In both cases, 7-bit controls should work when parse_7bits is zero.  The
   * tests relate to the nonzero value:
   *
   * parse_7bits == 1:
   * This is for the 8-bit environment, i.e., 8-bit controls should work.  But
   * ECMA-49 section 9 comes into play because 7-bit controls are parsed
   * ignoring the 8th bit.  The C1 controls have specific definitons which
   * override that, but by ignoring the 8th bit that guarantees that after the
   * initial C0 or C1 byte, all of the following characters will be parsed
   * without the 8th bit.
   *
   * This case is tested by setting the 8th bit in the controls to ensure that
   * the parser has to deal with that situation.
   *
   * parse_7bits == 2:
   * This is for the UTF-8 environment.  C1 controls in put_char() are recoded
   * to UTF-8 to test if the terminal can interpret those.  Normally vttest
   * resets UTF-8 mode on startup; the "-u" option suppresses that, allowing
   * this test.
   */
  if (assume_utf8 && allows_utf8) {
    parse_7bits = parse_7bits ? 0 : 2;
  } else {
    parse_7bits = parse_7bits ? 0 : 1;
  }
  if (LOG_ENABLED) {
    fprintf(log_fp, NOTE_STR "toggle parser from %s to %s\n",
            PARSE_NAME(save_7bits),
            PARSE_NAME(parse_7bits));
  }

  /*
   * If we're told to switch to 8-bit or C2 mode, check that we can do that.
   */
  if (parse_7bits != 0) {
    int save_output = output_8bits;
    int rc;
    output_8bits = 1;
    rc = check_8bit_toggle();
    output_8bits = save_output;
    if (!rc) {
      int requested = parse_7bits;
      char msg[80];
      parse_7bits = save_7bits;
      vt_clear(2);
      vt_move(1, 1);
      sprintf(msg,
              "Sorry, this terminal does not fully support %s parsing",
              PARSE_NAME(requested));
      println(msg);
      return MENU_HOLD;
    }
    output_8bits = save_output;
  } else {
    output_8bits = 0;
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

/*
 * While processing command-lines, "pathname" is non-null.  After that, it can
 * be null to temporarily disable logging.
 */
void
enable_logging(const char *pathname)
{
  static char *my_name;
  if (pathname != NULL) {
    free(my_name);
    my_name = strdup(pathname);
  }
  if (my_name != NULL) {
    log_fp = fopen(my_name, "w");
    if (log_fp == NULL) {
      failed(my_name);
    }
  }
}

void
reset_level(void)
{
  cur_level = max_level;
}

void
restore_level(const VTLEVEL *save)
{
  set_level(save->cur_level);
  if (cur_level > 1
      && save->input_8bits != input_8bits)  /* just in case level didn't change */
    s8c1t(save->input_8bits);
  output_8bits = save->output_8bits;  /* in case we thought this was VT100 */
  parse_7bits = save->parse_7bits;  /* in case we ignored ECMA-48 */
}

void
save_level(VTLEVEL *save)
{
  save->cur_level = cur_level;
  save->input_8bits = input_8bits;
  save->output_8bits = output_8bits;
  save->parse_7bits = parse_7bits;

  if (LOG_ENABLED)
    fprintf(log_fp, NOTE_STR "save_level(%d) in=%d, out=%d, fsm=%d\n",
            cur_level,
            input_8bits ? 8 : 7,
            output_8bits ? 8 : 7,
            parse_7bits ? 8 : 7);
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
    fprintf(log_fp, NOTE_STR "set_level(%d)\n", request);

  if (request > max_level) {
    printxx("Sorry, this terminal supports only VT%d\n", terminal_id());
    return FALSE;
  }

  if (request != cur_level) {
    if (request == 0) {
      rm("?2"); /* Reset ANSI (VT100) mode, Set VT52 mode  */
      input_8bits = FALSE;
      output_8bits = FALSE;
      parse_7bits = FALSE;
    } else {
      if (cur_level == 0) {
        esc("<");   /* Enter ANSI mode (VT100 mode) */
      }
      if (request == 1) {
        input_8bits = FALSE;
        output_8bits = FALSE;
        parse_7bits = FALSE;
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
    fprintf(log_fp, NOTE_STR "...set_level(%d) in=%d, out=%d, fsm=%d\n",
            cur_level,
            input_8bits ? 8 : 7,
            output_8bits ? 8 : 7,
            parse_7bits ? 8 : 7);

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
  static char txt_parse7[80] = "parser 7/8";
  static char txt_DECSCL[80] = "DECSCL";
  static char txt_logging[80] = "logging";
  static char txt_padded[80] = "padding";
  static char txt_slowly[80];
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
    { "Exit",                                                NULL },
    { "Setup terminal to original test-configuration",       setup_terminal },
    { txt_output,                                            toggle_8bit_out },
    { txt_input8,                                            toggle_8bit_in },
    { txt_parse7,                                            toggle_7bit_fsm },
    { txt_DECSCL,                                            toggle_DECSCL },
    { txt_logging,                                           toggle_Logging },
    { txt_padded,                                            toggle_Padding },
    { txt_slowly,                                            toggle_Slowly },
    { "",                                                    NULL }
  };
  /* *INDENT-ON* */

  if (cur_level < 0)
    find_levels();

  do {
    sprintf(txt_output, "Send %s controls",
            (output_8bits
             ? ((parse_7bits > 1)
                ? "C2"
                : "8-bit")
             : "7-bit"));
    sprintf(txt_input8, "Receive %d-bit controls", input_8bits ? 8 : 7);
    switch (parse_7bits) {
    default:
    case 1:
      sprintf(txt_parse7, "Assume %d-bit parser (see ECMA-48 section 9)",
              parse_7bits ? 7 : 8);
      break;
    case 2:
      sprintf(txt_parse7,
              "Assume parser accepts C2 (Unicode.org two-byte encoding)");
      break;
    }
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

/* $Id: setup.c,v 1.18 1996/09/09 19:51:52 tom Exp $ */

#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>

static int cur_level; /* current operating level (VT100=1) */
static int max_level; /* maximum operating level */

static int
check_8bit_toggle(void)
{
  char *report;

  set_tty_raw(TRUE);
  cup(1,1); dsr(6);
  padding(5); /* FIXME: may not be needed */
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

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  da();
  report = get_reply();
  if ((report = skip_csi(report)) == 0
   || strncmp(report, "?6", 2)
   || !isdigit(report[2])
   || report[3] != ';') {
    cur_level =
    max_level = 1; /* must be a VT100 */
  } else { /* "CSI ? 6 x ; ..." */
    cur_level =
    max_level = report[2] - '0'; /* VT220=2, VT320=3, VT420=4 */
    if (max_level >= 4) {
      decrqss("\"p");
      report = get_reply();
      if ((report = skip_dcs(report)) != 0
       && isdigit(*report++) /* 0 or 1 (by observation, though 1 is an err) */
       && *report++ == '$'
       && *report++ == 'r'
       && *report++ == '6'
       && isdigit(*report))
          cur_level = *report - '0';
    }
  }

  if (LOG_ENABLED) {
    fprintf(log_fp, "Max Operating Level: %d\n", max_level);
    fprintf(log_fp, "Cur Operating Level: %d\n", cur_level);
  }

  restore_ttymodes();
}

static int
toggle_DECSCL(MENU_ARGS)
{
  if (max_level == 1) {
    vt_move(1,1);
    printf("Sorry, terminal supports only VT100");
    vt_move(max_lines-1,1);
    return MENU_HOLD;
  }

  if (++cur_level > max_level)
    cur_level = 1;
  if (cur_level > 1)
    do_csi("6%d;%d\"p", cur_level, !output_8bits);
  else
    do_csi("61\"p");
  padding(5); /* FIXME: may not be needed */
  zleep(800); /* DECSCL does a soft reset, which is slow */

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
toggle_8bit_in(MENU_ARGS)
{
  int old = input_8bits;

  s8c1t(!old);
  fflush(stdout);
  if (!check_8bit_toggle()) {
    input_8bits = old;
    vt_clear(2);
    vt_move(1,1);
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
    vt_move(1,1);
    println("Sorry, this terminal does not support 8-bit output controls");
    return MENU_HOLD;
  }
  return MENU_NOHOLD;
}

void
enable_logging(void)
{
  static char my_name[] = "vttest.log";
  log_fp = fopen(my_name, "w");
  if (log_fp == 0) {
    perror(my_name);
    exit(EXIT_FAILURE);
  }
}

int
tst_setup(MENU_ARGS)
{
  static char txt_output[80] = "send 7/8";
  static char txt_input8[80] = "receive 7/8";
  static char txt_DECSCL[80] = "DECSCL";
  static char txt_logging[80] = "logging";
  static char txt_padded[80] = "padding";

  static MENU my_menu[] = {
    { "Return to main menu",                                 0 },
    { "Setup terminal to original test-configuration",       setup_terminal },
    { txt_output,                                            toggle_8bit_out },
    { txt_input8,                                            toggle_8bit_in },
    { txt_DECSCL,                                            toggle_DECSCL },
    { txt_logging,                                           toggle_Logging },
    { txt_padded,                                            toggle_Padding },
    { "",                                                    0 }
  };

  if (!cur_level)
    find_levels();

  do {
    sprintf(txt_output, "Send %d-bit controls", output_8bits ? 8 : 7);
    sprintf(txt_input8, "Receive %d-bit controls", input_8bits ? 8 : 7);
    sprintf(txt_DECSCL, "Operating level %d (VT%d00)",
        cur_level, cur_level);
    sprintf(txt_logging, "Logging %s", log_disabled ? "disabled" : "enabled");
    sprintf(txt_padded, "Padding %s", use_padding ? "enabled" : "disabled");

    vt_clear(2);
    title(0); println("Modify test-parameters");
    title(2); println("Select a number to modify it:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

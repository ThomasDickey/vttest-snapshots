/* $Id: setup.c,v 1.4 1996/08/19 00:15:51 tom Exp $ */

#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>

static int operating_level = 1;

static int
check_8bit_toggle(void)
{
  char *report;

  set_tty_raw(TRUE);
  cup(1,1); dsr(6);
  report = instr();
  restore_ttymodes();

  if ((report = skip_prefix(csi_input(), report)) != 0
   && !strcmp(report, "1;1R"))
    return 1;
  return 0;
}

static int
toggle_DECSCL(MENU_ARGS)
{
  char *report;
  static int level; /* FIXME */
  static int max_level = 3;

  if (++level > max_level)
    level = 0;
  if (level)
    do_csi("6%d;%d\"p", level+1, !output_8bits);
  else
    do_csi("61\"p");

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);
  da();
  if ((report = skip_prefix(csi_input(), instr())) == 0
   || strncmp(report, "?6", 2)
   || report[2] != level + 1 + '0'
   || report[3] != ';') {
    max_level = level;
    level = 0;
  }
  restore_ttymodes();
  operating_level = level + 1;
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
    ed(2);
    cup(1,1);
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
    ed(2);
    cup(1,1);
    println("Sorry, this terminal does not support 8-bit output controls");
    return MENU_HOLD;
  }
  return MENU_NOHOLD;
}

int
tst_setup(MENU_ARGS)
{
  static MENU my_menu[] = {
    { "Return to main menu",                                 0 },
    { "send 7/8",                                            toggle_8bit_out },
    { "receive 7/8",                                         toggle_8bit_in },
    { "DECSCL",                                              toggle_DECSCL },
    { "",                                                    0 }
  };

  do {
    my_menu[1].description = output_8bits
			   ? "Send 8-bit controls"
			   : "Send 7-bit controls";

    my_menu[2].description = input_8bits
			   ? "Receive 8-bit controls"
			   : "Receive 7-bit controls";
    switch (operating_level) {
    case 1:
      my_menu[3].description = "Operating level 1 (VT100)";
      break;
    case 2:
      my_menu[3].description = "Operating level 2 (VT200)";
      break;
    case 3:
      my_menu[3].description = "Operating level 3 (VT300)";
      break;
    case 4:
      my_menu[3].description = "Operating level 4 (VT400)";
      break;
    }
    ed(2);
    title(0); println("Modify test-parameters");
    title(2); println("Select a number to modify it:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

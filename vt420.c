/* $Id: vt420.c,v 1.9 1996/08/19 02:42:21 tom Exp $ */

#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>

typedef struct {
  int mode;
  char *name;
  } MODES;

/******************************************************************************/

static int
any_decrqpsr(MENU_ARGS, int Ps)
{
  char *report;

  cup(1,1);
  printf("Testing DECRQPSR: %s\r\n", the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("%d$w", Ps);
  report = instr();
  cup(3,10);
  chrprint(report);
  if ((report = skip_prefix(dcs_input(), report)) != 0) {
    char *st = st_input();
    int len = strlen(report) - (3 + strlen(st));
    if (len > 0
     && *report == Ps + '0'
     && !strncmp(report+1, "$u", 2)
     && !strcmp(report + strlen(report) - strlen(st), st)) {
      printf(" ok (valid request)");
    } else {
      printf(" failed");
    }
  } else {
    printf(" failed");
  }

  restore_ttymodes();
  cup(max_lines-1, 1);
  return MENU_HOLD;
}

static int
any_decrqss(char *msg, char *func)
{
  char *report;

  cup(1,1);
  printf("Testing DECRQSS: %s\r\n", msg);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  decrqss(func);
  report = instr();
  cup(3,10);
  chrprint(report);
  if ((report = skip_prefix(dcs_input(), report)) != 0) {
    char *st = st_input();
    int len = strlen(report) - (3 + strlen(st));
    if (len > 0
     && !strncmp(report, "0$r", 3)
     && !strcmp(report + strlen(report) - strlen(st), st)) {
      printf(" ok (valid request)");
    } else if (len >= 0
     && !strncmp(report, "1$r", 3)
     && !strcmp(report + strlen(report) - strlen(st), st)) {
      printf(" invalid request");
    } else {
      printf(" failed");
    }
  } else {
    printf(" failed");
  }

  restore_ttymodes();
  cup(max_lines-1, 1);
  return MENU_HOLD;
}

static int
any_DSR(MENU_ARGS, char *text)
{
  char *report;

  cup(1,1);
  printf("Testing DSR: %s\r\n", the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("%s", text);
  report = instr();
  cup(3,10);
  chrprint(report);
  if ((report = skip_prefix(csi_input(), report)) != 0
   && strlen(report) > 1) {
    printf(" ok"); /* FIXME: should validate parameters */
  } else {
    printf(" failed");
  }

  restore_ttymodes();
  cup(max_lines-1, 1);
  return MENU_HOLD;
}

/******************************************************************************/

static int
rpt_DECSASD(MENU_ARGS)
{
  return any_decrqss(the_title, "$}");
}

static int
rpt_DECSACE(MENU_ARGS)
{
  return any_decrqss(the_title, "*x");
}

static int
rpt_DECSCA(MENU_ARGS)
{
  return any_decrqss(the_title, "\"q");
}

static int
rpt_DECSCL(MENU_ARGS)
{
  return any_decrqss(the_title, "\"p");
}

static int
rpt_DECSCPP(MENU_ARGS)
{
  return any_decrqss(the_title, "$|");
}

static int
rpt_DECSLPP(MENU_ARGS)
{
  return any_decrqss(the_title, "t");
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
rpt_DECSSDT(MENU_ARGS)
{
  return any_decrqss(the_title, "$~");
}

static int
rpt_DECSTBM(MENU_ARGS)
{
  return any_decrqss(the_title, "r");
}

static int
rpt_SGR(MENU_ARGS)
{
  return any_decrqss(the_title, "m");
}

static int
rpt_DECELF(MENU_ARGS)
{
  return any_decrqss(the_title, "+q");
}

static int
rpt_DECLFKC(MENU_ARGS)
{
  return any_decrqss(the_title, "=}");
}

static int
rpt_DECSMKR(MENU_ARGS)
{
  return any_decrqss(the_title, "+r");
}

/******************************************************************************/

static int
tst_DECCIR(MENU_ARGS)
{
  return any_decrqpsr(PASS_ARGS, 1);
}

static int
tst_ISO_DECRPM(MENU_ARGS)
{
  int mode, Pa, Ps;
  char chr;
  char *report;

  static struct {
    int mode;
    char *name;
  } ansi_modes[] = {
    { GATM, "GATM" },
    { AM,   "AM"   },
    { CRM,  "CRM"  },
    { IRM,  "IRM"  },
    { SRTM, "SRTM" },
    { VEM,  "VEM"  },
    { HEM,  "HEM"  },
    { PUM,  "PUM"  },
    { SRM,  "SRM"  },
    { FEAM, "FEAM" },
    { FETM, "FETM" },
    { MATM, "MATM" },
    { TTM,  "TTM"  },
    { SATM, "SATM" },
    { TSM,  "TSM"  },
    { EBM,  "EBM"  },
    { LNM,  "LNM"  } };

  cup(1,1);
  printf("Testing %s\r\n", the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  for (mode = 0; mode < sizeof(ansi_modes)/sizeof(ansi_modes[0]); mode++) {
    do_csi("%d$p", ansi_modes[mode].mode);
    report = instr();
    cup(mode+2,10);
    printf("%8s", ansi_modes[mode].name);
    chrprint(report);
    if ((report = skip_prefix(csi_input(), report)) != 0
     && sscanf(report, "%d;%d$%c", &Pa, &Ps, &chr) == 3
     && Pa == ansi_modes[mode].mode
     && chr == 'y') {
      switch(Ps) {
      case 0:  printf(" unknown");           break;
      case 1:  printf(" set");               break;
      case 2:  printf(" reset");             break;
      case 3:  printf(" permanently set");   break;
      case 4:  printf(" permanently reset"); break;
      default: printf(" ?");                 break;
      }
    } else {
      printf(" -- failed");
      break;
    }
  }

  restore_ttymodes();
  cup(max_lines-1,1);
  return MENU_HOLD;
}

static int
tst_DEC_DECRPM(MENU_ARGS)
{
  int mode, Pa, Ps;
  char chr;
  char *report;

  static struct {
    int mode;
    char *name;
  } dec_modes[] = {
    { DECCKM,  "DECCKM"  },
    { DECANM,  "DECANM"  },
    { DECCOLM, "DECCOLM" },
    { DECSCLM, "DECSCLM" },
    { DECSCNM, "DECSCNM" },
    { DECOM,   "DECOM"   },
    { DECAWM,  "DECAWM"  },
    { DECARM,  "DECARM"  },
    { DECPFF,  "DECPFF"  },
    { DECPEX,  "DECPEX"  },
    { DECTCEM, "DECTCEM" },
    { DECNRCM, "DECNRCM" },
    { DECHCCM, "DECHCCM" },
    { DECVCCM, "DECVCCM" },
    { DECPCCM, "DECPCCM" },
    { DECNKM,  "DECNKM"  },
    { DECBKM,  "DECBKM"  },
    { DECKBUM, "DECKBUM" },
    { DECVSSM, "DECVSSM" },
    { DECXRLM, "DECXRLM" },
    { DECKPM,  "DECKPM"  } };

  cup(1,1);
  printf("Testing %s\r\n", the_title);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  for (mode = 0; mode < sizeof(dec_modes)/sizeof(dec_modes[0]); mode++) {
    do_csi("?%d$p", dec_modes[mode].mode);
    report = instr();
    cup(mode+2,10);
    printf("%8s", dec_modes[mode].name);
    chrprint(report);
    if ((report = skip_prefix(csi_input(), report)) != 0
     && sscanf(report, "%d;%d$%c", &Pa, &Ps, &chr) == 3
     && Pa == dec_modes[mode].mode
     && chr == 'y') {
      switch(Ps) {
      case 0:  printf(" unknown");           break;
      case 1:  printf(" set");               break;
      case 2:  printf(" reset");             break;
      case 3:  printf(" permanently set");   break;
      case 4:  printf(" permanently reset"); break;
      default: printf(" ?");                 break;
      }
    } else {
      printf(" -- failed");
      break;
    }
  }

  restore_ttymodes();
  cup(max_lines-1,1);
  return MENU_HOLD;
}

/* Test Window Report - VT400 */
static int
tst_DECRQDE(MENU_ARGS)
{
  char *report;
  char chr;
  int Ph, Pw, Pml, Pmt, Pmp;

  ed(2);
  cup(1,1);
  println("Testing DECRQDE/DECRPDE Window Report");

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("\"v");
  report = instr();
  cup(3,10);
  chrprint(report);

  if ((report = skip_prefix(csi_input(), report)) != 0
   && sscanf(report, "%d;%d;%d;%d;%d\"%c",
      &Ph, &Pw, &Pml, &Pmt, &Pmp, &chr) == 6
   && chr == 'w') {
    cup(5, 10);
    printf("lines:%d, cols:%d, left col:%d, top line:%d, page %d",
      Ph, Pw, Pml, Pmt, Pmp);
  } else {
    printf(" failed");
  }

  restore_ttymodes();
  cup(max_lines-1,1);
  return MENU_HOLD;
}

static int
tst_DECRPSS(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Select active status display (DECSASD)",            rpt_DECSASD },
      { "Select attribute change extent (DECSACE)",          rpt_DECSACE },
      { "Set character attribute (DECSCA)",                  rpt_DECSCA },
      { "Set conformance level (DECSCL)",                    rpt_DECSCL },
      { "Set columns per page (DECSCPP)",                    rpt_DECSCPP },
      { "Set lines per page (DECSLPP)",                      rpt_DECSLPP },
      { "Set number of lines per screen (DECSNLS)",          rpt_DECSNLS },
      { "Set status line type (DECSSDT)",                    rpt_DECSSDT },
      { "Set left and right margins (DECSLRM)",              rpt_DECSLRM },
      { "Set top and bottom margins (DECSTBM)",              rpt_DECSTBM },
      { "Select graphic rendition (SGR)",                    rpt_SGR },
      { "Enable local functions (DECELF)",                   rpt_DECELF },
      { "Local function key control (DECLFKC)",              rpt_DECLFKC },
      { "Select modifier key reporting (DECSMKR)",           rpt_DECSMKR },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT420 Status-Strings Reports");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/* Test User-Preferred Supplemental Set - VT400 */
static int
tst_DECRQUPSS(MENU_ARGS)
{
  char *report;

  ed(2);
  cup(1,1);
  println("Testing DECRQDE/DECRPDE Window Report");

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("&u");
  report = instr();
  cup(3,10);
  chrprint(report);
  if ((report = skip_prefix(csi_input(), report)) != 0) {
    if (strlen(report) > 4
     && !strncmp(report, "!u%5", 4)
     && !strcmp(report+4, st_input()))
      printf("DEC Supplemental Graphic");
    else if (strlen(report) > 3
     && !strncmp(report, "!uA", 3)
     && !strcmp(report+3, st_input()))
      printf("ISO Latin-1 supplemental");
    else
      printf(" unknown");
  } else {
    printf(" failed");
  }

  restore_ttymodes();
  cup(max_lines-1,1);
  return MENU_HOLD;
}

/* Test soft terminal-reset - Vt400 */
static int
tst_DECSTR(MENU_ARGS)
{
  return not_impl(PASS_ARGS);
}

static int
tst_DECTABSR(MENU_ARGS)
{
  return any_decrqpsr(PASS_ARGS, 1);
}

static int
tst_DECUDK(MENU_ARGS)
{
  int key;

  static struct {
    int code;
    char *name;
  } keytable[] = {
    /* xterm programs these: */
    { 11, "F1" },
    { 12, "F2" },
    { 13, "F3" },
    { 14, "F4" },
    { 15, "F5" },
    /* vt420 programs these: */
    { 17, "F6" },
    { 18, "F7" },
    { 19, "F8" },
    { 20, "F9" },
    { 21, "F10" },
    { 23, "F11" },
    { 24, "F12" },
    { 25, "F13" },
    { 26, "F14" },
    { 28, "F15" },
    { 29, "F16" },
    { 31, "F17" },
    { 32, "F18" },
    { 33, "F19" },
    { 34, "F20" } };

  for (key = 0; key < sizeof(keytable)/sizeof(keytable[0]); key++) {
    char temp[80];
    char *s;
    temp[0] = '\0';
    for (s = keytable[key].name; *s; s++)
      sprintf(temp + strlen(temp), "%02x", *s & 0xff);
    do_dcs("1|%d/%s", keytable[key].code, temp);
  }

  cup(1,1);
  println(the_title);
  println("Press 'q' to quit.  Function keys should echo their labels.");

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  for (;;) {
    char *report = instr();
    if (*report == 'q')
      break;
    cup(4,10);
    el(2);
    chrprint(report);
  }

  do_dcs("0"); /* clear all keys */

  restore_ttymodes();
  cup(max_lines-1,1);
  return MENU_HOLD;
}

static int
tst_DSR_cursor(MENU_ARGS)
{
  return any_DSR(PASS_ARGS, "?6n");
}

static int
tst_DSR_keyboard(MENU_ARGS)
{
  return any_DSR(PASS_ARGS, "?26n");
}

static int
tst_DSR_printer(MENU_ARGS)
{
  return any_DSR(PASS_ARGS, "?15n");
}

static int
tst_DSR_userkeys(MENU_ARGS)
{
  return any_DSR(PASS_ARGS, "?25n");
}

/******************************************************************************/

/*
 * The main vt100 module tests CUP, HVP, CUF, CUB, CUU, CUD
 */
static int
tst_VT420_cursor(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Back Index (DECBI)",                           not_impl },
      { "Test Forward Index (DECFI)",                        not_impl },
      { "Test Pan down (SU)",                                tst_SU },
      { "Test Pan up (SD)",                                  tst_SD_DEC },
      { "Test Vertical Cursor Coupling (DECVCCM)",           not_impl },
      { "Test Page Cursor Coupling (DECPCCM)",               not_impl },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT420 Cursor-Movement Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * The main vt100 module tests AM, LNM, DECKPAM, DECARM, DECAWM
 */
static int
tst_VT420_keyboard_ctl(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Backarrow key (DECBKM)",                       not_impl },
      { "Test Numeric keypad (DECNKM)",                      not_impl },
      { "Test Keyboard usage (DECKBUM)",                     not_impl },
      { "Test Key position (DECKPM)",                        not_impl },
      { "Test Enable Local Functions (DECELF)",              not_impl },
      { "Test Local Function-Key Control (DECLFKC)",         not_impl },
      { "Test Select Modifier-Key Reporting (DECSMKR)",      not_impl }, /* DECEKBD */
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT420 Keyboard-Control Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
tst_VT420_printing(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Printer-Extent mode (DECPEX)",                 not_impl },
      { "Test Print form-feed mode (DECPFF)",                not_impl },
      { "Test Auto-print mode (MC - DEC private mode)",      not_impl },
      { "Test Printer-controller mode (MC)",                 not_impl },
      { "Test Print-page (MC)",                              not_impl },
      { "Test Print composed main-display (MC)",             not_impl },
      { "Test Print all pages (MC)",                         not_impl },
      { "Test Print cursor line (MC)",                       not_impl },
      { "Test Assign/Release-printer (MC)",                  not_impl },
      { "Test Start/Stop-printer (MC)",                      not_impl },
#if 0 /* FIXME: does this imply that when printing, the printer gets SGR's ? */
      { "Test Send Line-Attributes (MC)",                    not_impl },
      { "Test Send Visual Character-Attributes (MC)",        not_impl },
#endif
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT420 Printing-Control Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * These apply only to VT400's
 */
static int
tst_VT420_rectangle(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Change-Attributes in Rectangular Area (DECCARA)", not_impl },
      { "Test Copy Rectangular area (DECCRA)",               not_impl },
      { "Test Erase Rectangular area (DECERA)",              not_impl },
      { "Test Fill Rectangular area (DECFRA)",               not_impl },
      { "Test Reverse-Attributes in Rectangular Area (DECRARA)", not_impl },
      { "Test Selective-Attribute Change Extent (DECSACE)",  not_impl },
      { "Test Selective-Erase Rectangular area (DECSERA)",   not_impl },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT420 Rectangular Area Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/* UDK and rectangle-checksum status are available only on VT400 */

static int
tst_VT420_report_device(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Extended Cursor-Position",                     tst_DSR_cursor },
      { "Test Printer Status",                               tst_DSR_printer },
      { "Test UDK Status",                                   tst_DSR_userkeys },
      { "Test Keyboard Status",                              tst_DSR_keyboard },
      { "Test Macro Space",                                  not_impl },
      { "Test Memory Checksum",                              not_impl },
      { "Test Data Integrity",                               not_impl },
      { "Test Multiple Session Status",                      not_impl },
      { "Test Checksum of Rectangular Area",                 not_impl },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT420 Device Status Reports");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
tst_VT420_report_presentation(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Cursor Information Report (DECCIR)",                tst_DECCIR },
      { "Tab Stop Report (DECTABSR)",                        tst_DECTABSR },
      { "ANSI Mode Report (DECRPM)",                         tst_ISO_DECRPM },
      { "DEC Mode Report (DECRPM)",                          tst_DEC_DECRPM },
      { "Status-String Report (DECRPSS)",                    tst_DECRPSS },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT420 Device Status Reports");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

static int
tst_VT420_reports(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Device Status Reports",                        tst_VT420_report_device },
      { "Test Terminal State Reports",                       not_impl },
      { "Test Presentation State Reports",                   tst_VT420_report_presentation },
      { "Test Window Report (DECRPDE)",                      tst_DECRQDE },
      { "Test User-Preferred Supplemental Set (DECAUPSS)",   tst_DECRQUPSS },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT420 Reports");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

/* DECSASD and DECSSDT are for VT400's only */
static int
tst_VT420_screen(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test Send/Receive mode (SRM)",                      not_impl },
      { "Test Select Number of Lines per Screen (DECSNLS)",  not_impl }, /* FIXME: xterm */
      { "Test Select Active Status Display (DECSASD)",       not_impl },
      { "Test Select Status line type (DECSSDT)",            not_impl },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT420 Screen-Display Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/******************************************************************************/

int
tst_vt420(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Test cursor-movement",                              tst_VT420_cursor },
      { "Test downloading soft-chars (DECDLD)",              not_impl },
      { "Test keyboard-control",                             tst_VT420_keyboard_ctl },
      { "Test macro-definition (DECDMAC)",                   not_impl },
      { "Test printing functions",                           tst_VT420_printing },
      { "Test rectangular area functions",                   tst_VT420_rectangle },
      { "Test reporting functions",                          tst_VT420_reports },
      { "Test screen-display functions",                     tst_VT420_screen },
      { "Test soft terminal-reset",                          tst_DECSTR },
      { "Test user-defined keys (DECUDK)",                   tst_DECUDK },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("VT420 Tests");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

/* $Id: reports.c,v 1.11 1996/08/19 00:15:02 tom Exp $ */

#include <vttest.h>
#include <ttymodes.h>
#include <esc.h>
#include <ctype.h>

static
struct table {
    int key;
    char *msg;
} paritytable[] = {
    { 1, "NONE" },
    { 4, "ODD"  },
    { 5, "EVEN" },
    { -1, "" }
},nbitstable[] = {
    { 1, "8" },
    { 2, "7" },
    { -1,"" }
},speedtable[] = {
    {   0,    "50" },
    {   8,    "75" },
    {  16,   "110" },
    {  24,   "132.5"},
    {  32,   "150" },
    {  40,   "200" },
    {  48,   "300" },
    {  56,   "600" },
    {  64,  "1200" },
    {  72,  "1800" },
    {  80,  "2000" },
    {  88,  "2400" },
    {  96,  "3600" },
    { 104,  "4800" },
    { 112,  "9600" },
    { 120, "19200" },
    { -1, "" }
},operating_level[] = {
    {  61, "VT100 family" },
    {  62, "VT200 family" },
    {  63, "VT300 family" },
    {  64, "VT400 family" },
    {  -1, "" }
},extensions[] = {
    {   1, "132 columns" },				/* vt400 */
    {   2, "printer port" },				/* vt400 */
    {   3, "ReGIS Graphics" },				/* kermit */ 
    {   4, "Sixel Graphics" },				/* kermit */ 
    {   6, "selective erase" },				/* vt400 */
    {   7, "soft character set (DRCS)" },		/* vt400 */
    {   8, "user-defined keys" },			/* vt400 */
    {   9, "national replacement character-sets" },	/* kermit */
    {  13, "local editing mode" },			/* kermit */
    {  15, "DEC technical set" },			/* vt400 */
    {  16, "locator device port" },			/* kermit */
    {  18, "user windows" },				/* vt400 */
    {  19, "two sessions" },				/* vt400 */
    {  21, "horizontal scrolling" },			/* vt400 */
    {  -1, "" }
};

static int
legend(int n, char *input, char *word, char *description)
{
  int i;
  unsigned len = strlen(word);
  for (i = 0; input[i] != 0; i++) {
    if ((i == 0 || !isalpha(input[i-1]))
     && !strncmp(word, input+i, len)) {
      printf("%-8s %-3s = %s\r\n", n ? "" : "Legend:", word, description);
      return n+1;
    }
  }
  return n;
}

static char *
lookup(struct table t[], int k)
{
  int i;
  for (i = 0; t[i].key != -1; i++) {
    if (t[i].key == k) return(t[i].msg);
  }
  return("BAD VALUE");
}

static int
scan_DA(char *str, int *pos)
{
  int save = *pos;
  int value = scanto(str, pos, ';');
  if (value == 0) {
    *pos = save;
    value = scanto(str, pos, 'c');
    if (str[*pos] != '\0')
      value = 0;
  }
  return value;
}

/******************************************************************************/
static int
tst_DA(MENU_ARGS)
{
  int i, found;
  char *report, *cmp;

  static char *attributes[][2] = { /* after CSI */
    { "?1;0c",   "No options (vanilla VT100)" },
    { "?1;1c",   "VT100 with STP" },
    { "?1;2c",   "VT100 with AVO (could be a VT102)" },
    { "?1;3c",   "VT100 with STP and AVO" },
    { "?1;4c",   "VT100 with GPO" },
    { "?1;5c",   "VT100 with STP and GPO" },
    { "?1;6c",   "VT100 with AVO and GPO" },
    { "?1;7c",   "VT100 with STP, AVO and GPO" },
    { "?1;11c",  "VT100 with PP and AVO" },
    { "?1;15c",  "VT100 with PP, GPO and AVO" },
    { "?4;2c",   "VT132 with AVO" },
    { "?4;3c",   "VT132 with AVO and STP" },
    { "?4;6c",   "VT132 with GPO and AVO" },
    { "?4;7c",   "VT132 with GPO, AVO, and STP" },
    { "?4;11c",  "VT132 with PP and AVO" },
    { "?4;15c",  "VT132 with PP, GPO and AVO" },
    { "?6c",     "VT102" },
    { "?7c",     "VT131" },
    { "?12;5c",  "VT125" },           /* VT125 also has ROM version */
    { "?12;7c",  "VT125 with AVO" },  /* number, so this won't work */
    { "?5;0c",   "VK100 (GIGI)" },
    { "?5c",     "VK100 (GIGI)" },
    { "?62;1;2;4;6;8;9;15c",       "VT220" },
    { "?63;1;2;8;9c",              "VT320" },
    { "?63;1;2;4;6;8;9;15c",       "VT320" },
    { "?63;1;3;4;6;8;9;15;16;29c", "DXterm" },
    { "", "" }
  };

  set_tty_raw(TRUE);
  cup(7,1);
  println("Test of Device Attributes report (what are you)");
  cup(8,1);
  da();
  report = instr();
  cup(8,1);
  el(0);
  printf("Report is: ");
  chrprint(report);

  found = FALSE;
  if ((cmp = skip_prefix(csi_input(), report)) != 0) {
    for (i = 0; *attributes[i][0] != '\0'; i++) {
      if (!strcmp(cmp, attributes[i][0])) {
	int n = 0;
	printf(" -- means %s", attributes[i][1]);
	cup(9,1);
	n = legend(n, attributes[i][1], "STP", "Processor Option");
	n = legend(n, attributes[i][1], "AVO", "Advanced Video Option");
	n = legend(n, attributes[i][1], "GPO", "Graphics Processor Option");
	n = legend(n, attributes[i][1], "PP",  "Printer Port");
	found = TRUE;
        break;
      }
    }
  }
  if (!found) { /* this could be a vt200+ with some options disabled */
    if (*cmp == '?') {
      int reportpos = 1;
      int value = scan_DA(cmp, &reportpos);
      printf("%s\n",
        lookup(operating_level, value));
      while ((value = scan_DA(cmp, &reportpos)) != 0)
        printf("%3d = %s\n", value, lookup(extensions, value));
      found = TRUE;
    }
  }
  if (!found)
    printf(" -- Unknown response, refer to the manual");

  restore_ttymodes();
  cup(max_lines-1,1);
  return MENU_HOLD;
}

static int
tst_DECREQTPARM(MENU_ARGS)
{
  int parity, nbits, xspeed, rspeed, clkmul, flags;
  int reportpos;
  char *report, *report2, *cmp;

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);
  cup(2,1);
  println("Test of the \"Request Terminal Parameters\" feature, argument 0.");
  cup(3,1);
  decreqtparm(0);
  report = instr();
  cup(5,1);
  el(0);
  printf("Report is: ");
  chrprint(report);

  if ((cmp = skip_prefix(csi_input(), report)) != 0)
    report = cmp;

  if (strlen(report) < 14
   || report[0] != '2'
   || report[1] != ';')
    println(" -- Bad format");
  else {
    reportpos = 2;
    parity = scanto(report, &reportpos, ';');
    nbits  = scanto(report, &reportpos, ';');
    xspeed = scanto(report, &reportpos, ';');
    rspeed = scanto(report, &reportpos, ';');
    clkmul = scanto(report, &reportpos, ';');
    flags  = scanto(report, &reportpos, 'x');
    if (parity == 0 || nbits == 0 || clkmul == 0) println(" -- Bad format");
    else                                          println(" -- OK");
    printf(
      "This means: Parity %s, %s bits, xmitspeed %s, recvspeed %s.\n",
      lookup(paritytable, parity),
      lookup(nbitstable, nbits),
      lookup(speedtable, xspeed),
      lookup(speedtable, rspeed));
    printf("(CLoCk MULtiplier = %d, STP option flags = %d)\n", clkmul, flags);
  }

  cup(10,1);
  println("Test of the \"Request Terminal Parameters\" feature, argument 1.");
  cup(11,1);
  decreqtparm(1);	/* Does the same as decreqtparm(0), reports "3" */
  report2 = instr();
  cup(13,1);
  el(0);
  printf("Report is: ");
  chrprint(report2);

  if ((cmp = skip_prefix(csi_input(), report2)) != 0)
    report2 = cmp;

  if (strlen(report2) < 1
   || report2[0] != '3')
    println(" -- Bad format");
  else {
    report2[0] = '2';
    if (!strcmp(report,report2)) println(" -- OK");
    else                         println(" -- Bad format");
  }
  cup(max_lines,1);

  restore_ttymodes();
  return MENU_HOLD;
}

static int
tst_DSR(MENU_ARGS)
{
  int found;
  char *report, *cmp;

  set_tty_raw(TRUE);
  ed(2);
  cup(1,1);
  printf("Test of Device Status Report 5 (report terminal status).");
  cup(2,1);
  dsr(5);
  report = instr();
  cup(2,1);
  el(0);
  printf("Report is: ");
  chrprint(report);

  if ((cmp = skip_prefix(csi_input(), report)) != 0)
    found = !strcmp(cmp, "0n") || !strcmp(cmp, "3n");
  else
    found = 0;

  if (found)
    printf(" -- means \"TERMINAL OK\"");
  else
    printf(" -- Unknown response!");

  cup(4,1);
  println("Test of Device Status Report 6 (report cursor position).");
  cup(5,1);
  dsr(6);
  report = instr();
  cup(5,1);
  el(0);
  printf("Report is: ");
  chrprint(report);

  if ((cmp = skip_prefix(csi_input(), report)) != 0)
    found = !strcmp(cmp,"5;1R");
  else
    found = 0;

  if (found)
    printf(" -- OK");
  else
    printf(" -- Unknown response!");

  cup(max_lines-1,1);
  restore_ttymodes();
  return MENU_HOLD;
}

static int
tst_ENQ(MENU_ARGS)
{
  char *report;

  cup(5,1);
  println("This is a test of the ANSWERBACK MESSAGE. (To load the A.B.M.");
  println("see the TEST KEYBOARD part of this program). Below here, the");
  println("current answerback message in your terminal should be");
  println("displayed. Finish this test with RETURN.");
  cup(10,1);

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);
  inflush();
  printf("%c", 5); /* ENQ */
  report = instr();
  cup(10,1);
  chrprint(report);
  cup(12,1);

  restore_ttymodes();
  return MENU_HOLD;
}

static int
tst_NLM(MENU_ARGS)
{
  char *report;

  ed(2);
  cup(1,1);
  println("Test of LineFeed/NewLine mode.");
  cup(3,1);
  sm("20");
  set_tty_crmod(FALSE);
  printf("NewLine mode set. Push the RETURN key: ");
  report = instr();
  cup(4,1);
  el(0);
  chrprint(report);
  if (!strcmp(report, "\015\012")) printf(" -- OK");
  else                             printf(" -- Not expected");
  cup(6,1);
  rm("20");
  printf("NewLine mode reset. Push the RETURN key: ");
  report = instr();
  cup(7,1);
  el(0);
  chrprint(report);
  if (!strcmp(report, "\015")) printf(" -- OK");
  else                         printf(" -- Not expected");
  cup(9,1);

  restore_ttymodes();
  return MENU_HOLD;
}

/******************************************************************************/
int
tst_reports(MENU_ARGS)
{
  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "<ENQ> (AnswerBack Message)",                        tst_ENQ },
      { "SM RM (Set/Reset Mode) - LineFeed / Newline",       tst_NLM },
      { "DSR (Device Status Report)",                        tst_DSR },
      { "DA (Device Attributes)",                            tst_DA },
      { "DECREQTPARM (Request Terminal Parameters)",         tst_DECREQTPARM },
      { "",                                                  0 }
    };

  do {
    ed(2);
    title(0); printf("Terminal Reports/Responses");
    title(2); println("Choose test type:");
  } while (menu(my_menu));
  return MENU_NOHOLD;
}

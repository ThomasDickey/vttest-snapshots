/* $Id: charsets.c,v 1.1 1996/11/26 02:27:31 tom Exp $ */

/*
 * Test character-sets (e.g., SCS control, DECNRCM mode)
 */
#include <vttest.h>
#include <esc.h>

typedef enum {
  ASCII = 1,
  British = 2,
  Flemish = 3,
  French_Canadian = 4,
  Danish = 5,
  Finnish = 6,
  German = 7,
  Dutch = 8,
  Italian = 9,
  Swiss_French = 10,
  Swiss_German = 11,
  Swedish = 12,
  Norwegian_Danish = 13,
  French = 14,
  Spanish = 15,
  Portugese = 16,
  Hebrew = 17,
  DEC_Spec_Graphic,
  DEC_Supp,
  DEC_Supp_Graphic,
  DEC_Tech,
  Unknown
} National;

static const struct {
  National code;
  int allow96;
  char *final;
  char *name;
} KnownCharsets[] = {
  { ASCII,             0, "B",    "US ASCII" },
  { British,           1, "A",    "British/Latin-1" },
  { DEC_Spec_Graphic,  0, "0",    "DEC Special Graphics" },
  { DEC_Supp,          0, "<",    "DEC Supplemental" },
  { DEC_Supp_Graphic,  0, "%5",   "DEC Supplemental Graphic" },
  { DEC_Tech,          0, ">",    "DEC Technical" },
  { Danish,            1, "?",    "Danish" },
  { Dutch,             1, "4",    "Dutch" },
  { Finnish,           1, "C",    "Finnish" },
  { Flemish,           1, "?",    "Flemish" },
  { French,            1, "R",    "French" },
  { French_Canadian,   1, "Q",    "French Canadian" },
  { German,            1, "K",    "German" },
  { Hebrew,            1, "%=",   "Hebrew" },
  { Italian,           1, "Y",    "Italian" },
  { Norwegian_Danish,  1, "E",    "Norwegian/Danish" },
  { Portugese,         1, "g",    "Portugese" },
  { Spanish,           1, "Z",    "Spanish" },
  { Swedish,           1, "H",    "Swedish" },
  { Swiss_French,      1, "?",    "Swiss (French)" },
  { Swiss_German,      1, "?",    "Swiss (German)" },
  { Unknown,           0, "?",    "Unknown" }
};

static int current_Gx[4];

static void
do_scs(int g)
{
  int n = current_Gx[g];
  char buffer[80];

  sprintf(buffer, "%c%s",
    (KnownCharsets[n].allow96)
      ? "?-./"[g]
      : "()*+"[g],
    KnownCharsets[n].final);
  esc(buffer);
}

static int
lookupCode(National code)
{
  int n;
  for (n = 0; n < TABLESIZE(KnownCharsets); n++) {
    if (KnownCharsets[n].code == code)
      return n;
  }
  return lookupCode(ASCII);
}

static int
reset_charset(MENU_ARGS)
{
  int n;

  for (n = 0; n < 4; n++) {
    current_Gx[n] = lookupCode(n < 2 ? ASCII : DEC_Supp_Graphic);
    do_scs(n);
  }
  return MENU_NOHOLD;
}

static int the_code;

static int
lookup_Gx(MENU_ARGS)
{
  int n;
  the_code = -1;
  for (n = 0; n < TABLESIZE(KnownCharsets); n++) {
    if (!strcmp(the_title, KnownCharsets[n].name)) {
      the_code = n;
      break;
    }
  }
  return MENU_NOHOLD;
}

static void
specify_any_Gx(int g)
{
  MENU my_menu[TABLESIZE(KnownCharsets)+2];
  int n, m;

  for (n = m = 0; n < TABLESIZE(KnownCharsets); n++) {
    if (!strcmp(KnownCharsets[n].final, "?"))
      continue;
    if (g == 0 && KnownCharsets[n].allow96)
      continue;
    my_menu[m].description = KnownCharsets[n].name;
    my_menu[m].dispatch = lookup_Gx;
    m++;
  }
  my_menu[m].description = "";
  my_menu[m].dispatch = 0;

  do {
    vt_clear(2);
    title(0); println("Choose character-set:");
  } while (menu(my_menu) && the_code < 0);

  current_Gx[g] = the_code;
}

static int
specify_G0(MENU_ARGS)
{
  specify_any_Gx(0);
  return MENU_NOHOLD;
}

static int
specify_G1(MENU_ARGS)
{
  specify_any_Gx(1);
  return MENU_NOHOLD;
}

static int
specify_G2(MENU_ARGS)
{
  specify_any_Gx(2);
  return MENU_NOHOLD;
}

static int
specify_G3(MENU_ARGS)
{
  specify_any_Gx(3);
  return MENU_NOHOLD;
}

static int
tst_vt100_charsets(MENU_ARGS)
{
  /* Test of:
     SCS    (Select character Set)
  */
  static const struct { char code; char *msg; } table[] = {
    { 'A', "UK / national" },
    { 'B', "US ASCII" },
    { '0', "Special graphics and line drawing" },
    { '1', "Alternate character ROM standard characters" },
    { '2', "Alternate character ROM special graphics" },
  };

  int i, j, g, cset;

  cup(1,10); printf("Selected as G0 (with SI)");
  cup(1,48); printf("Selected as G1 (with SO)");
  for (cset = 0; cset < TABLESIZE(table); cset++) {
    scs(1,'B');
    cup(3 + 4 * cset, 1);
    sgr("1");
    printf("Character set %c (%s)", table[cset].code, table[cset].msg);
    sgr("0");
    for (g = 0; g <= 1; g++) {
      scs(g, table[cset].code);
      for (i = 1; i <= 3; i++) {
        cup(3 + 4 * cset + i, 10 + 38 * g);
        for (j = 0; j <= 31; j++) {
          printf("%c", i * 32 + j);
        }
      }
    }
  }
  scs(1,'B');
  cup(max_lines,1); printf("These are the installed character sets. ");
  return MENU_HOLD;
}

static int
tst_vt220_locking(MENU_ARGS)
{
  /* Test of:
     SCS    (Select character Set)
  */
  static const struct { int upper; char *code; char *msg; } table[] = {
    { 1, "~", "G1 into GR (LS1R)" },
    { 0, "n", "G2 into GL (LS2)"  },
    { 1, "}", "G2 into GR (LS2R)" },
    { 0, "o", "G3 into GL (LS3)"  },
    { 1, "|", "G3 into GR (LS3R)" },
  };

  int i, j, cset;

  cup(1,10); printf("Locking shifts:");
  for (cset = 0; cset < TABLESIZE(table); cset++) {
    scs(1,'B');
    cup(3 + 4 * cset, 1);
    sgr("1");
    printf("Maps %s", table[cset].msg);
    sgr("0");
    esc(table[cset].code);
    for (i = 1; i <= 3; i++) {
      cup(3 + 4 * cset + i, 10);
      for (j = 0; j <= 31; j++) {
        printf("%c", table[cset].upper * 128 + i * 32 + j);
      }
    }
  }
  scs(1,'B');
  cup(max_lines,1);
  return MENU_HOLD;
}

static int
tst_vt220_single(MENU_ARGS)
{
  return not_impl(PASS_ARGS);
}

/******************************************************************************/

/* Reset G0 to ASCII */
void
scs_normal(void)
{
  scs(0,'B');
}

/* Set G0 to Line Graphics */
void
scs_graphics(void)
{
  scs(0,'0');
}

int
tst_characters(MENU_ARGS)
{
  static char whatis_Gx[4][80];

  static MENU my_menu[] = {
      { "Exit",                                              0 },
      { "Reset (ASCII for G0, G1)",                          reset_charset },
      { whatis_Gx[0],                                        specify_G0 },
      { whatis_Gx[1],                                        specify_G1 },
      { whatis_Gx[2],                                        specify_G2 },
      { whatis_Gx[3],                                        specify_G3 },
      { "Test VT100 Character Sets",                         tst_vt100_charsets },
      { "Test VT220 Locking Shifts",                         tst_vt220_locking },
      { "Test VT220 Single Shifts",                          tst_vt220_single },
      { "",                                                  0 }
  };
  int n;

  if (get_level() > 1 || input_8bits || output_8bits) {
    do {
      vt_clear(2);
      title(0); printf("Character-Set Tests");
      title(2); println("Choose test type:");
      for (n = 0; n < 4; n++) {
        sprintf(whatis_Gx[n], "Specify G%d (now %s)",
            n, KnownCharsets[current_Gx[n]].name);
      }
    } while (menu(my_menu));
    return MENU_NOHOLD;
  } else {
    return tst_vt100_charsets(PASS_ARGS);
  }
}

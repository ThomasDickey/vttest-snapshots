/* $Id: charsets.c,v 1.107 2024/12/05 00:38:44 tom Exp $ */

/*
 * Test character-sets (e.g., SCS control, DECNRCM mode)
 */
#include <vttest.h>
#include <esc.h>
#include <ttymodes.h>

/* the values, where specified, correspond to the keyboard-language codes */
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
  Swiss,
  Swedish = 12,
  Norwegian_Danish = 13,
  French = 14,
  Spanish = 15,
  Portuguese = 16,
  Hebrew = 17,
  British_Latin_1,
  Cyrillic_DEC,
  DEC_Alt_Chars,
  DEC_Alt_Graphics,
  DEC_Spec_Graphic,
  DEC_Supp,
  DEC_Supp_Graphic,
  DEC_Technical,
  DEC_UPSS,
  Greek,
  Greek_DEC,
  Greek_Supp,
  Hebrew_DEC,
  Hebrew_Supp,
  JIS_Roman,
  JIS_Katakana,
  Latin_2_Supp,
  Latin_5_Supp,
  Latin_Cyrillic,
  Russian,
  Turkish,
  Turkish_DEC,
  SCS_NRCS,
  Unknown
} National;

typedef struct {
  National code;                /* internal name (chosen to sort 'name' member) */
  int cs_type;                  /* 0=94-charset, 1=NRCS(94-charset), 2=96-charset */
  int order;                    /* check-column so we can mechanically-sort this table */
  int first;                    /* first model: 0=base, 2=vt220, 3=vt320, etc. */
  int last;                     /* last model: 0=base, 2=vt220, 3=vt320, etc. */
  const char *final;            /* end of SCS string */
  const char *name;             /* the string we'll show the user */
  const char *not11;            /* cells which are not 1-1 with ISO-8859-1 */
} CHARSETS;
/* *INDENT-OFF* */

/* compare mappings using only 7-bits */
#define Only7(c)   ((c) & 0x7f)
#define Not11(a,b) (Only7(a) == Only7(b))

/*
 * User-preferred selection set is unknown until reset.
 */
static National current_upss = Unknown;

/*
 * The "map_XXX" strings list the characters that should be replaced for the
 * given NRCS.  Use that to highlight them for clarity.
 */
static const char map_pound[]        = "#";
static const char map_all94[]        = "!\"#$%&'()*+,-./0123456789:;<=>?"
                                       "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                                       "`abcdefghijklmnopqrstuvwxyz{|}~";
static const char map_all96[]        = " !\"#$%&'()*+,-./0123456789:;<=>?"
                                       "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                                       "`abcdefghijklmnopqrstuvwxyz{|}~\177";
/* National Replacement Character Sets */
static const char map_DEC_Supp[]     = "$&,-./48>GHIJKLMNOPW^pw}~";
static const char map_Spec_Graphic[] = "`abcdefghijklmnopqrstuvwxyz{|}~";
static const char map_Supp_Graphic[] = "$&(,-./48>PW]^pw}~\177";
static const char map_Dutch[]        = "#@[\\]{|}~";
static const char map_Finnish[]      = "[\\]^`{|}~";
static const char map_French[]       = "#@[\\]{|}~";
static const char map_French_Canadian[] = "@[\\]^`{|}~";
static const char map_German[]       = "@[\\]{|}~";
static const char map_Greek[]        = "abcdefghijklmnopqrstuvwxyz";
static const char map_Hebrew[]       = "`abcdefghijklmnopqrstuvwxyz";
static const char map_Italian[]      = "#@[\\]`{|}~";
static const char map_Norwegian[]    = "@[\\]^`{|}~";
static const char map_Portuguese[]   = "[\\]{|}";
static const char map_Russian[]      = "`abcdefghijklmnopqrstuvwxyz{|}~";
static const char map_SCS[]          = "@[\\]^`{|}~";
static const char map_Spanish[]      = "#@[\\]{|}";
static const char map_Swedish[]      = "@[\\]^`{|}~";
static const char map_Swiss[]        = "#@[\\]^_`{|}~";
static const char map_Turkish[]      = "&@[\\]^`{|}~";

/* DEC pre-ISO 94-Character Sets */
static const char map_DEC_Greek[]    = "$&,-./48>"
                                       "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                                       "`abcdefghijklmnopqrstuvwxyz{|}~";
static const char map_DEC_Hebrew[]   = "$&,-./48>"
                                       "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                                       "`abcdefghijklmnopqrstuvwxyz{|}~";
static const char map_DEC_Turkish[]  = "$&,-./48>?"
                                       "PW]^"
                                       "pw~";

/* DEC VT382 94-character sets */
static const char map_JIS_Roman[]    = "\\~";
static const char map_JIS_Katakana[] = "!\"#$%&'()*+,-./0123456789:;<=>?"
                                       "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";

/* ISO 96-Character Sets */
static const char map_ISO_Greek[]    = "!\"$%*./45689:<>?"
                                       "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                                       "`abcdefghijklmnopqrstuvwxyz{|}~\177";
static const char map_ISO_Hebrew[]   = "!*:?"
                                       "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
                                       "`abcdefghijklmnopqrstuvwxyz{|}~\177";
static const char map_ISO_Latin_2[]  = "!\"#%&)*+,./1235679:;<=>?"
                                       "@CEFHJLOPQRUXY[^"
                                       "`cefhjlopqruxy{~\177";
static const char map_ISO_Latin_5[]  = "P]^p}~";

static const CHARSETS KnownCharsets[] = {
  { ASCII,             0, 0, 0, 9, "B",    "US ASCII", NULL },
  { British,           1, 0, 0, 9, "A",    "British", map_pound },
  { British_Latin_1,   2, 0, 3, 9, "A",    "ISO Latin-1", NULL },
  { Cyrillic_DEC,      1, 0, 5, 9, "&4",   "Cyrillic (DEC)", map_all94 },
  { DEC_Spec_Graphic,  0, 0, 0, 9, "0",    "DEC Special graphics and line drawing", map_Spec_Graphic },
  { DEC_Alt_Chars,     0, 0, 0, 0, "1",    "DEC Alternate character ROM standard characters", NULL },
  { DEC_Alt_Graphics,  0, 0, 0, 0, "2",    "DEC Alternate character ROM special graphics", NULL },
  { DEC_Supp,          0, 0, 2, 2, "<",    "DEC Supplemental", map_DEC_Supp },
  { DEC_UPSS,          0, 0, 3, 9, "<",    "User-preferred supplemental", NULL },
  { DEC_Supp_Graphic,  1, 0, 3, 9, "%5",   "DEC Supplemental Graphic", map_Supp_Graphic },
  { DEC_Technical,     0, 0, 3, 9, ">",    "DEC Technical", map_all94 },
  { Dutch,             1, 0, 2, 9, "4",    "Dutch", map_Dutch },
  { Finnish,           1, 0, 2, 9, "5",    "Finnish", map_Finnish },
  { Finnish,           1, 1, 2, 9, "C",    "Finnish", map_French },
  { French,            1, 0, 2, 9, "R",    "French", map_French },
  { French,            1, 1, 2, 9, "f",    "French", map_French }, /* Kermit (vt340 model?) */
  { French_Canadian,   1, 0, 2, 9, "Q",    "French Canadian", map_French_Canadian },
  { French_Canadian,   1, 1, 3, 9, "9",    "French Canadian", map_French_Canadian },
  { German,            1, 0, 2, 9, "K",    "German", map_German },
  { Greek,             1, 0, 5, 9, "\">",  "Greek", map_Greek },
  { Greek_DEC,         1, 0, 5, 9, "\"?",  "Greek (DEC)", map_DEC_Greek },
  { Greek_Supp,        2, 0, 5, 9, "F",    "ISO Greek Supplemental", map_ISO_Greek },
  { Hebrew,            1, 0, 5, 9, "%=",   "Hebrew", map_Hebrew },
  { Hebrew_DEC,        1, 0, 5, 9, "\"4",  "Hebrew (DEC)", map_DEC_Hebrew },
  { Hebrew_Supp,       2, 0, 5, 9, "H",    "ISO Hebrew Supplemental", map_ISO_Hebrew },
  { Italian,           1, 0, 2, 9, "Y",    "Italian", map_Italian },
  { JIS_Roman,         0, 0, 3, 3, "J",    "JIS-Roman", map_JIS_Roman },
  { JIS_Katakana,      0, 0, 3, 3, "I",    "JIS-Katakana", map_JIS_Katakana },
  { Latin_2_Supp,      2, 0, 5, 9, "B",    "ISO Latin-2 Supplemental", map_ISO_Latin_2 },
  { Latin_5_Supp,      2, 0, 5, 9, "M",    "ISO Latin-5 Supplemental", map_ISO_Latin_5 },
  { Latin_Cyrillic,    2, 0, 5, 9, "L",    "ISO Latin-Cyrillic", map_all96 },
  { Norwegian_Danish,  1, 0, 3, 9, "`",    "Norwegian/Danish", map_Norwegian },
  { Norwegian_Danish,  1, 1, 2, 9, "E",    "Norwegian/Danish", map_Norwegian },
  { Norwegian_Danish,  1, 2, 2, 9, "6",    "Norwegian/Danish", map_Norwegian },
  { Portuguese,        1, 0, 3, 9, "%6",   "Portuguese", map_Portuguese },
  { Russian,           1, 0, 5, 9, "&5",   "Russian", map_Russian },
  { SCS_NRCS,          1, 0, 5, 9, "%3",   "SCS", map_SCS },
  { Spanish,           1, 0, 2, 9, "Z",    "Spanish", map_Spanish },
  { Swedish,           1, 0, 2, 9, "7",    "Swedish", map_Swedish },
  { Swedish,           1, 1, 2, 9, "H",    "Swedish", map_Swedish },
  { Swiss,             1, 0, 2, 9, "=",    "Swiss", map_Swiss },
  { Turkish,           1, 0, 5, 9, "%2",   "Turkish", map_Turkish },
  { Turkish_DEC,       1, 0, 5, 9, "%0",   "Turkish (DEC)", map_DEC_Turkish },
  { Unknown,           0, 0,-1,-1, "?",    "Unknown", NULL }
};
/* *INDENT-ON* */

static int hilite_not11;

static char sgr_hilite[10];
static char sgr_reset[10];

int scs_national;               /* true if NRCS should be tested */
int current_Gx[4];

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

static const CHARSETS *
lookupCharset(int g, int n)
{
  const CHARSETS *result = NULL;
  if (n >= 0 && n < TABLESIZE(KnownCharsets)) {
    if (!strcmp(KnownCharsets[n].final, "A")) {
      if (scs_national || (g == 0)) {
        n = lookupCode(British);
      } else {
        n = lookupCode(British_Latin_1);
      }
    }
    result = &KnownCharsets[n];
  }
  return result;
}

static const char *
code2name(National code)
{
  int n = lookupCode(code);
  return KnownCharsets[n].name;
}

const char *
charset_name(int g, int n)
{
  return lookupCharset(g, n)->name;
}

static int
append_sgr(char *buffer, int used, const char *sgr_string)
{
  strcpy(buffer + used, sgr_string);
  used += (int) strlen(sgr_string);
  return used;
}

static void
send32(int row, int upper, const char *not11)
{
  int col;
  int used = 0;
  int hilited = 0;
  char buffer[33 * 8];

  if (LOG_ENABLED) {
    fprintf(log_fp, NOTE_STR "send32 row %d, upper %d, not11:", row, upper);
    if (not11) {
      int lo = -1;
      int hi = -1;
      int nn;
      for (nn = 0; not11[nn]; ++nn) {
        int cc = Only7(not11[nn]);
        if (lo < 0) {
          lo = cc;
        } else if (cc > hi + 1) {   /* a gap after the existing range? */
          if (lo == hi)
            fprintf(log_fp, " %d", hi);
          else
            fprintf(log_fp, " %d-%d", lo, hi);
          lo = cc;
        }
        hi = cc;
      }
      if (lo > 0 && hi > 0) {
        if (lo == hi)
          fprintf(log_fp, " %d", hi);
        else
          fprintf(log_fp, " %d-%d", lo, hi);
      }
    }
    fprintf(log_fp, "\n");
  }
  for (col = 0; col <= 31; col++) {
    char ch = (char) (row * 32 + upper + col);
    if (not11 != NULL && hilite_not11) {
      const char *p;
      int found = 0;
      for (p = not11; *p; ++p) {
        if (Not11(*p, ch)) {
          found = 1;
          break;
        }
      }
      if (found) {
        if (!hilited) {
          used = append_sgr(buffer, used, sgr_hilite);
          hilited = 1;
        }
      } else {
        if (hilited) {
          used = append_sgr(buffer, used, sgr_reset);
          hilited = 0;
        }
      }
    }
    buffer[used++] = ch;
  }
  if (hilited) {
    used = append_sgr(buffer, used, sgr_reset);
  }
  buffer[used] = 0;
  cprintf("%s", buffer);
}

static char *
scs_params(char *dst, int g)
{
  const CHARSETS *tbl = lookupCharset(g, current_Gx[g]);

  sprintf(dst, "%c%s",
          (((g != 0) && (tbl->cs_type == 2) && (get_level() > 2))
           ? "?-./"[g]
           : "()*+"[g]),
          tbl->final);
  return dst;
}

int
do_scs(int g)
{
  int rc = 0;
  char buffer[80];

  scs_params(buffer, g);
  /* "?" is a placeholder: you cannot designate a 96-character set into G0 */
  if (*buffer != '?') {
    esc(buffer);
    rc = 1;
  }
  return rc;
}

/* reset given Gg back to sane setting */
int
sane_cs(int g)
{
  return lookupCode(((g == 0) || (get_level() <= 1))
                    ? ASCII
                    : (get_level() < 3
                       ? British
                       : British_Latin_1));   /* ...to get 8-bit codes 128-255 */
}

/* reset given Gg back to sane setting */
static int
reset_scs(int g)
{
  return do_scs(sane_cs(g));
}

/* reset all of the Gn to sane settings */
int
reset_charset(MENU_ARGS)
{
  int n;

  scs_national = FALSE;
  decnrcm(scs_national);
  for (n = 0; n < 4; n++) {
    int m = sane_cs(n);
    if (m != current_Gx[n] || (m == 0)) {
      int save = current_Gx[n];
      current_Gx[n] = n ? m : 0;
      if (!do_scs(n)) {
        current_Gx[n] = save;
      }
    }
  }
  return MENU_NOHOLD;
}

static int the_code;
static int the_list[TABLESIZE(KnownCharsets) + 2];

static int
lookup_Gx(MENU_ARGS)
{
  int n;
  the_code = -1;
  for (n = 0; n < TABLESIZE(KnownCharsets); n++) {
    if (the_list[n]
        && !strcmp(the_title, KnownCharsets[n].name)) {
      the_code = n;
      break;
    }
  }
  return MENU_NOHOLD;
}

static void
specify_any_Gx(MENU_ARGS, int g)
{
  MENU my_menu[TABLESIZE(KnownCharsets) + 2];
  int n, m;

  /*
   * Build up a menu of the character sets we will allow the user to specify.
   * There are a couple of tentative table entries (the "?" ones), which we
   * won't show in any event.  Beyond that, we limit some of the character sets
   * based on the emulation level (vt220 implements national replacement
   * character sets, for example, but not the 96-character ISO Latin-1).
   */
  for (n = m = 0; n < TABLESIZE(KnownCharsets); n++) {
    the_list[n] = 0;
    if (!strcmp(KnownCharsets[n].final, "?"))
      continue;
    if (get_level() < KnownCharsets[n].first)
      continue;
    if (get_level() > KnownCharsets[n].last)
      continue;
    if (scs_national && (KnownCharsets[n].cs_type == 2))
      continue;
    if (!scs_national && (KnownCharsets[n].cs_type == 1))
      continue;
    if (m && !strcmp(my_menu[m - 1].description, KnownCharsets[n].name))
      continue;
    my_menu[m].description = KnownCharsets[n].name;
    my_menu[m].dispatch = lookup_Gx;
    the_list[n] = 1;
    m++;
  }
  my_menu[m].description = "";
  my_menu[m].dispatch = NULL;

  do {
    vt_clear(2);
    __(title(0), println(the_title));
    __(title(2), println("Choose character-set:"));
  } while (menu(my_menu) && the_code < 0);

  current_Gx[g] = the_code;
}

static int
toggle_hilite(MENU_ARGS)
{
  hilite_not11 = !hilite_not11;
  if (hilite_not11) {
    sprintf(sgr_hilite, "%s7m", csi_output());
    sprintf(sgr_reset, "%sm", csi_output());
  }
  return MENU_NOHOLD;
}

int
toggle_nrc(MENU_ARGS)
{
  scs_national = !scs_national;
  decnrcm(scs_national);
  return MENU_NOHOLD;
}

int
specify_G0(MENU_ARGS)
{
  specify_any_Gx(PASS_ARGS, 0);
  return MENU_NOHOLD;
}

int
specify_G1(MENU_ARGS)
{
  specify_any_Gx(PASS_ARGS, 1);
  return MENU_NOHOLD;
}

int
specify_G2(MENU_ARGS)
{
  specify_any_Gx(PASS_ARGS, 2);
  return MENU_NOHOLD;
}

int
specify_G3(MENU_ARGS)
{
  specify_any_Gx(PASS_ARGS, 3);
  return MENU_NOHOLD;
}

static int
tst_layout(MENU_ARGS)
{
  char buffer[80];
  return tst_keyboard_layout(scs_params(buffer, 0));
}

static int
tst_vt100_charsets(MENU_ARGS)
{
  /* Test of:
   * SCS    (Select character Set)
   */
  int i, g, count, cset;

  __(cup(1, 10), printxx("Selected as G0 (with SI)"));
  __(cup(1, 48), printxx("Selected as G1 (with SO)"));
  for (count = cset = 0; count < TABLESIZE(KnownCharsets); count++) {
    const CHARSETS *tbl = KnownCharsets + count;
    if (tbl->first == 0) {
      int row = 3 + (4 * cset);

      scs(1, 'B');
      cup(row, 1);
      sgr("1");
      tprintf("Character set %s (%s)", tbl->final, tbl->name);
      sgr("0");
      for (g = 0; g <= 1; g++) {
        int set_nrc = (get_level() >= 2 && tbl->final[0] == 'A');
        if (set_nrc)
          decnrcm(TRUE);
        scs(g, (int) tbl->final[0]);
        for (i = 1; i <= 3; i++) {
          cup(row + i, 10 + 38 * g);
          send32(i, 0, tbl->not11);
        }
        if (set_nrc != scs_national)
          decnrcm(scs_national);
      }
      ++cset;
    }
  }
  scs_normal();
  __(cup(max_lines, 1), printxx("These are the installed character sets. "));
  return MENU_HOLD;
}

static int
tst_shift_in_out(MENU_ARGS)
{
  /* Test of:
     SCS    (Select character Set)
   */
  static const char *label[] =
  {
    "Selected as G0 (with SI)",
    "Selected as G1 (with SO)"
  };
  int i, cset;
  char buffer[80];

  __(cup(1, 10), printxx("These are the G0 and G1 character sets."));
  for (cset = 0; cset < 2; cset++) {
    const CHARSETS *tbl = lookupCharset(cset, current_Gx[cset]);
    int row = 3 + (4 * cset);

    scs(cset, 'B');
    cup(row, 1);
    sgr("1");
    tprintf("Character set %s (%s)", tbl->final, tbl->name);
    sgr("0");

    cup(row, 48);
    tprintf("%s", label[cset]);

    esc(scs_params(buffer, cset));
    for (i = 1; i <= 3; i++) {
      cup(row + i, 10);
      send32(i, 0, tbl->not11);
    }
    scs(cset, 'B');
  }
  cup(max_lines, 1);
  scs_normal();
  return MENU_HOLD;
}

#define map_g1_to_gr() esc("~")   /* LS1R */

static int
tst_vt220_locking(MENU_ARGS)
{
  /* *INDENT-OFF* */
  static const struct {
    int upper;
    int mapped;
    const char *code;
    const char *msg;
  } table[] = {
    { 1, 1, "~", "G1 into GR (LS1R)" },
    { 0, 2, "n", "G2 into GL (LS2)"  }, /* "{" vi */
    { 1, 2, "}", "G2 into GR (LS2R)" },
    { 0, 3, "o", "G3 into GL (LS3)"  },
    { 1, 3, "|", "G3 into GR (LS3R)" },
  };
  /* *INDENT-ON* */

  int i, cset;

  __(cup(1, 10), tprintf("Locking shifts, with NRC %s:",
                         STR_ENABLED(scs_national)));
  for (cset = 0; cset < TABLESIZE(table); cset++) {
    int row = 3 + (4 * cset);
    int map = table[cset].mapped;
    const CHARSETS *tbl = lookupCharset(map, current_Gx[map]);
    int map_gl = (strstr(table[cset].msg, "into GL") != NULL);

    scs_normal();
    cup(row, 1);
    sgr("1");
    tprintf("Character set %s (%s) in G%d", tbl->final, tbl->name, map);
    sgr("0");

    cup(row, 48);
    tprintf("Maps %s", table[cset].msg);

    for (i = 1; i <= 3; i++) {
      if (table[cset].upper) {
        scs_normal();
        map_g1_to_gr();
      } else {
        do_scs(map);
        esc(table[cset].code);
      }
      cup(row + i, 5);
      send32(i, 0, map_gl ? tbl->not11 : NULL);

      if (table[cset].upper) {
        do_scs(map);
        esc(table[cset].code);
      } else {
        scs_normal();
        /*
         * That set G1 to ASCII, just like G0.  While that works for VT100 (a
         * 7-bit device), the later 8-bit models use Latin-1 for the default
         * value of G1.
         */
        switch (get_level()) {
        case 0:
        case 1:
          break;
        case 2:
          esc(")A");  /* select the 94-character NRCS, closest to MCS */
          break;
        default:
          esc("-A");  /* select the 96-character set */
          break;
        }
        map_g1_to_gr();
      }
      cup(row + i, 40);
      send32(i, 128, map_gl ? NULL : tbl->not11);
    }
    reset_scs(cset);
  }
  scs_normal();
  cup(max_lines, 1);
  return MENU_HOLD;
}

static int
tst_vt220_single(MENU_ARGS)
{
  int pass, x, y;

  for (pass = 0; pass < 2; pass++) {
    int g = pass + 2;
    const CHARSETS *tbl = lookupCharset(g, current_Gx[g]);

    vt_clear(2);
    cup(1, 1);
    tprintf("Testing single-shift G%d into GL (SS%d) with NRC %s\n",
            g, g, STR_ENABLED(scs_national));
    tprintf("G%d is %s", g, tbl->name);

    do_scs(g);
    for (y = 0; y < 16; y++) {
      for (x = 0; x < 6; x++) {
        int ch = y + (x * 16) + 32;
        int hilited = 0;

        cup(y + 5, (x * 12) + 5);
        tprintf("%3d: (", ch);
        esc(pass ? "O" : "N");  /* SS3 or SS2 */
        if (tbl->not11 && hilite_not11) {
          const char *p;
          for (p = tbl->not11; *p; ++p) {
            if (Not11(*p, ch)) {
              cprintf("%s", sgr_hilite);
              hilited = 1;
              break;
            }
          }
        }
        tprintf("%c", ch);
        if (ch == 127 && (tbl->cs_type != 2))
          tprintf(" ");   /* DEL should have been eaten - skip past */
        if (hilited) {
          cprintf("%s", sgr_reset);
        }
        tprintf(")");
      }
    }

    cup(max_lines, 1);
    holdit();
  }

  return MENU_NOHOLD;
}

/******************************************************************************/

/*
 * For parsing DECCIR response.  The end of the response consists of so-called
 * intermediate and final bytes as used by the SCS controls.  Most of the
 * strings fit into that description, but note that '<', '=' and '>' do not,
 * since they are used to denote private parameters rather than final bytes.
 * (But ECMA-48 hedges this by stating that the format in those cases is not
 * specified).
 */
const char *
parse_Sdesig(const char *source, int *offset)
{
  int j;
  const char *first = source + (*offset);
  const char *result = NULL;
  size_t limit = strlen(first);

  for (j = 0; j < TABLESIZE(KnownCharsets); ++j) {
    if (KnownCharsets[j].code != Unknown) {
      size_t check = strlen(KnownCharsets[j].final);
      if (check <= limit
          && (strcmp(KnownCharsets[j].final, "A")
              || (scs_national != (KnownCharsets[j].cs_type == 2)))
          && !strncmp(KnownCharsets[j].final, first, check)) {
        result = KnownCharsets[j].name;
        *offset += (int) check;
        break;
      }
    }
  }
  if (result == NULL) {
    static char temp[80];
    sprintf(temp, "? %#x\n", *source);
    *offset += 1;
    result = temp;
  }
  return result;
}

/*
 * Lookup character set name for DECRQUPSS.
 */
const char *
parse_upss_name(const char *source, int size)
{
  const char *result = NULL;
  int j;
  int allow96 = (size == 96);

  for (j = 0; j < TABLESIZE(KnownCharsets); ++j) {
    if (allow96 != (KnownCharsets[j].cs_type == 2))
      continue;
    if (!strcmp(source, KnownCharsets[j].final)) {
      result = KnownCharsets[j].name;
      current_upss = KnownCharsets[j].code;
      break;
    }
  }
  /*
   * Work around a contradiction between DEC 070 and the reference manuals.
   */
  if (result == NULL
      && allow96
      && (!strcmp(source, "A")
          || !strcmp(source, "B"))) {
    result = parse_upss_name(source, 94);
  }
  return result;
}

/*
 * Reset G0 to ASCII
 * Reset G1 to ASCII
 * Shift-in.
 */
void
scs_normal(void)
{
  scs(0, 'B');
}

/*
 * Set G0 to Line Graphics
 * Reset G1 to ASCII
 * Shift-in.
 */
void
scs_graphics(void)
{
  scs(0, '0');
}

static void
assign_upss(National code, int allow96)
{
  if (code != current_upss) {
    int n;
    for (n = 0; n < TABLESIZE(KnownCharsets); ++n) {
      if (KnownCharsets[n].code == code) {
        if (get_level() >= KnownCharsets[n].first
            && get_level() <= KnownCharsets[n].last) {
          set_tty_raw(TRUE);
          set_tty_echo(FALSE);
          do_dcs("%d!u%s", allow96, KnownCharsets[n].final);
          current_upss = code;
        }
        break;
      }
    }
  }
  restore_ttymodes();
}

static int
reset_upss(MENU_ARGS)
{
  assign_upss(DEC_Supp_Graphic, 0);
  return MENU_NOHOLD;
}

/* Test Assign User-Preferred Supplemental Set - VT320 and up */
static int
tst_DECAUPSS(MENU_ARGS)
{
  MENU my_menu[TABLESIZE(KnownCharsets) + 2];
  int n, m;
  int level = get_level();

  /*
   * Build up a menu of the character sets we will allow the user to specify.
   */
  for (n = m = 0; n < TABLESIZE(KnownCharsets); n++) {
    the_list[n] = 0;
    if (!strcmp(KnownCharsets[n].final, "?"))
      continue;
    if (KnownCharsets[n].cs_type != 2)
      switch (KnownCharsets[n].code) {
      case ASCII: /* undocumented */
      case DEC_Spec_Graphic:    /* undocumented */
      case DEC_Supp_Graphic:
      case DEC_Technical: /* undocumented */
      case Greek_DEC:
      case Hebrew_DEC:
      case Turkish_DEC:
      case Cyrillic_DEC:
        break;
      default:
        continue;
      }
    if (KnownCharsets[n].first == 2)  /* exclude NRCS */
      continue;
    if (level < KnownCharsets[n].first)
      continue;
    if (level > KnownCharsets[n].last)
      continue;
    if (m && !strcmp(my_menu[m - 1].description, KnownCharsets[n].name))
      continue;
    my_menu[m].description = KnownCharsets[n].name;
    my_menu[m].dispatch = lookup_Gx;
    the_list[n] = 1;
    m++;
  }
  my_menu[m].description = "";
  my_menu[m].dispatch = NULL;

  do {
    vt_clear(2);
    __(title(0), println(the_title));
    __(title(2), println("Choose character-set:"));
  } while (menu(my_menu) && the_code < 0);

  assign_upss(KnownCharsets[the_code].code,
              KnownCharsets[the_code].cs_type == 2);
  return MENU_HOLD;
}

/* Test Report User-Preferred Supplemental Set - VT320 and up */
static int
tst_DECRQUPSS(MENU_ARGS)
{
  int row, col;
  char *report;
  const char *show;
  char buffer[80];

  __(vt_move(1, 1), println("Testing DECRQUPSS Window Report"));

  set_tty_raw(TRUE);
  set_tty_echo(FALSE);

  do_csi("&u");
  report = get_reply();
  vt_move(row = 3, col = 10);
  chrprint2(report, row, col);
  if ((report = skip_dcs(report)) != NULL
      && strip_terminator(report)) {
    int cs_size;
    const char *cs_name;

    if (*report == '0')
      cs_size = 94;
    else if (*report == '1')
      cs_size = 96;
    else
      cs_size = -1;
    show = "unknown";
    if (cs_size > 0
        && *++report == '!'
        && *++report == 'u'
        && (cs_name = parse_upss_name(++report, cs_size)) != NULL) {
      sprintf(buffer, "%s (%d characters)", cs_name, cs_size);
      show = buffer;
    }
  } else {
    show = SHOW_FAILURE;
  }
  show_result("%s", show);

  restore_ttymodes();
  vt_move(max_lines - 1, 1);
  return MENU_HOLD;
}

int
tst_characters(MENU_ARGS)
{
  static char whatis_Gx[4][80];
  static char hilite_mesg[80];
  static char nrc_mesg[80];
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              NULL },
      { "Reset (G0 ASCII, G1 Latin-1, no NRC mode)",         reset_charset },
      { hilite_mesg,                                         toggle_hilite },
      { nrc_mesg,                                            toggle_nrc },
      { whatis_Gx[0],                                        specify_G0 },
      { whatis_Gx[1],                                        specify_G1 },
      { whatis_Gx[2],                                        specify_G2 },
      { whatis_Gx[3],                                        specify_G3 },
      { "Test VT100 Character Sets",                         tst_vt100_charsets },
      { "Test Shift In/Shift Out (SI/SO)",                   tst_shift_in_out },
      { "Test VT220 Locking Shifts",                         tst_vt220_locking },
      { "Test VT220 Single Shifts",                          tst_vt220_single },
      { "Test Soft Character Sets",                          not_impl },
      { "Test Keyboard Layout with G0 Selection",            tst_layout },
      { "",                                                  NULL }
  };
  /* *INDENT-ON* */

  hilite_not11 = 1;
  toggle_hilite(PASS_ARGS);
  reset_charset(PASS_ARGS);   /* make the menu consistent */

  if (get_level() > 1 || input_8bits || output_8bits) {
    int n;

    do {
      vt_clear(2);
      __(title(0), printxx("Character-Set Tests"));
      __(title(2), println("Choose test type:"));
      sprintf(hilite_mesg, "%s highlighting of non-ISO-8859-1 mapping",
              STR_ENABLE(hilite_not11));
      sprintf(nrc_mesg, "%s National Replacement Character (NRC) mode",
              STR_ENABLE(scs_national));
      for (n = 0; n < 4; n++) {
        sprintf(whatis_Gx[n], "Specify G%d (now %s)",
                n, charset_name(n, current_Gx[n]));
      }
    } while (menu(my_menu));
    /* tidy in case a "vt100" emulator does not ignore SCS */
    vt_clear(1);
    return reset_charset(PASS_ARGS);
  } else {
    return tst_vt100_charsets(PASS_ARGS);
  }
}

int
tst_upss(MENU_ARGS)
{
  static char upss_mesg[120];
  /* *INDENT-OFF* */
  static MENU my_menu[] = {
      { "Exit",                                              NULL },
      { "Test character sets",                               tst_characters },
      { "Reset UPSS (User-Preferred Supplemental Sets)",     reset_upss },
      { upss_mesg,                                           tst_DECAUPSS },
      { "Test DECRQUPSS",                                    tst_DECRQUPSS },
      { "",                                                  NULL }
  };
  /* *INDENT-ON* */

  hilite_not11 = 1;
  toggle_hilite(PASS_ARGS);
  reset_charset(PASS_ARGS);   /* make the menu consistent */

  if (get_level() > 2) {
    do {
      sprintf(upss_mesg,
              "Assign UPSS (now %s)",
              code2name(current_upss));
      vt_clear(2);
      __(title(0), printxx("User-Preferred Supplemental Sets Tests"));
      __(title(2), println("Choose test type:"));
    } while (menu(my_menu));
    /* tidy in case a "vt100" emulator does not ignore SCS */
    vt_clear(1);
    return reset_upss(PASS_ARGS);
  } else {
    return tst_characters(PASS_ARGS);
  }
}

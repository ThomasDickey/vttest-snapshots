/* $Id: vt52.c,v 1.11 1996/10/30 02:05:08 tom Exp $ */

#include <vttest.h>
#include <ttymodes.h>
#include <esc.h>

int
tst_vt52(MENU_ARGS)
{
  static struct rtabl {
      char *rcode;
      char *rmsg;
  } resptable[] = {
      { "\033/K", " -- OK (means Standard VT52)" },
      { "\033/Z", " -- OK (means VT100 emulating VT52)" },
      { "",       " -- Unknown response"}
  };

  int i,j;
  char *response;
  VTLEVEL save;

  save_level(&save);
  set_level(0);  /* Reset ANSI (VT100) mode, Set VT52 mode  */
  vt52home();    /* Cursor home     */
  vt52ed();      /* Erase to end of screen  */
  vt52home();    /* Cursor home     */
  for (i = 0; i <= max_lines-1; i++) {
    for (j = 0; j <= 9; j++)
    printf("%s", "FooBar ");
    println("Bletch");
  }
  vt52home();  /* Cursor home     */
  vt52ed();    /* Erase to end of screen  */

  vt52cup(7,47);
  printf("nothing more.");
  for (i = 1; i <= 10; i++) printf("THIS SHOULD GO AWAY! ");
  for (i = 1; i <= 5; i++) {
    vt52cup(1,1);
    printf("%s", "Back scroll (this should go away)");
    vt52ri();           /* Reverse LineFeed (with backscroll!)  */
  }
  vt52cup(12,60);
  vt52ed();    /* Erase to end of screen  */
  for (i = 2; i <= 6; i++) {
    vt52cup(i,1);
    vt52el();           /* Erase to end of line */
  }

  for (i = 2; i <= max_lines-1; i++) {
    vt52cup(i,70); printf("%s", "**Foobar");
  }
  vt52cup(max_lines-1,10);
  for (i = max_lines-1; i >= 2; i--) {
    printf("%s", "*");
    printf("%c", 8);    /* BS */
    vt52ri();           /* Reverse LineFeed (LineStarve)        */
  }
  vt52cup(1,70);
  for (i = 70; i >= 10; i--) {
    printf("%s", "*");
    vt52cub1(); vt52cub1(); /* Cursor Left */
  }
  vt52cup(max_lines,10);
  for (i = 10; i <= 70; i++) {
    printf("%s", "*");
    printf("%c", 8);    /* BS */
    vt52cuf1();         /* Cursor Right */
  }
  vt52cup(2,11);
  for (i = 2; i <= max_lines-1; i++) {
    printf("%s", "!");
    printf("%c", 8);    /* BS */
    vt52cud1();         /* Cursor Down  */
  }
  vt52cup(max_lines-1,69);
  for (i = max_lines-1; i >= 2; i--) {
    printf("%s", "!");
    printf("%c", 8);    /* BS */
    vt52cuu1();         /* Cursor Up    */
  }
  for (i = 2; i <= max_lines-1; i++) {
    vt52cup(i,71);
    vt52el();           /* Erase to end of line */
  }

  vt52cup(10,16);
  printf("%s", "The screen should be cleared, and have a centered");
  vt52cup(11,16);
  printf("%s", "rectangle of \"*\"s with \"!\"s on the inside to the");
  vt52cup(12,16);
  printf("%s", "left and right. Only this, and");
  vt52cup(13,16);
  holdit();

  vt52home();  /* Cursor home     */
  vt52ed();  /* Erase to end of screen  */
  printf("%s", "This is the normal character set:");
  for (j =  0; j <=  1; j++) {
    vt52cup(3 + j, 16);
    for (i = 0; i <= 47; i++)
    printf("%c", 32 + i + 48 * j);
  }
  vt52cup(6,1);
  printf("%s", "This is the special graphics character set:");
  esc("F");     /* Select Special Graphics character set        */
  for (j =  0; j <=  1; j++) {
    vt52cup(8 + j, 16);
    for (i = 0; i <= 47; i++)
    printf("%c", 32 + i + 48 * j);
  }
  esc("G");     /* Select ASCII character set   */
  vt52cup(12,1);
  holdit();

  vt52home();  /* Cursor home     */
  vt52ed();    /* Erase to end of screen  */
  println("Test of terminal response to IDENTIFY command");

  /*
   * According to J.Altman, DECID isn't recognized by VT5xx terminals.  Real
   * DEC terminals through VT420 do, though it isn't recommended.  VT420's
   * emulation of VT52 does not recognize DA -- so we use DECID in this case.
   */
  set_tty_raw(TRUE);
  decid();     /* Identify     */
  response = get_reply();
  println("");

  restore_level(&save);
  restore_ttymodes();
  padding(10);  /* some terminals miss part of the 'chrprint()' otherwise */

  printf("Response was");
  chrprint(response);
  for(i = 0; resptable[i].rcode[0] != '\0'; i++) {
    if (!strcmp(response, resptable[i].rcode)) {
      show_result("%s", resptable[i].rmsg);
      break;
    }
  }
  println("");
  println("");

  /*
   * Verify whether returning to ANSI mode restores the previous operating
   * level.  If it was a VT220, we can check this by seeing if 8-bit controls
   * work; if a VT420 we can check the value of DECSCL.
   */
  if (terminal_id() >= 200) {
    set_level(0);  /* Reset ANSI (VT100) mode, Set VT52 mode  */
    println("Verify operating level after restoring ANSI mode");
    esc("<");    /* Enter ANSI mode (VT100 mode) */
    set_tty_raw(TRUE);
    if (save.cur_level >= 3) { /* VT340 implements DECRQSS */
      printf("Testing DECSCL ");
      decrqss("\"p");
      response = get_reply();
      chrprint(response);
      println("");
      if (parse_decrqss(response, "\"p") > 0)
        printf("MAYBE:%s\n", response);
      else
        printf("NO:%s\n", response);
    }
    if (save.cur_level >= 2) {
      vt_move(10,1);
      printf("Testing S8C1T ");
      s8c1t(1);
      cup(1,1); dsr(6);
      response = instr();
      vt_move(10,15);
      chrprint(response);
      if ((response = skip_prefix(csi_input(), response)) != 0
       && !strcmp("1;1R", response))
        printf(SHOW_SUCCESS);
    }
    restore_level(&save);
    restore_ttymodes();
    println("");
    println("");
  }
  return MENU_HOLD;
}

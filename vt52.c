/* $Id: vt52.c,v 1.7 1996/10/28 00:52:31 tom Exp $ */

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
    esc("I");           /* Reverse LineFeed (with backscroll!)  */
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
    esc("I");           /* Reverse LineFeed (LineStarve)        */
  }
  vt52cup(1,70);
  for (i = 70; i >= 10; i--) {
    printf("%s", "*");
    esc("D"); esc("D"); /* Cursor Left */
  }
  vt52cup(max_lines,10);
  for (i = 10; i <= 70; i++) {
    printf("%s", "*");
    printf("%c", 8);    /* BS */
    esc("C");           /* Cursor Right */
  }
  vt52cup(2,11);
  for (i = 2; i <= max_lines-1; i++) {
    printf("%s", "!");
    printf("%c", 8);    /* BS */
    esc("B");           /* Cursor Down  */
  }
  vt52cup(max_lines-1,69);
  for (i = max_lines-1; i >= 2; i--) {
    printf("%s", "!");
    printf("%c", 8);    /* BS */
    esc("A");           /* Cursor Up    */
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
   * According to J.Altman, DECID isn't recognized by VT5xx terminals. However,
   * real DEC terminals through VT420 do, though it isn't recommended.
   */
  set_tty_raw(TRUE);
  if (terminal_id() < 200) {
    esc("Z");     /* Identify     */
  } else {
    da();
  }
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
  return MENU_HOLD;
}

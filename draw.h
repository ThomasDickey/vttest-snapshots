/* $Id: draw.h,v 1.2 2004/08/01 22:51:46 tom Exp $ */

typedef struct {
  int top;
  int left;
  int bottom;
  int right;
} BOX;

extern int make_box_params(BOX *box, int vmargin, int hmargin);
extern void draw_box_outline(BOX *box, int mark);
extern void draw_box_filled(BOX *box, int mark);
extern void draw_box_caption(BOX *box, int margin, char **c);

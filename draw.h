/* $Id: draw.h,v 1.5 2024/10/02 00:12:34 tom Exp $ */

typedef struct {
  int top;
  int left;
  int bottom;
  int right;
} BOX;

extern int make_box_params(BOX *box, int vmargin, int hmargin);
extern void draw_box_outline(const BOX *box, int mark);
extern void draw_box_filled(const BOX *box, int mark);
extern void draw_box_caption(const BOX *box, int margin, const char **c);
extern void ruler(int row, int width);

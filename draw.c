/* $Id: draw.c,v 1.6 2006/11/26 18:29:18 tom Exp $ */

#include <vttest.h>
#include <draw.h>
#include <esc.h>

/*
 * Useful drawing functions for vttest.
 */

static int
check_box_params(BOX *box)
{
  if (box->top >= box->bottom || box->left >= box->right) {
    printf("The screen is too small for box with margins %d,%d.", box->top, box->left);
    holdit();
    return -1;
  }
  return 0;
}

/*
 * Compute box params for given vertical and horizontal margin,
 * returns -1 if the screen is too small, 0 otherwise.
 */
int
make_box_params(BOX *box, int vmargin, int hmargin)
{
  box->top = vmargin;
  box->bottom = max_lines - vmargin;
  box->left = hmargin;
  box->right = min_cols - hmargin;
  return check_box_params(box);
}

void
draw_box_outline(BOX *box, int mark)
{
  int j;

  for (j = box->top; j <= box->bottom; j++) {
    __(cup(j, box->left), putchar(mark));
    __(cup(j, box->right), putchar(mark));
  }
  cup(box->top, box->left);
  for (j = box->left; j < box->right; j++)
    putchar(mark);
  cup(box->bottom, box->left);
  for (j = box->left; j < box->right; j++)
    putchar(mark);
}

void
draw_box_filled(BOX *box, int mark)
{
  int i, j;

  for (i = box->top; i < box->bottom; i++) {
    cup(i, box->left);
    for (j = box->left; j < box->right; j++) {
      putchar(mark);
    }
  }
}

static int
next_word(char *s)
{
  char *base;
  while (*s == ' ')
    s++;
  base = s;
  while (*s && *s != ' ')
    s++;
  return (int) (s - base);
}

void
draw_box_caption(BOX *box, int margin, char **c)
{
  int x0 = (box->left + margin);
  int y0 = (box->top + margin);
  int x1 = (box->right - margin);
  int y1 = (box->bottom - margin);
  int x = x0;
  int y = y0;
  int t;
  char *s;

  while ((s = *c++) != 0) {
    while ((t = *s++) != 0) {
      if (x == x0) {
        if (t == ' ')
          continue;
        cup(y, x++);
        putchar(' ');
      }
      putchar(t);
      x++;
      if ((t == ' ') && (next_word(s) > (x1 - x - 2))) {
        while (x < x1) {
          putchar(' ');
          x++;
        }
      }
      if (x >= x1) {
        putchar(' ');
        x = x0;
        y++;
      }
    }
  }
  while (y <= y1) {
    if (x == x0) {
      cup(y, x);
    }
    putchar(' ');
    if (++x >= x1) {
      putchar(' ');
      x = x0;
      y++;
    }
  }
}

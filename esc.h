/* $Id: esc.h,v 1.5 1996/08/03 19:09:45 tom Exp $ */

#ifndef ESC_H
#define ESC_H 1

/* esc.c */
char *instr(void);
char inchar(void);
void brc(int pn, int c);
void brc2(int pn1, int pn2, int c);
void brc3(int pn1, int pn2, int pn3, int c);
void brcstr(char *ps, int c);
void cbt(int pn);
void cha(int pn);
void cht(int pn);
void cnl(int pn);
void cpl(int pn);
void cub(int pn);
void cud(int pn);
void cuf(int pn);
void cup(int pn1, int pn2);
void cuu(int pn);
void da(void);
void dch(int pn);
void decaln(void);
void decdhl(int lower);
void decdwl(void);
void deckpam(void);
void deckpnm(void);
void decll(char *ps);
void decrc(void);
void decreqtparm(int pn);
void decsc(void);
void decsca(int pn1);
void decsed(int pn1);
void decsel(int pn1);
void decstbm(int pn1, int pn2);
void decswl(void);
void dectst(int pn);
void dl(int pn);
void dsr(int pn);
void ech(int pn);
void ed(int pn);
void el(int pn);
void esc(char *s);
void holdit(void);
void hpa(int pn);
void hts(void);
void hvp(int pn1, int pn2);
void ich(int pn);
void il(int pn);
void ind(void);
void inflush(void);
void inputline(char *s);
void nel(void);
void println(char *s);
void readnl(void);
void ri(void);
void ris(void);
void rm(char *ps);
void scs(int g, int c);
void sgr(char *ps);
void sl(int pn);
void sm(char *ps);
void sr(int pn);
void tbc(int pn);
void trmop(int fc, int arg);
void ttybin(int bin);
void vpa(int pn);
void vt52cup(int l, int c);
void zleep(int t);

#endif /* ESC_H */

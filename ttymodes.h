/* $Id: ttymodes.h,v 1.2 1996/06/18 19:17:44 tom Exp $ */

#ifndef TTYMODES_H
#define TTYMODES_H 1

#ifdef DEBUG
void log_ttymodes(char *file, int line);
void dump_ttymodes(char *tag, int flag);
#else
#define log_ttymodes(file, line) /*nothing*/
#define dump_ttymodes(tag, flag) /*nothing*/
#endif

void init_ttymodes(int pn);
void restore_ttymodes(void);
void set_tty_crmod(int enabled);
void set_tty_echo(int enabled);
void set_tty_raw(int enabled);

#endif /* TTYMODES_H */

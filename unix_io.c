/* $Id: unix_io.c,v 1.39 2024/10/20 17:18:13 tom Exp $ */

#include <stdarg.h>
#include <unistd.h>
#include <vttest.h>
#include <esc.h>

static void
give_up(SIG_ARGS GCC_UNUSED)
{
  if (LOG_ENABLED) {
    fprintf(log_fp, "** killing program due to timeout\n");
    fflush(log_fp);
  }
  kill(getpid(), (int) SIGTERM);
}

static int last_char;

void
reset_inchar(void)
{
  last_char = -1;
}

/*
 * Wait until a character is typed on the terminal then read it, without
 * waiting for CR.
 */
char
inchar(void)
{
  int lval;
  int ch;
  char one_byte = '\0';
  int active = is_replaying();

  fflush(stdout);
  lval = last_char;
  brkrd = FALSE;
  reading = TRUE;
#ifdef HAVE_ALARM
  signal(SIGALRM, give_up);
  alarm(60);    /* timeout after 1 minute, in case user's keyboard is hung */
#endif
  if (read(0, &one_byte, (size_t) 1) < 0)
    ch = EOF;
  else
    ch = (int) one_byte;
#ifdef HAVE_ALARM
  alarm(0);
#endif
  reading = FALSE;
#ifdef DEBUG
  {
    FILE *fp = fopen("ttymodes.log", "a");
    if (fp != 0) {
      fprintf(fp, "%d>%#x\n", brkrd, ch);
      fclose(fp);
    }
  }
#endif
  if (brkrd)
    last_char = 0177;
  else
    last_char = ch;
  if ((last_char == 0177) && (last_char == lval))
    give_up(SIGTERM);

  if (active)
    pause_replay();
  if (LOG_ENABLED) {
    fputs(READ_STR, log_fp);
    put_char(log_fp, last_char);
    fputs("\n", log_fp);
  }
  if (active)
    resume_replay();
  return (char) (last_char);
}

static int
read_buffer(char *result, int want)
{
#if USE_FIONREAD
  int l1;
#endif
  int i = 0;

/* Wait 0.1 seconds (1 second in vanilla UNIX) */
  zleep(100);
  fflush(stdout);
#ifdef HAVE_RDCHK
  while (rdchk(0)) {
    read(0, result + i, 1);
    if (i++ >= want)
      break;
  }
#else
#if USE_FIONREAD
  while (ioctl(0, FIONREAD, &l1), l1 > 0L) {
    while (l1-- > 0L) {
      if ((int) read(0, result + i, (size_t) 1) <= 0)
        goto out1;
      if (i++ >= want)
        goto out1;
    }
  }
out1:
#else
  while (read(2, result + i, 1) == 1)
    if (i++ >= want)
      break;
#endif
#endif
  result[i] = '\0';

  return i;
}

/*
 * Get an unfinished string from the terminal:  wait until a character is typed
 * on the terminal, then read it, and all other available characters.  Return a
 * pointer to that string.
 */
char *
instr(void)
{
  static char result[BUF_SIZE];

  FILE *save = log_fp;
  int i = 0;

  pause_replay();
  log_fp = NULL;
  result[i++] = inchar();
  (void) read_buffer(result + i, (int) sizeof(result) - 2);
  log_fp = save;

  if (LOG_ENABLED) {
    fputs(READ_STR, log_fp);
    put_string(log_fp, result);
    fputs("\n", log_fp);
  }
  resume_replay();

  return (result);
}

/* cf: vms_io.c */
char *
get_reply(void)
{
  static char result[BUF_SIZE * 2];
  int old_len = 0;
  int new_len = 0;

  fflush(stdout);
  pause_replay();

  zleep(100);
  do {
    new_len = read_buffer(result + old_len, (int) sizeof(result) - 2 - old_len);
    old_len += new_len;
  } while (new_len != 0 && old_len < (BUF_SIZE - 2));

  if (LOG_ENABLED) {
    fputs(READ_STR, log_fp);
    put_string(log_fp, result);
    fputs("\n", log_fp);
  }

  resume_replay();

  return (result);
}

/*
 * Read to the next newline, truncating the buffer at BUF_SIZE-1 characters
 */
void
inputline(char *s)
{
  char *result;

  if (is_replaying() && (result = replay_string()) != NULL) {
    strcpy(s, result);
    puts(result);
    fflush(stdout);
    zleep(1000);
  } else {
    do {
      int ch;
      char *d = s;
      result = s;
      while ((ch = getchar()) != EOF && ch != '\n') {
        if ((d - s) < BUF_SIZE - 2)
          *d++ = (char) ch;
      }
      *d = 0;
    } while (!*s);
  }

  if (LOG_ENABLED) {
    fputs(READ_STR, log_fp);
    put_string(log_fp, result);
    fputs("\n", log_fp);
  }
}

/*
 * Flush input buffer, make sure no pending input character
 */
void
inflush(void)
{
  int val;

#ifdef HAVE_RDCHK
  while (rdchk(0))
    read(0, &val, 1);
#else
#if USE_FIONREAD
  int l1;
  ioctl(0, FIONREAD, &l1);
  while (l1-- > 0L) {
    if ((int) read(0, &val, (size_t) 1) <= 0)
      break;
  }
#else
  while (read(2, &val, (size_t) 1) > 0) ;
#endif
#endif
}

void
holdit(void)
{
  inflush();
  tprintf("Push <RETURN>");
  readnl();
}

void
readnl(void)
{
  char *result;

  if (is_replaying() && (result = replay_string()) != NULL) {
    puts(result);
    fflush(stdout);
    zleep(1000);
  } else {
    int ch = '\0';
    char one_byte = '\0';

    fflush(stdout);
    brkrd = FALSE;
    reading = TRUE;
    do {
      if (read(0, &one_byte, (size_t) 1) < 0) {
        break;
      } else {
        ch = (int) one_byte;
      }
    } while (ch != '\n' && !brkrd);
    if (brkrd)
      give_up(SIGTERM);
    reading = FALSE;
  }

  if (LOG_ENABLED) {
    fputs(READ_STR "\n", log_fp);
  }
}

/*
 * Sleep and do nothing (don't waste CPU) for t milliseconds
 */
void
zleep(int t)
{
#ifdef HAVE_USLEEP
  unsigned msecs = (unsigned) (t * 1000);
  usleep(msecs);
#else
  unsigned secs = (unsigned) (t / 1000);
  if (secs == 0)
    secs = 1;
  sleep(secs);  /* UNIX can only sleep whole seconds */
#endif
}

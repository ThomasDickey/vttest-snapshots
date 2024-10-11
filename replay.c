/*
 * $Id: replay.c,v 1.4 2024/10/11 00:10:48 tom Exp $
 */
#include <vttest.h>

static FILE *rp;
static int paused;
static int marker;
static char *buffer;
static size_t length;
static unsigned lineno;

void
setup_replay(const char *pathname)
{
  close_replay();
  rp = fopen(pathname, "rb");
}

void
close_replay(void)
{
  if (rp != NULL) {
    fclose(rp);
    rp = NULL;
    paused = 0;
    marker = 0;
    lineno = 0;
    free(buffer);
    length = 0;
  }
}

int
is_replaying(void)
{
  return (rp != NULL && !paused);
}

/* Read the "next" line, skipping data, notes, etc., to return a "Read" line */
static char *
next_line(void)
{
  char *result = NULL;
  if (rp != NULL) {
    char *buf;
    char *tmp;
    size_t now = 0;

    if (buffer == NULL)
      buffer = malloc(length = BUF_SIZE);
    buf = buffer;

    while ((tmp = fgets(buf, BUF_SIZE, rp)) != NULL) {
      size_t len = strlen(buf);
      if (len != 0) {
        if (buf[len - 1] == '\n')
          break;
        if ((now + BUF_SIZE) >= length) {
          buffer = realloc(buffer, length += BUF_SIZE);
        }
        now += len;
        buf = buffer + now;
      } else {
        if (buf != buffer)  /* just in case the file had no final \n */
          tmp = NULL;
        break;
      }
    }
    if (tmp == NULL)
      close_replay();
    ++lineno;
    if (LOG_ENABLED && debug_level > 1)
      fprintf(log_fp, "[%04u]: %s", lineno, buffer);
    result = buffer;
  }
  return result;
}

static void
skip_to_tag(const char *tag)
{
  size_t need = strlen(tag);
  char *check;
  while ((check = next_line()) != NULL) {
    if (LOG_ENABLED && debug_level > 0 && strncmp(check, SKIP_STR, need))
      fprintf(log_fp, SKIP_STR "%s", check);
    if (!strncmp(check, tag, need)) {
      break;
    }
  }
}

static char *
trim_newline(char *source)
{
  size_t len = strlen(source);
  if (len != 0 && source[len - 1] == '\n')
    source[--len] = '\0';
  return source;
}

/*
 * Returns a single character, not replayable.
 */
int
get_char(void)
{
  int result;
  pause_replay();
  result = inchar();
  resume_replay();
  return result;
}

#define READ_LEN (sizeof(READ_STR) - 1)

/*
 * Returns the string from the next "Read", trimming the newline.
 */
char *
replay_string(void)
{
  char *result = NULL;
  skip_to_tag(READ_STR);
  if (buffer != NULL && !strncmp(buffer, READ_STR, READ_LEN)) {
    result = trim_newline(buffer) + READ_LEN;
  }
  return result;
}

/*
 * Suspend command-replay when waiting for responses from the terminal, as well
 * as unpredictable user-initiated mouse and key events.  The command-file can
 * hold several records; we only care about the "Done" line which happens when
 * replay is resumed.
 */
void
pause_replay(void)
{
  if (is_replaying())
    skip_to_tag(WAIT_STR);
  paused = 1;
  if (LOG_ENABLED) {
    fprintf(log_fp, WAIT_STR "%d\n", ++marker);
    fflush(log_fp);
  }
}

/*
 * Pausing replay caused us to stop reading the command-file.  Skip any unread
 * lines until the "Done" line.
 */
void
resume_replay(void)
{
  if (rp != NULL)
    skip_to_tag(DONE_STR);
  paused = 0;
  if (LOG_ENABLED) {
    fprintf(log_fp, DONE_STR "%d\n", marker);
    fflush(log_fp);
  }
}

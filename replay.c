/*
 * $Id: replay.c,v 1.9 2024/12/05 00:37:39 tom Exp $
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
  lineno = 0;
}

void
close_replay(void)
{
  if (rp != NULL) {
    fclose(rp);
    rp = NULL;
    paused = 0;
    marker = 0;
    free(buffer);
    buffer = NULL;
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
    if (buffer != NULL)
      ++lineno;
    if (LOG_ENABLED && debug_level > 1) {
      fprintf(log_fp, "[%04u]: %s", lineno,
              (tmp && buffer) ? buffer : "<EOF>\n");
    }
    result = buffer;
  }
  return result;
}

static void
skip_to_tag(const char *tag)
{
  size_t need = strlen(tag);
  char *check;
  int skip = 0;
  int jump = !strcmp(tag, WAIT_STR);
  while ((check = next_line()) != NULL) {
    if (LOG_ENABLED && debug_level > 0 && strncmp(check, SKIP_STR, need))
      fprintf(log_fp, SKIP_STR "%s", check);
    if (!jump && !strncmp(check, WAIT_STR, need))
      skip = 1;
    if (!skip && !strncmp(check, tag, need))
      break;
    if (!jump && !strncmp(check, DONE_STR, need))
      skip = 0;
  }
}

/*
 * Reverse the changes made by put_string/put_char
 */
static char *
get_string(char *source)
{
  size_t len = strlen(source);
  char *temp = malloc(len + 1);

  if (len != 0 && source[len - 1] == '\n')
    source[--len] = '\0';

  if (temp != NULL) {
    char *s, *t;
    for (s = source, t = temp; *s != '\0'; ++s) {
      if (*s == '<' && isdigit(s[1])) {
        long value = strtol(++s, &s, 10);
        if (s == NULL)
          break;
        *t++ = (char) (value & 0xff);
      } else if (*s != ' ') {
        *t++ = *s;
      }
    }
    *t = '\0';
    strcpy(source, temp);
    free(temp);
  }
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
    result = get_string(buffer + READ_LEN);
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
  paused++;
  if (LOG_ENABLED) {
    if (paused == 1) {
      fprintf(log_fp, WAIT_STR "%d\n", ++marker);
      fflush(log_fp);
    }
  }
}

/*
 * Pausing replay caused us to stop reading the command-file.  Skip any unread
 * lines until the "Done" line.
 */
void
resume_replay(void)
{
  if (rp != NULL && paused == 1)
    skip_to_tag(DONE_STR);
  if (paused > 0) {
    --paused;
  }
  if (LOG_ENABLED) {
    if (paused == 0) {
      fprintf(log_fp, DONE_STR "%d\n", marker);
      fflush(log_fp);
    }
  }
}

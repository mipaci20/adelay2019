/*
expand_tilde  --  expand ~ and ~user into home directory.

Copyright 1993 by AT&T Bell Laboratories; all rights reserved
*/

#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>   /* strchr */
#include <stdio.h>    /* sprintf */


/*
* Expand ~/file or ~user/file notation.
* Buffer must be large enough to accomodate whole filename.
* Return pointer to buffer.
*/
char *expand_tilde(char *fn, char *buffer)
{
  struct passwd *pw = 0;     /* password entry */
  char *slash;               /* file name after user name */
  uid_t euid;                /* effective user id */

  if (!fn) return 0;
  if (*fn == '~') {
    slash = strchr(fn, '/');
    if (*(fn+1) == '/' || *(fn+1) == '\0') {
      euid = geteuid();
      pw = getpwuid(euid);
    }
    else {
      if (slash) *slash = '\0';
      pw = getpwnam(fn+1);
      if (slash) *slash = '/';
    }
    if (pw) {
      sprintf(buffer, "%s%s", pw->pw_dir, slash ? slash : "");
      return buffer;
    }
    else return 0;
  }
  else strcpy(buffer, fn);
  return fn;
} /* expand_tilde */

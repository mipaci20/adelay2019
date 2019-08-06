/*
strsave  --  saves string in dynamically allocated area of memory.

Copyright 1993 by AT&T Bell Laboratories; all rights reserved
*/
#include <stdlib.h>
#include <string.h>   /* strdup() */

static char rcsid[] = "$Id: strsave.c,v 1.2 1995/07/11 20:21:26 hgs Exp $";

char *strsaven(const char *s, int n)
{
  char *p;

  if (!s) return 0;
  if ((p = (char *)malloc(n+1)) != NULL) {
    memcpy(p, s, n);
    *(p+n) = '\0';
  }
  return p;
} /* strsaven */


/*
* Append 's' to 'd', extending if necessary. Return new string.
*/
char *strappendn(
  char **d,  /* destination string (pointer to pointer!) */
  char *s,   /* string to append */
  int n)     /* length of 's' */
{
  int current;

  if (!s) return *d;
  if (*d == 0) {
    *d = strdup(s);
    return *d;
  }

  current = strlen(*d);
  *d = realloc(*d, current + n + 1);
  memcpy(&(*d)[current], s, n);
  (*d)[current + n] = '\0';
  return *d;
} /* strappendn */


char *strappend(char **d, char *s)
{
  return strappendn(d, s, strlen(s));
} /* strappend */

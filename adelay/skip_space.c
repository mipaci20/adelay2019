/*
skip_space   ---  skip initial space.

Copyright 1993 by AT&T Bell Laboratories; all rights reserved
*/

#include <ctype.h>
#include "string.h"


char *skip_space(char *s)
{
  if (!s) return NULL;
  while (*s && isspace(*s)) s++;
  return s;
} /* skip_space */

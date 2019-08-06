/*
open_file  ---   open FILE, with directory path and extension.

Copyright 1994 by GMD Fokus; all rights reserved
*/

#include <stdio.h>
#include <string.h>  /* strdup() */
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>  /* STDxx_FILENO */


extern char *skip_space(char *);
extern char *expand_tilde(char *, char *);
extern int sflags(char *mode, int *optr);

/*
* Open file.  If name is "", open stdin or stdout, as appropriate.  If
* 'dir_list' is non-zero, treat as path list, containing list of
* candidate directories separated by colons.  Each path is
* tilde-expanded.  'ext' is the default extension (part after dot). 
* If the file name begins with /, ignore directory.  Return name
* chosen as 'chosen' (dynamically allocated).
* Return file descriptor on success; -1 on error.
*   errno == ENOENT indicates invalid characters in file name.
*/
int open_file(char *dir_list, char *name, char *ext, char *mode, char **chosen)
{
printf("Opening file... \n");
  int f = -1;
  char *s;
  char efn[1024], fn_ext[1024];
  int o_flags = 0;

  if (!sflags(mode, &o_flags) || !name){ printf("sflags failed\n"); return -1;}
  if (!ext) ext = "";
  name = skip_space(name);
  if ((!name || name[0] == '\0' || strcmp(name, "-") == 0) &&
      strlen(ext) == 0) {
    if (o_flags & O_RDONLY) f = STDIN_FILENO;
    if (o_flags & O_RDWR || o_flags & O_APPEND) f = STDOUT_FILENO;
  }
  else {
    /* check name */
    for (s = name; *s; s++) {
      if (*s <= ' ' || *s > '~') {
        errno = ENOENT;
printf("Doesn't exist. \n");
        return -1;
      }
    }
    /* if no dir_list, search current directory */
    if (!dir_list) dir_list = "";

    /* check if file already has an extension */
    s = strrchr(name, '.');
    if (s) ext = "";

    /* if file name starts with '/', ignore directory list */
    if (*name == '/') {
      f = open(name, o_flags, 0666);
      if (chosen) *chosen = strdup(name);
    }
    /* otherwise, search through directory list */
    else {
      while (dir_list && f < 0) {
        char *dir = dir_list;
        if ((dir_list = strchr(dir_list, ':'))) *dir_list++ = '\0';
        sprintf(fn_ext, "%s%s%s%s", dir,
          *dir ? "/" : "", name, ext);
        expand_tilde(fn_ext, efn);  /* expand ~ to home directory */
        f = open(efn, o_flags, 0666);
      } /* for */
      if (chosen) *chosen = strdup(efn);
    }
  }
printf("Success on opening file. f = %d\n",f);
  return f;
} /* open_file */

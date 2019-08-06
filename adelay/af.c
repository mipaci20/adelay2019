/*
af  --  reads Sun .au and AIFC files

Copyright 1993 by AT&T Bell Laboratories; all rights reserved
*/

#include <stdio.h>           /* fdopen(), FILE */
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>          /* perror() */
#include "audio_generic.h"   /* audio_descr_t, audio_file_t */
#include "ansi.h"

static char rcsid[] = "$Id: af.c,v 1.4 1995/08/17 13:58:26 hgs Exp $";

extern int snd_read_hdr(), snd_write_hdr(), aiff_read_hdr();

static audio_file_hdr_t af_hdr[] = {
{"Sun/Next", ".au",   snd_read_hdr,  snd_write_hdr},
{"Sun/Next", ".snd",  snd_read_hdr,  snd_write_hdr},
{"AIFF",     ".aiff", aiff_read_hdr, 0},
{"AIFC",     ".afc",  aiff_read_hdr, 0},
{"unknown",  0,       0,             0}
}; 


/*
* Open an audio file and skip header.  If reading, fill in audio_file
* descriptor 'af' from file, for writing, create appropriate header from
* 'af'.  Return file descriptor.
*/
FILE *af_open(
  int fd,              /* I  file to be opened */
  char *mode,          /* I  "r", "w", "a" */
  audio_file_t *af)    /* O  type of sound file */
{
  FILE *f = 0;
  int v;        /* return value: -1: error; -2: wrong type; >= 0 size */
  int i;

  if (fd < 0) return 0;

  if (*mode == 'r') {
    for (i = 0; af_hdr[i].read; i++) {
      af->data_offset = 0;  /* start at beginning of file */
      af->type = i;
      v = (*af_hdr[i].read)(fd, af);
      if (v >= 0) break;
      else if (v == -1){ printf("Read file error\n"); return 0;}
    }
    /* no header decoding function, thus use default values */
    if (af_hdr[i].read == 0) {
      af->ad.encoding         = AE_PCMU;
      af->ad.sample_rate      = 8000;
      af->ad.channels         = 1;
      af->info                = 0;
      af->data_offset         = 0;
      af->data_size           = AUDIO_UNKNOWN_SIZE;
    }
  }
  else if (*mode == 'w') {
    if (af_hdr[(int)af->type].write) {
      (*af_hdr[(int)af->type].write)(fd, af);
    }
  }
  f = fdopen(fd, mode);
printf("f = %p\n",f);
  return f;
} /* af_open */


/*
* Reset audio file 'f' to its beginning, skipping the header. Return 0 if
* successful, -1 if not.
*/
int af_reset(FILE *f, audio_file_t *af)
{
  if (fseek(f, af->data_offset, SEEK_SET) == -1) {
    perror("fseek audio file");
    return -1;
  }
  return 0;
} /* af_reset */


/*
* Open audio file for recording.
* Return file descriptor, zero on error.
*/
FILE *af_record(
  int fd,            /* I  file descriptor */
  char *fn,          /* I  file name (only suffix is used to get type) */
  audio_descr_t *ad, /* I  default audio descriptor */
  char *info         /* I  info field */
)
{
  audio_file_t af;

  af.ad          = *ad;
  af.data_offset = 0;
  af.data_size   = AUDIO_UNKNOWN_SIZE;
  af.type        = AF_other;

  fn = strrchr(fn, '.');
  if (fn && (int)strlen(fn) > 1) {
    for (af.type = 0; af_hdr[af.type].suffix; af.type++) {
      if (af_hdr[af.type].write && strcmp(fn, af_hdr[af.type].suffix) == 0)
        break;
    }
  }  
  if (af.type >= AF_other) return 0;
  af.info = info;

  return af_open(fd, "w", &af);
} /* af_record */

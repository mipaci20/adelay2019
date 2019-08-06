#ifndef _audio_file_h
#define _audio_file_h

#include <stdio.h>
#include "audio_descr.h"

/* we assume that all-ones are not legal values for any of the above */
#define AUDIO_UNDEF ((unsigned)(~0))
#ifndef AUDIO_UNKNOWN_SIZE
#define AUDIO_UNKNOWN_SIZE AUDIO_UNDEF
#endif

/* sound file types */
typedef struct {
  audio_descr_t ad;     /* generic audio description */
  enum {AF_au, AF_snd, AF_aiff, AF_other} type;
  int data_offset;      /* offset of sound data from beginning */
  int data_size;        /* number of sound bytes */
  char *info;           /* textual description */
} audio_file_t;

extern FILE *af_open(int fd, char *mode, audio_file_t *);
extern FILE *af_record(int fd, char *fn, audio_descr_t *ad, char *info);
extern int   af_reset(FILE *f, audio_file_t *af);
#endif

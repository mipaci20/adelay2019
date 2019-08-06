#ifndef _audio_generic_h
#define _audio_generic_h

/*
* System-independent audio device abstractions; we need to deal with
* bytes of less than one byte.  For low-rate, frame oriented codecs,
* it is not clear whether the notion of samples is particularly
* useful.  For LPC, for example, a sample unit is 22.5 ms.
*/

#include "types.h"
#include <stdio.h>       /* FILE */
#include "audio_descr.h"
#include "audio_file.h"

extern long code2rate[];
extern int codec_sample_rate[];
extern short bpu[], spu[];  /* bytes and samples per unit */

/* dynamic audio statistics */
typedef struct {
  struct {
    float peak;         /* short term VU meter (0..1.0) */
    float dc;           /* estimate of DC offset */
  } c[2];               /* per channel */
} audio_stats_t;

typedef struct {
  char *name;                        /* descriptive name */
  char *suffix;                      /* file name suffix */
  int (*read)(int, audio_file_t *);  /* function that reads the header */
  int (*write)(int, audio_file_t *); /* function that writes the header */
} audio_file_hdr_t;

typedef struct {
  char *name;          /* name of codec (for messages) */
  double bps;          /* bytes per sample */
  int bpm;             /* packets per message */
  void *(*init)(void *state, double period);       /* codec initialization */
  int (*encoder)(void *in_buf, int in_size, audio_descr_t *ad,
    void *out_buf, int *out_size, void *state, int hflag); /* encoder */
  int (*decoder)(void *in_buf, int in_size, audio_descr_t *ad,
    void *out_buf, int *out_size, void *state);            /* decoder */
} codec_t;

extern   double   audio_bytes_to_seconds(audio_descr_t *, int);
extern   unsigned audio_bytes_to_samples(audio_descr_t *, int);
extern   void     audio_close(void);
extern   void     audio_close_idle(void);
extern   int      audio_cmp(audio_descr_t *, audio_descr_t *);
extern   unsigned audio_samples_to_bytes(audio_descr_t *, int);
extern   unsigned audio_seconds_to_bytes(audio_descr_t *, double);
extern   int      audio_silence(void *, size_t, audio_encoding_t);
extern   int      audio_stats(void *, size_t, audio_descr_t *, audio_stats_t *);
extern   void     audio_stats_init(void);
extern   char *   audio_ntoa(audio_descr_t *);
extern   void *   gsm_init(void *);
#endif

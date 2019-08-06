/*
snd  ---  read and write Sun/Next .au/.snd file headers

Copyright 1993 by AT&T Bell Laboratories; all rights reserved
*/

#include "types.h"        /* u_int32 */
#include <values.h>       /* MAXINT */
#include <sys/types.h>    /* ntohl() */
#include <unistd.h>       /* SEEK_SET */
#include <stdlib.h>       /* malloc() */
#include <netinet/in.h>   /* ntohl() */
#include <string.h>       /* strlen() */
#include "audio_generic.h"

static char rcsid[] = "$Id: snd.c,v 1.4 1995/08/17 13:58:26 hgs Exp $";

/*
 * Used internally only; from multimedia/audio_filehdr.h;
 * all assumed to be in big endian order?
 * The info following is supposed to be zero-terminated.
 */
typedef struct {
  u_int32 magic;          /* magic number */
  u_int32 hdr_size;       /* size of this header, with info (in bytes) */
  u_int32 data_size;      /* length of data (optional) */
  u_int32 encoding;       /* data encoding format */
  u_int32 sample_rate;    /* samples per second */
  u_int32 channels;       /* number of interleaved channels */
} audio_filehdr_t;

#define AUDIO_FILE_MAGIC 0x2e736e64   /* .snd */
/* Define the encoding fields */
#define AUDIO_FILE_ENCODING_MULAW_8       (1) /* 8-bit ISDN u-law */
#define AUDIO_FILE_ENCODING_LINEAR_8      (2) /* 8-bit linear PCM */
#define AUDIO_FILE_ENCODING_LINEAR_16     (3) /* 16-bit linear PCM */
#define AUDIO_FILE_ENCODING_LINEAR_24     (4) /* 24-bit linear PCM */
#define AUDIO_FILE_ENCODING_LINEAR_32     (5) /* 32-bit linear PCM */
#define AUDIO_FILE_ENCODING_FLOAT         (6) /* 32-bit IEEE floating point */
#define AUDIO_FILE_ENCODING_DOUBLE        (7) /* 64-bit IEEE floating point */
#define AUDIO_FILE_ENCODING_ADPCM_G721   (23) /* 4-bit CCITT g.721 ADPCM */
#define AUDIO_FILE_ENCODING_ADPCM_G722   (24) /* CCITT g.722 ADPCM */
#define AUDIO_FILE_ENCODING_ADPCM_G723_3 (25) /* CCITT g.723 3-bit ADPCM */
#define AUDIO_FILE_ENCODING_ADPCM_G723_5 (26) /* CCITT g.723 5-bit ADPCM */
#define AUDIO_FILE_ENCODING_ALAW_8       (27) /* 8-bit ISDN A-law */

/*
* Read .snd file header.  On read errors (file not readable, etc.), -1
* is returned.  If the magic number is incorrect, -2 is returned and
* the file position is restored to where it was on calling the
* routine.  On success, return 0; on failure, -1.
*/
int snd_read_hdr(int fd, audio_file_t *af)
{
printf("Reading %d...\n", fd);
  audio_filehdr_t hdr;
  int len;

  /* read fixed-size file header */
  af->data_offset = lseek(fd, 0, SEEK_CUR);
int rv = read(fd, &hdr, sizeof(hdr));
  if (rv < sizeof(hdr)){printf("not enough data, size of header = %d, size read = %d \n", sizeof(hdr), rv); return -1;}

  /* check magic number */
  hdr.magic = ntohl(hdr.magic);
  if (hdr.magic != AUDIO_FILE_MAGIC) {
    lseek(fd, af->data_offset, SEEK_SET);
    return -2;
  }
  hdr.encoding  = ntohl(hdr.encoding);
  hdr.hdr_size  = ntohl(hdr.hdr_size);
  hdr.data_size = ntohl(hdr.data_size);

  if (af) {
    af->ad.sample_rate = ntohl(hdr.sample_rate);
    switch(hdr.encoding) {
    case AUDIO_FILE_ENCODING_MULAW_8:
      af->ad.encoding         = AE_PCMU;
      break;
    case AUDIO_FILE_ENCODING_LINEAR_8:
      af->ad.encoding         = AE_L8;
      break;
    case AUDIO_FILE_ENCODING_LINEAR_16:
      af->ad.encoding         = AE_L16;
      break;
    case AUDIO_FILE_ENCODING_LINEAR_32:
      af->ad.encoding         = AE_L32;
      break;
    case AUDIO_FILE_ENCODING_FLOAT:
      af->ad.encoding         = AE_MAX;  /* XXX */
      break;
    case AUDIO_FILE_ENCODING_DOUBLE:
      af->ad.encoding         = AE_MAX;  /* XXX */
      break;
    case AUDIO_FILE_ENCODING_ADPCM_G721:
      af->ad.encoding         = AE_G721;
      break;
    case AUDIO_FILE_ENCODING_ADPCM_G723_3:
      af->ad.encoding         = AE_G723;
      break;
    case AUDIO_FILE_ENCODING_ADPCM_G723_5:
      af->ad.encoding         = AE_G723;
      break;
    }
    af->ad.channels = ntohl(hdr.channels);
  }

  /* read info, if it exists */
  len = hdr.hdr_size - sizeof(hdr);
  if (len < 0){ printf("length is negative\n"); return -1;}
  if ((af->info = malloc(len+1)) == 0) return -1;
  if (len > 0) {
    if (read(fd, af->info, len) < len){printf("sizes don't match, again\n"); return -1;}
  }
  else {
    af->info[0] = '\0';
  }

  af->data_offset = hdr.hdr_size;
  af->data_size   = hdr.data_size;
  return 0;
} /* snd_read_hdr */

/*
* Write .snd header to file descriptor 'fd'.
* Return -1 on error, 0 on success.
*/
int snd_write_hdr(int fd, audio_file_t *af)
{
  audio_filehdr_t hdr;
  int i;
  int info_len = 0;    /* info length */
  static struct {
    u_int32 hdr;
    audio_encoding_t encoding;
  } map[] = {
    {AUDIO_FILE_ENCODING_MULAW_8,      AE_PCMU},
    {AUDIO_FILE_ENCODING_ALAW_8,       AE_PCMA},
    {AUDIO_FILE_ENCODING_LINEAR_8,     AE_L8},
    {AUDIO_FILE_ENCODING_LINEAR_16,    AE_L16},
    {AUDIO_FILE_ENCODING_LINEAR_24,    AE_L24},
    {AUDIO_FILE_ENCODING_LINEAR_32,    AE_L32},
    {AUDIO_FILE_ENCODING_FLOAT,        AE_MAX}, /* XXX */
    {AUDIO_FILE_ENCODING_DOUBLE,       AE_MAX}, /* XXX */
    {AUDIO_FILE_ENCODING_ADPCM_G721,   AE_G721},
    {AUDIO_FILE_ENCODING_ADPCM_G722,   AE_MAX},
    {AUDIO_FILE_ENCODING_ADPCM_G723_3, AE_G723},
    {AUDIO_FILE_ENCODING_ADPCM_G723_5, AE_MAX},
    {AUDIO_UNKNOWN_SIZE, 0}
  };

  if (af->info) info_len = strlen(af->info) + 1;
  hdr.magic       = htonl(AUDIO_FILE_MAGIC);
  hdr.hdr_size    = htonl(sizeof(hdr) + info_len);
  hdr.data_size   = htonl(af->data_size);
  for (i = 0; map[i].hdr != AUDIO_UNKNOWN_SIZE; i++) {
    if (map[i].encoding == af->ad.encoding) {
      hdr.encoding = htonl(map[i].hdr);
      break;
    }
  }
  hdr.sample_rate = htonl(af->ad.sample_rate);
  hdr.channels    = htonl(af->ad.channels);
  if (write(fd, (void *)&hdr, sizeof(hdr)) < sizeof(hdr)) return -1;

  if (write(fd, af->info, info_len) < info_len) return -1;
  return 0;
} /* snd_write_hdr */

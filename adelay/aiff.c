/*
aiff  ---  reads AIFF (audio interchange file format) and AIFF-C files.

Copyright 1993 by AT&T Bell Laboratories; all rights reserved
*/

#include "types.h"        /* u_int32 */
#include "audio_generic.h"
#include "aiff.h"
#include "bit.h"          /* ROUND() */
#include "util.h"         /* strappend() */
#include <unistd.h>       /* SEEK_ */
#include <netinet/in.h>   /* ntohl() */
#include <memory.h>       /* memcpy() */

static char rcsid[] = "$Id: aiff.c,v 1.3 1995/08/17 13:58:26 hgs Exp $";

extern double ConvertFromIeeeExtended(unsigned char *bytes);

/* type definitions (since not all compilers know about multibyte
   character constants); NOTE: byte ordering! */
enum {
  A_c    = 0x28632920,
  A_AIFC = 0x41494643,
  A_AIFF = 0x41494646,
  A_ANNO = 0x414e4e4f,
  A_AUTH = 0x41555448,
  A_COMM = 0x434f4d4d,
  A_COMT = 0x434f4d54,
  A_FVER = 0x46564552,
  A_NAME = 0x4e414d45,
  A_SSND = 0x53534e44,
  A_NONE = 0x4e4f4e45,
  A_ACE2 = 0x41434532,
  A_ACE8 = 0x41434538, 
  A_MAC3 = 0x4d414333,
  A_MAC6 = 0x4d414336
};

/*
* Read data structure common to all chunks.
*/
static int r_chunk(int fd, Chunk *chunk)
{
  return read(fd, chunk, sizeof(Chunk));
} /* r_chunk */


/*
* Assume that the file pointer is positioned just after the common
* chunk header. 'chunk' is converted to host byte order; 'seen'
* contains bytes of chunk already processed.
*/
static void skip_chunk(int fd, Chunk *chunk, int seen)
{
  u_int32 size = chunk->ckDataSize; 

  /* round to even number of bytes */
  size = ROUND(size, int16);
  size -= seen;
  if (size) lseek(fd, size, SEEK_CUR);
} /* skip_chunk */


/*
* Return aligned 32-bit integer.
*/
static long align(char *c)
{
  u_int32 l;
  memcpy((char *)&l, c, sizeof(u_int32));
  return ntohl(l);
} /* align */


/*
* Parse AIFF and AIFC headers.  Leaves file pointer at beginning of
* audio data and fills descriptor structure.  
*  0: success
* -1: invalid version
* -2: not an AIFF file
* -3: unknown AIFC encoding
*/
int aiff_read_hdr(int fd, audio_file_t *af)
{
  Chunk chunk;
  ID formType;
  union {
    char    c[80];
    u_int8  uc[80];
    u_int16 us[40]; 
    u_int32 ul[20];
  } buffer;
  int len;           /* bytes read */
  int i;             /* loop variable */
  int precision;     /* data precision */

  af->info = 0;      /* default: no info available */

  /* read FORM chunk */
  af->data_offset = lseek(fd, 0, SEEK_CUR);
  len = r_chunk(fd, &chunk);
  if (len <= 0) return 0;
  read(fd, &buffer, sizeof(FormAIFCChunk) - sizeof(Chunk));
  formType = buffer.ul[0];
  if (formType != A_AIFF && formType != A_AIFC) {
    lseek(fd, af->data_offset, SEEK_SET);
    return -2; 
  }

  /* read remaining chunks */
  while (1) {
    if (r_chunk(fd, &chunk) <= 0) break;
    chunk.ckDataSize = ntohl(chunk.ckDataSize);

    switch (chunk.ckID) {
    case A_FVER:
      len = read(fd, &buffer, sizeof(FormatVersionChunk) - sizeof(Chunk));
      if (len < sizeof(FormatVersionChunk) - sizeof(Chunk))
        break;
      skip_chunk(fd, &chunk, len);
      i = align(&buffer.c[0]);
      if (i != 0xa2805140) return -1;
      break;

    case A_COMM:
      len = read(fd, &buffer, chunk.ckDataSize);
      if (len < chunk.ckDataSize) break;
      skip_chunk(fd, &chunk, len);
      af->ad.sample_rate = ConvertFromIeeeExtended(&buffer.uc[8]);
      af->ad.channels    = ntohs(buffer.us[0]);
      precision          = ntohs(buffer.us[3]);
      /* handle compression */
      if (formType == A_AIFC && len > 18) {
        i = align(&buffer.c[18]);
        if (i != A_NONE) return -3;
      }
      else { /* AIFF */
        i = A_NONE;
      }
      if (i == A_NONE) {
        if      (precision <=  8) af->ad.encoding = AE_L8;
        else if (precision <= 16) af->ad.encoding = AE_L16;
        else if (precision <= 24) af->ad.encoding = AE_L24;
      }
      break;

    case A_SSND:
      len = read(fd, &buffer, sizeof(SoundDataChunk) - sizeof(Chunk));
      if (len < sizeof(SoundDataChunk) - sizeof(Chunk))
        break;
      af->data_offset = lseek(fd, 0, SEEK_CUR);
      af->data_size = chunk.ckDataSize - 2*sizeof(u_int32);
      skip_chunk(fd, &chunk, len);
      break;

    case A_COMT:  /* comments chunk */
      len = read(fd, &buffer, sizeof(CommentsChunk) - sizeof(Chunk));
      if (len < sizeof(CommentsChunk) - sizeof(Chunk))
        break;
      for (i = 0; i < buffer.us[0]; i++) {
        int count;

        len = read(fd, &buffer, sizeof(Comment)-sizeof(char));
        count = buffer.us[3];
        len = read(fd, &buffer, ROUND(count, int16));
        strappend (&af->info, "COMT: ");
        strappendn(&af->info, &buffer.c[8], count);
      }
      break;

    case A_NAME:
    case A_AUTH:
    case A_c:
    case A_ANNO:
      len = read(fd, &buffer, ROUND(chunk.ckDataSize, int16));
      if (len > 0) {
        strappendn(&af->info, (char *)&chunk.ckID, 4);
        strappend (&af->info, ": ");
        strappendn(&af->info, buffer.c, chunk.ckDataSize);
        strappend (&af->info, ";");
      }
      break;

    default:
      skip_chunk(fd, &chunk, 0);
      break;
    } 
  }
  /* position file pointer at beginning of audio part */
  lseek(fd, af->data_offset, SEEK_SET);
  return 0;
} /* aiff_read_hdr */

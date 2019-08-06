/*
Definitions for AIFF and AIFF-C sound files.
Based on 8/26/91 draft of 'Audio interchange File Format AIFF-C'
Apple Computer, Inc.

All entities are stored in big-endian order. Since the fields in AIFF
chunks are not properly aligned, we cannot simply map a structure onto
the chunk.
*/
#include <sys/types.h>

typedef struct {
  u_char count;
  u_char text[1];
} pstring;

typedef u_int32 ID;  /* 4-character string */
typedef u_int32 OSType;

/* chunks are padded to have an even number of characters; the pad
   byte is NOT included in the ckDataSize
*/
typedef struct {
  ID ckID;            /* chunk ID */
  long ckDataSize;    /* chunk data size, in bytes (w/o ckID and ckDataSize) */
} Chunk;

typedef struct {
  ID ckID;            /* 'FORM' */
  long ckDataSize;
  ID formType;        /* 'AIFC' */
  /* data follows */
} FormAIFCChunk;

typedef struct {
  ID        ckId;            /* 'FVER' */
  long      ckDataSize;      /* 4 */
  u_int32    timestamp;       /* AIFCVersion1 */
} FormatVersionChunk;

/* a maximum of one SoundDataChunk can appear in a FORM AIFC */
typedef struct {
  ID       ckID;             /* 'SSND' */
  long     ckDataSize;
  u_int32   offset;
  u_int32   blockSize;
  /* sound data follows */
} SoundDataChunk;

/* a maximum of one CommentsChunk can appear in a FORM AIFC */
typedef struct {
  ID       ckID;             /* 'COMT' */
  long     ckDataSize;
  u_int16  numComments;
  /* comment(s) follow */
} CommentsChunk;

typedef short MarkerId;
typedef struct {
  u_int32  timestamp;       /* comment creation date */
  MarkerId marker;          /* comments for this marker number */
  u_int16  count;           /* comment text string length */
  char     text[1];         /* comment text */
} Comment;

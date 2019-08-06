/* ********************************************************************
 * File:        aufileutil.c
 * Project:     Audio Delay
 * Module:      Audio File Untilities
 * SubModule:
 * Purpose:     Read audio (.au, .aiff, .snd) data file and decode data
 * Data Format:	AE_PCMU, AE_PCMA, AE_L8 (sign?), AE_L16, AE_L32(sign?)
 * Frequency:	Any
 * Call:        None
 * Called by:   delay.c
 * Author:      Hao Huang
 * Date:        1/1/98
 * Revision:
 * Notes:
 *
 * machine             bits            max sampling rate    #output channels
 * -------------------------------------------------------------------------
 * Mac (all types)     8               22k                  1
 * Mac (newer ones)    16              64k                  4(128)
 * Apple IIgs          8               32k / &gt;70k           16(st)
 * PC/soundblaster pro 8               ?/(22k st, 44.1k mo) 1(st)
 * PC/soundblaster 16  16              44.1k                1(st)
 * PC/pas              8               44.1k st, 88.2k mo   1(st)
 * PC/pas-16           16              44.1k st, 88.2k mo   1(st)
 * PC/turtle beach multisound 16       44.1k                1(st)
 * PC/cards with aria chipset 16       44.1k                1(st)
 * PC/roland rap-10    16              44.1k                1(st)
 * PC/gravis ultrasound 8/16           44.1k                14-32(st)
 * Atari ST            8               22k                  1
 * Atari STE,TT        8               50k                  2
 * Atari Falcon 030    16              50k                  8(st)
 * Amiga               8               varies above 29k     4(st)
 * Sun Sparc           U-LAW           8k                   1
 * Sun Sparcst. 10     U-LAW,8,16      48k                  1(st)
 * NeXT                U-LAW,8,16      44.1k                1(st)
 * SGI Indigo          8,16            48k                  4(st)
 * SGI Indigo2,Indy    8,16            48k                  16(st,4-channel)
 * Acorn Archimedes    ~U-LAW          ~180k                8(st)
 * Sony NWS-3xxx       U,A,8,16        8-37.8k              1(st)
 * Sony NWS-5xxx       U,A,8,16        8-48k                1(st)
 * VAXstation 4000     U-LAW           8k                   1
 * DEC 3000            U-LAW           8k                   1
 * DEC 5000/20-25      U-LAW           8k                   1
 * Tandy 1000L         8               &gt;=44k                1
 * Tandy 2500          8               &gt;=44k                1
 * HP9000/705,710,425e U,A-LAW,16      8k                   1
 * HP9000/715,725,735  U,A-LAW,16      48k                  1(st)
 * HP9000/755 option:  U,A-LAW,16      48k                  1(st)
 * NCD MCX terminal    U,A,8,16        52k                  1(st)
 *
 * Inverse Mu-law
 *	      mp (       y     )
 *	m =  --- ( (u+1)e  - 1 )
 *	      u  (             )
 * Mu-Law: mp is the peak value, u = 255
 *		sgn(m)   (     |m |)       |m |
 *	y =    ------- ln( 1+ u|--|)       |--| =< 1
 *	       ln(1+u)   (     |mp|)       |mp|
 * Mu-Law simplified with x = m / mp
 *		    ln(1+ u|x|)
 *	y = sgn(x) -------------
 *		      ln(u+1)
 * ********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>   // ntohl() and ntohs()

#include "audio_generic.h"
#include "util.h"

#include "aufileutil.h"

const char * ENCODING[] = {
  "PCMU", "PCMA", "G721", "DVI4", "G723", "GSM",
  "1016", "LPC",
  "Linear 8-bit", "Linear 16-bit", "Linear 24-bit", "Linear 32-bit",
  "G728", "TRUE"
};

const char * OPEN_AUDIO_FILE_ERROR[] = {
  "The channel count is not 2",
  "Insufficeint samples in the file\n",
};

extern int open_file(char *dir_list, char *name, char *ext, char *mode, char **chosen);

/*
 * Given the encoding, return the data unit size in byte
 * This is incomplete.  Most default to 1
 */
int getUnitSize(int encoding)
{
  switch (encoding) {
  case AE_PCMU: return(1);
  case AE_PCMA: return(1);
  case AE_G721: return(1); // unknown
  case AE_DVI4: return(1); // unknown
  case AE_G723: return(1); // unknown
  case AE_GSM:  return(1); // unknown
  case AE_1016: return(1); // unknown
  case AE_LPC:  return(1); // unknown
  case AE_L8:   return(1);
  case AE_L16:  return(2);
  case AE_L24:  return(3);
  case AE_L32:  return(4);
  case AE_G728: return(1); // unknown
  case AE_TRUE: return(1); // unknown
  case AE_MAX:  return(1); // unknown
  default:      return(1);
  }
}  // int getUnitSize(int encoding)


/*
 * Open the audio file and read in header
 * time is in seconds
 * Return NULL if failed
 * ErrorCode:  -1 failed to open the file
 *		0 the channel number is not 2
 *		1 insufficient sample data in file
 */
FILE* openAudioFile(char * name, float time, audio_file_t * af_p,
		    int * error_p)
{
  FILE * fp;
  int n;

  if (!(fp = af_open(open_file(0, name, 0, "r", 0), "r", af_p))) {
    *error_p = -1;
printf("Failed to open file\n");
    return(NULL);
  }
  if (2 != af_p->ad.channels) {
    fclose(fp);
    *error_p = 0;
    return(NULL);
  }
  n = time * af_p->ad.sample_rate;
  if (af_p->data_size < n) {
    //fprintf(stderr, "datasize: %d\n", af_p->data_size);
    //fprintf(stderr, "sample_rate:%d\n", af_p->ad.sample_rate);
    fclose(fp);
    *error_p = 1;
    return(NULL);
  }
  return(fp);
} /* openAudioFile */


/* Print the audio description */
void printAudioDescr(FILE * fp, const audio_descr_t * p)
{
  fprintf(fp, "Encoding = %s\nSampleRate = %d\nChannels = %d\n",
	  p->encoding < AE_MAX ? ENCODING[p->encoding] : "Unkown",
	  p->sample_rate, p->channels);
} /* printAudioDescr */


/* Print the info of the audio file */
void printAudioFileInfo(FILE * fp, const audio_file_t * p)
{
  char * s;

  printAudioDescr(fp, &p->ad);
  switch (p->type) {
  case AF_au: s = ".au"; break;
  case AF_snd: s = ".snd"; break;
  case AF_aiff: s = ".aiff"; break;
  case AF_other: s = "others"; break;
  default: s = "Unknown"; break;
  }
  fprintf(fp, "Type = %s\nDataOffset = %d\nDataSize = %d\n",
	  s, p->data_offset, p->data_size);
  if (p->info) {
    fprintf(fp, "Info = %s\n", p->info);
  }
} /* printAudioFileInfo */


/*
 * Decode audio data in buffer to data1_a and data2_a for given encoding
 * Return non-zero if error occured
 */
int decodeAudioData(void * buffer, float *data1_a, float *data2_a,
		    int len, audio_encoding_t encoding)
{
  union {
    signed char * b1;
    signed short * b2;
    signed long * b4;
    unsigned char * ub1;
    unsigned short * ub2;
    unsigned long * ub4;
  } buf;
  int i;

  //fprintf(stderr, "Decoding ... \n");
  // Data are interleaved, so put them in data1 and data2
  buf.b1 = (char*)buffer;
  switch (encoding) {
  case AE_PCMU: // U-LAW 8-bit signed
  case AE_PCMA: // A-LAW 8-bit signed
    //fprintf(stderr, "Decoding U-LAW or A-LAW\n");
    // Not really decode it since the waveform does not change.
    for (i = 0; i < 2*len; i+=2) {
      data1_a[1+i/2] = buf.b1[i];
      data2_a[1+i/2] = buf.b1[i+1];
    }
    break;
  case AE_G721: return(-1); // unknown
  case AE_DVI4: return(-1); // unknown
  case AE_G723: return(-1); // unknown
  case AE_GSM:  return(-1); // unknown
  case AE_1016: return(-1); // unknown
  case AE_LPC:  return(-1); // unknown
  case AE_L8: // b-bit unsigned
    for (i = 0; i < 2*len; i+=2) {
      data1_a[1+i/2] = buf.b1[i];
      data2_a[1+i/2] = buf.b1[i+1];
    }
    break;
  case AE_L16: // 16-bit signed!
    //fprintf(stderr, "Decoding 16-bit linear\n");
    for (i = 0; i < 2*len; i+=2) {
      data1_a[1+i/2] = buf.b2[i];
      data2_a[1+i/2] = buf.b2[i+1];
    }
    break;
  case AE_L24:  return(3);
  case AE_L32: // 32-bit unsigned?
    for (i = 0; i < 2*len; i+=2) {
      data1_a[1+i/2] = buf.b4[i];
      data2_a[1+i/2] = buf.b4[i+1];
    }
    break;
  case AE_G728: return(-1); // unknown
  case AE_TRUE: return(-1); // unknown
  case AE_MAX: // unsupported
  default:     // unsupported
    return(-1);
  }
  return(0);
} /* decodeAudioData */

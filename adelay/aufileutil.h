/* ******************************************************************** 
 * File:        aufileutil.h 
 * Project:     Audio Delay
 * Module:      Audio File Untilities
 * SubModule:   
 * Purpose:     Read audio (.au, .aiff, .snd) data file 
 * Call:        None
 * Called by:   delay.c
 * Author:      Hao Huang 
 * Date:        1/1/98 
 * Revision: 
 * ******************************************************************** 
 */

#ifndef H_AU_FILE_UTIL
#define H_AU_FILE_UTIL

extern const char * ENCODING[];
extern const char * OPEN_AUDIO_FILE_ERROR[];

/* Given the encoding, return the data unit size in byte */
int getUnitSize(int encoding);

/*
 * Open the audio file and read in header
 * time is in seconds
 * Return NULL if failed
 * ErrorCode:  -1 failed to open the file
 *		0 the channel number is not 2
 *		1 insufficient sample data in file
 */
FILE* openAudioFile(char * name, float time, audio_file_t * af_p, 
		    int * error_p);

/* Print the audio description */
void printAudioDescr(FILE * fp, const audio_descr_t * p);

/* Print the info of the audio file */
void printAudioFileInfo(FILE * fp, const audio_file_t * p);

/*
 * Decode audio data in buffer to data1_a and data2_a for given encoding
 * Return non-zero if error occured
 */
int decodeAudioData(void * buffer, float *data1_a, float *data2_a,
		    int len, audio_encoding_t encoding);


#endif

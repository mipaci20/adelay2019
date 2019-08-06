/* modified by Kazu
   repeat calculation for each talk spurt which detected by nevot_sd
   This is only for .au file, not for audio device.
*/
/*
 * Options
 * -h	  Help message
 * -d	  Print data in addition to correlation coefficients
 * -e ##  encoding: PCMU|PCMA|L16, default L16
 * -f ##  sampling frequency in Hz, default 8000
 * -p ##  period for checking delay, default 60 sec, <= 0 for no looping
 * -s ##  skip first seconds, default 0 sec
 * -t ##  sampling length in seconds, default 2 sec
 * -i in  read from given file, default audio device
 * -o out output for fft result in non-looping mode, use - for stdout
 *
 * Works fine even with high level of noise
 */

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>

#include "audio_generic.h"
#include "ahw.h"

#include "aufileutil.h"
#include "fft.h"
#include "adelay.h"

/* global variable */
int printData = 0;
audio_file_t af;
int freq = 8000;  /* sampling frequency [Hz] */
int period = 60;  /* period for checking delay [sec] */
float samp_time = 4.0; /* sampling length [sec] */ 
float skip = 0.0; /* skipping time [sec] */
char * infilename = NULL; /* input audio file name */ 
FILE * out;       /* output file for the fft table */
FILE * out_spurt = NULL; /* the output file for talk spurt */
char * sdfilename = NULL; /* input silence detect data file name */ 

int frame_ms=20;  /* 20ms */
FILE * fp = NULL; /* the input audio wave file */
FILE * fp_sd = NULL; /* the input silence detect data file */
float *data1 = NULL, *data2 = NULL, *answer = NULL;
int lag;

/* main */
int main(int argc, char *argv[])
{
  int len;     /* No. of data in the array, must be power of 2 */
  int fd = -1; /* input device file descriptor */
  
  /* set up default device parameters */
  af.ad.channels    = 2;
  af.ad.encoding    = AE_L16;
  af.ad.sample_rate = 8000;
  af.info = NULL;

  parseCmd(argc, argv);

  if (infilename) {
    if (sdfilename == NULL) {
      fprintf(stderr, "No silence detector file specified\n");
      exit (1);
    }
    else {
      int code = 0;
      fp = openAudioFile(infilename, skip+samp_time, &af, &code);
      //fprintf(stderr, "code = %d\n", code);
      if (!fp) {
        if (code < 0) {
	  perror(infilename);
          exit(5);
        }
        else {
          //fprintf(stderr, "ErrorCode: %d\n", code);
          fprintf(stderr, "Error: %s\n", OPEN_AUDIO_FILE_ERROR[code]);
          exit(6);
        }
      }
      else {
        fp_sd = fopen(sdfilename, "r");
        if (fp_sd == NULL) {
          fprintf(stderr,
                  "Silence Detection File Open Error!:%s\n", sdfilename);
          exit (1);
        }

        printAudioFileInfo(stderr, &af);
        freq = af.ad.sample_rate;
      }
    }
    //fprintf(stderr, "open done\n");
  }
  else { /* read from device */
    audio_descr_t ad = af.ad;
    fprintf(stderr, "Opening audio device\n");
    //printAudioDescr(stderr, &af.ad);
    if (ahw_set(0, AS_port, AP_in_line) ||
	ahw_fmt(&af.ad, &af.ad)) {
      fprintf(stderr, "Failed to configure input\n");
      perror("/dev/audio");
      exit(7);
    }
    if ((fd = ahw_open(0, -1)) < 0) {
      perror("/dev/audio");
      exit(7);
    }
    if (ad.encoding != af.ad.encoding || 
	ad.sample_rate != af.ad.sample_rate ||
	ad.channels != af.ad.channels) {
      fprintf(stderr, "Failed to init audio device for the given format.\n");
      printAudioDescr(stderr, &af.ad);
      exit(8);
    }
    signal(SIGINT, sigint);
    printAudioDescr(stderr, &af.ad);
  }

  len = getDataLength(freq, samp_time);
  fprintf(stderr, "Data Length: %d\n", len);
  data1 = (float*)malloc(sizeof(float)*(2*len+1));
  data2 = (float*)malloc(sizeof(float)*(2*len+1));
  answer = (float*)malloc(sizeof(float)*(4*len+1));
  if (!data1 || !data2 || !answer) {
    fprintf(stderr, "Failed to allocate memory in main\n");
    exit(9);
  }
  /* Zero padding the data at the end */
  setFloatZero(data1, 1+len, len);
  setFloatZero(data2, 1+len, len);
  fprintf(stderr, "Reading data ... \n");

  if (infilename) {
    int num = 0;
    int start, spurt, silence;

    while (search_spurt(&start, &spurt, &silence) != 0) {
      num++;
      printf("Talk Spurt No. %d\n", num);
      printf("  at %.2f[s], duration=%d[ms]\n",start/1000.0, spurt);
      if (spurt >= 4096) { /* talk_spurt >= 4.096 sec */
        while (spurt >= 4096) {
          cal_delay(num, start, 4096); /* sampling length = 4.096sec */
          spurt -= 4096;
          start += 4096;
        }
      }
      if (spurt >= 2048) { /* 2.048 sec =< spurt < 4.096 sec */
        if ((spurt + silence) >= 4096) {
          cal_delay(num, start, 4096); /* sampling length = 4.096sec */
          continue;
        }
        else {
          cal_delay(num, start, 2048); /* sampling length = 2.048sec */
          spurt -= 2048;
          start += 2048;
        }
      }
      if (spurt >= 1024) { /* 1.024 sec =< spurt < 2.048 sec */
        if ((spurt + silence) >= 2048) {
          cal_delay(num, start, 2048); /* sampling length = 2sec */
          continue;
        }
        else {
          cal_delay(num, start, 1024); /* sampling length = 1sec */
          spurt -= 1024;
        }
      }
      if (spurt < 1024) {
        printf("  talk spurt(%d[ms]) is not calculated.\n", spurt);
      }
    } /* while */
    fclose(fp);
    fclose(fp_sd);
  }
  else { /* read data from device */
    //fprintf(stderr, "Reading data from audio device\n");
    int loop = 1;
    int u = getUnitSize(af.ad.encoding);
    int n = u * len * 2; /* interleaved data */
    char * buf = (char*)answer;
    int k, bytes;
    struct timeval tv;
    int last; /* time in sec before reading started */

    gettimeofday(&tv, 0);
    last = tv.tv_sec;
    while (loop) {
      /* First read all bytes into answer which is used as buffer */
      /* Audio packet is either 160 bytes or 320 bytes */
      k = 0;
      while (k < n) {
	//fprintf(stderr, "Attempt to read %d bytes from fd %d\n", n-k, fd);
	if ((bytes = read(fd, buf+k, n-k)) < 0) {
	  perror("read");
	  exit(11);
	}
	if (bytes % u) {
	  fprintf(stderr, "%d bytes read while unit size is %d\n", bytes, u);
	  exit(12);
	}
	k += bytes;
	//fprintf(stderr, "Read %d bytes, %d of %d done\n", bytes, k, n);
      } // while (k < n)
      if (decodeAudioData(answer, data1, data2, len, af.ad.encoding)) {
	fprintf(stderr, "Encoding %s not supported by readDataFromFile\n",
		ENCODING[af.ad.encoding]);
      } 
      lag = correl(data1, data2, len*2, answer);
      printLag(stdout, lag, freq, samp_time);
      gettimeofday(&tv, 0);
      if (last + period > tv.tv_sec) {
	sleep(last + period - tv.tv_sec);
      }
      else {
	loop = 0;
      }
      last += period;
    } /* while (loop) */
    ahw_close();
    /* Print data only in non-looping mode */
    if (out) {
      printResult(out, answer, data1, data2, len, freq, printData);
    }
  }
  free(data1);
  free(data2);
  free(answer);
  return(0);
} /* main */


/* 
 * Read silence detection data file and search talk spurt and silence.
 * The unit is [frame_msec]
 */
int search_spurt(int *start, int *spurt, int *silence)
{
  char line[256];
  int start_fms; /* start time [frame_ms] */
  int len_fms;   /* duration [frame_ms] */
  float len_s;     /* duration [sec] */
  int talk;      /* 1:talk spurt, 0:silence */

  if (fgets(line, sizeof(line), fp_sd) == NULL)    return (0);
  if (sscanf(line, "%d %d %f %d", &start_fms, &len_fms, &len_s, &talk) < 4) {
    return (0);
  }

  if (start_fms < 0) /* end of data */
    return (0);
  if (talk == 0) {  /* the first data of the file is silence, so skip */
    if (fgets(line, sizeof(line), fp_sd) == NULL)    return (0);
    if (sscanf(line, "%d %d %f %d", &start_fms, &len_fms, &len_s, &talk) < 4) {
      return (0);
    }
  }
  *start = (start_fms -1) * frame_ms;
  *spurt = len_fms * frame_ms;

  if (fgets(line, sizeof(line), fp_sd) == NULL)    return (0);
  if (sscanf(line, "%d %d %f %d", &start_fms, &len_fms, &len_s, &talk) < 4) {
    return (0);
  }
  *silence = len_fms * frame_ms;
  return (1);
}


/* Added by Kazu */
/* calculate delay from the specified time and the length */
/* The unit of start and length is [msec] */
int cal_delay(int num, int start, int length)
{
  int d_length; /* length of data in wave file */

  /* skipping specified length of data */
  if (fseek(fp, 
      af.data_offset + 2*getUnitSize(af.ad.encoding)*freq/1000*start, 
      SEEK_SET)) {
    fprintf(stderr, "Failed to fseek from %s\n", infilename);
    return 1;
  }

  d_length=(int)(freq*length/1000);
  if (fread(answer, 2*getUnitSize(af.ad.encoding), d_length, fp) != d_length) {
    fprintf(stderr, "Failed to read data from %s\n", infilename);
    return 1;
  }

  if (decodeAudioData(answer, data1, data2, d_length, af.ad.encoding)) {
    fprintf(stderr, "Encoding %s not supported by readDataFromFile\n",
 	      ENCODING[af.ad.encoding]);
    return 1;
  }

  lag = correl(data1, data2, d_length*2, answer);
  if (lag < 0) lag *= -1;
  fprintf(out_spurt, "%d %f\n", num-1, 1000*lag/(double)freq);

  printLag(stdout, lag, freq, length/1000.0);
  if (out) {
    printResult(out, answer, data1, data2, d_length, freq, printData);
  }
  setFloatZero(data1, 0, 2 * d_length + 1);
  setFloatZero(data2, 0, 2 * d_length + 1);
  return(0);
}


/*
* Handles interrupt signal: close the opened audio device
*/
static void sigint(int sig)
{
  fprintf(stderr, "\nClosing audio device\n");
  ahw_close();
  exit(0);
} /* sigint */


/*
 * Given frequency and time length
 * Return the data length as power of 2 just bigger than freq * time
 */
int getDataLength(int freq, float time) 
{
  double t = freq * time;
  return(1 << (int)(ceil(log(t) / log(2))));
} /* getDataLength */


/*
 * Print the lag to out
 */
static void printLag(FILE * out, int lag, int freq, float time)
{
  if (lag > time * freq / 2 || lag < -time * freq / 2) {
    //fprintf(out, "Lag > samples/2, more samples may be needed\n");
  }
  if (lag > 0) {
    fprintf(out, "LeftChannel lags behind RightChannel by %f msec\n", 
	    1000*lag/(double)freq);
  }
  else { // r < 0
    fprintf(out, "LeftChannel is ahead of RightChannel by %f msec\n", 
	    -1000*lag/(double)freq);
  }
} /* printLag */


/*
 * Print results to out in "lag	Correlation"
 * If printData is true, in "lag Correlation LeftChannel RightChannel"
 */
void printResult(FILE * out, float * answer, float * data1, float * data2,
		 int len, int freq, int printData)
{
  int i;
  
  if (printData) {
    fprintf(out, "Lag (msec)\tCorrelation\tLeft Channel\tRight Channel\n");
    for (i = len/2+1; i <= len; ++i) {
      fprintf(out, "%f\t%e\t%e\t%e\n", 1000*(i-1-len)/(double)freq, 
	      answer[i], data1[i], data2[i]);
    }
    for (i = 1; i <= len/2; ++i) {
      fprintf(out, "%f\t%e\t%e\t%e\n", 1000*(i-1)/(double)freq, 
	      answer[i], data1[i], data2[i]);
    }
  }
  else {
    fprintf(out, "Lag (msec)\tCorrelation\n");
    for (i = len/2+1; i <= len; ++i) {
      fprintf(out, "%f\t%e\n", 1000*(i-1-len)/(double)freq, answer[i]);
    }
    for (i = 1; i <= len; ++i) {
      fprintf(out, "%f\t%e\n", 1000*(i-1)/(double)freq, answer[i]);
    }
  }
} /* void printResult */


/* Parse command line option */
int parseCmd(int argc, char *argv[])
{
  int c;

  if (argc < 2) {
    usage(argv[0]);
    exit(1);
  }

  while((c=getopt(argc, argv, "hde:f:p:s:t:i:o:k:m:")) != EOF){
    switch(c) {
    case 'h':
      usage(argv[0]);
      break;
    case 'd':
      printData = 1;
      break;
    case 'e':
      if (!strcasecmp(optarg, "pcmu")) {
	af.ad.encoding = AE_PCMU;
      }
      else if (!strcasecmp(optarg, "pcma")) {
	af.ad.encoding = AE_PCMA;
      }
      else if (!strcasecmp(optarg, "l16")) {
	af.ad.encoding = AE_L16;
      }
      else {
	fprintf(stderr, "Illegal encoding %s\n", optarg);
	exit(1);
      }
      break;
    case 'f':
      if ((freq = atoi(optarg)) <= 0) {
	fprintf(stderr, "sampling frequency must > 0\n");
	exit(2);
      }
      break;
    case 'p':
      period = atoi(optarg);
      break;
    case 's':
      if ((skip = atof(optarg)) <= 0) {
	fprintf(stderr, "skipping time must > 0\n");
	exit(3);
      }
      break;
    case 't':
      if ((samp_time = atof(optarg)) <= 0) {
	fprintf(stderr, "sampling time must > 0\n");
	exit(4);
      }
      break;
    case 'i':
      infilename = optarg;
      break;
    case 'o':
      if (!strcmp(optarg, "-")) {
	out = stdout;
      }
      else if (!(out = fopen(optarg, "w"))) {
	perror(optarg);
      }
      break;
    case 'k':
      sdfilename = optarg;
      break;
    case 'm':
      if (!(out_spurt = fopen(optarg, "w"))) {
	perror(optarg);
      }
      break;
    case '?':
    default:
      usage(argv[0]);
      break;
    }
  }
  return (0);
}


/*
 * Print usage and quit
 */
void usage(const char * pname)
{
  fprintf(stderr, "usage: %s [-hd] [-e ##] [-f ##] [-p ##] [-s ##] ", pname);
  fprintf(stderr, "[-t ##] [-i in] [-o out] [-k file] [-m file]\n");
  fprintf(stderr, "\t-h      print this message\n");
  fprintf(stderr, "\t-d      print data in addition to correlation\n");
  fprintf(stderr, "\t-e ##   encoding (PCMU|PCMA|L16, default L16)\n");
  fprintf(stderr, "\t-f ##   sampling frequency in Hz (default 8000)\n");
  fprintf(stderr, "\t-p ##   period for checking delay (default 60 sec), ");
  fprintf(stderr, "<= 0 for no looping\n");
  fprintf(stderr, "\t-s ##   skip first seconds (default 0 sec)\n");
  fprintf(stderr, "\t-t ##   sampling length in seconds (default: 4 sec)\n");
  fprintf(stderr, "\t-i in   read from given file (default audio device)\n");
  fprintf(stderr, "\t-o out  output for fft result in non-looping ");
  fprintf(stderr, "mode, use - for stdout\n");
  fprintf(stderr, "\t-k file read talk spurt data from given file\n");
  fprintf(stderr, "\t-m file write delay per talk spurt to given file\n");
  exit(1);
} /* void usage */

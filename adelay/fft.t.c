// ******************************************************************** 
// File:        fft.t.c
// Project:     Audio Delay
// Module:      FFT
// SubModule:   
// Purpose:     test the fft module
// Call:        fft
// Called by:   None
// Author:      Hao Huang 
// Date:        1/1/98 
// Revision: 
// ******************************************************************** 

#include <math.h>
#include <stdlib.h>
#include "fft.h"

// Print usage and quit
void usage(const char * pname);


int main(int argc, char** argv) 
{
  int len = 64;
  int delay = 8;
  int absdelay;
  int i = 0, r = 0;
  float *data1, *data2, *answer;
  float f = 1.0;
  char c;
  
  while(EOF != (c=getopt(argc, argv, "hd:f:l:"))){
    switch(c){
    case 'h':
      usage(argv[0]);
      break;
    case 'd':
      delay = atoi(optarg);
      break;
    case 'f':
      if (0 == (f = atof(optarg))) {
	fprintf(stderr, "sample coefficient must != 0\n");
	exit(-1);
      }
      break;
    case 'l':
      if ((len = atoi(optarg)) <= 0) {
	fprintf(stderr, "sample length must > 0\n");
	exit(-1);
      }
      break;
    case '?':
    default:
      usage(argv[0]);
      break;
    }
  }
  absdelay = (delay > 0 ? delay : -delay);
  if (delay > len / 2 || delay < -len / 2) {
    fprintf(stderr, "dealy out of range: -length/2 < delay < length/2\n");
    exit(-1);
  }

  data1 = (float*) malloc(sizeof(float) * (2*len+1));
  data2 = (float*) malloc(sizeof(float) * (2*len+1));
  answer = (float*) malloc(sizeof(float) * (4*len+1));
  setFloatZero(data1, 0, 2*len+1);
  setFloatZero(data2, 0, 2*len+1);
  for (i = 0; i <= len; ++i) {
    data2[absdelay+i] = sin(M_PI * i / (float)len);
    data1[absdelay+delay+i] = data2[absdelay+i] * f;
  }
  r = correl(data1, data2, 2*len, answer);
  if (r > 0) {
    printf("Data1 lags behind Data2 by %d cells\n", r);
  }
  else { // r < 0
    printf("Data1 is ahead Data2 by %d cells\n", -r);
  }
  for (i = 1; i <= 2*len; ++i) {
    printf("%d\t%e\t%e\t%e\n", i, data1[i], data2[i], answer[i]);
  }
  return(0);
}


// Print usage and quit
void usage(const char * pname)
{
  fprintf(stderr, "usage: %s [-h] [-d ##] [-f ##] [-l ##]\n", pname);
  fprintf(stderr, "\t-h      print this message\n");
  fprintf(stderr, "\t-d ##   sample delay (default 8)\n");
  fprintf(stderr, "\t-f ##   data1 = f * data2 (default f=1.0)\n");
  fprintf(stderr, "\t-l ##   sample data length (default: 64)\n");
  exit(1);
}

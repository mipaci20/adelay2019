/* ******************************************************************** 
 * File:        fft.c
 * Project:     Audio Delay
 * Module:      FFT
 * SubModule:   
 * Purpose:     Compute the correlation of two real data sets
 * Call:        None
 * Called by:   delay.c
 * Author:      Hao Huang 
 * Date:        1/1/98 
 * Revision: 
 * ******************************************************************** 
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "fft.h"

/*
 * Computes the correlation of two real data sets data1[1..n] and data2[1..n]
 * (including any user-supplied zero padding). n MUST be an integer power of 
 * two. The answer is returned as the first n points in ans[1..2*n] stored
 * in wrap-around order, i.e., correlations at increasingly negative lags 
 * are in ans[n] on down to ans[n/2+1], while correlations at increasingly 
 * positive lags are in ans[1] (zero lag) on up to ans[n/2]. Note that ans 
 * must be supplied in the calling program with length at least 2*n, since
 * it is also used as working space. Sign convention of this routine: 
 * if data1 lags data2, i.e., is shifted to the right of it, then ans will 
 * show a peak at positive lags.
 * Return the lag: > 0 if data1 lags behind data2
 *		   < 0 if data1 is ahead of data2
 */
int correl(const float data1[], const float data2[],
	   unsigned long n, float ans[])
{
  unsigned long no2, i;
  float tmp, *fft;
  int r = 0;

  if (!(fft = (float*)malloc(sizeof(float)*(2*n+1)))) {
    fprintf(stderr, "malloc failed in correl\n");
    exit(-1);
  }
  twofft(data1, data2, fft, ans, n); //Transform both data vectors at once.
  no2 = n >> 1;  // Normalization for inverse FFT.
  for (i = 2; i <= n+2; i += 2) {
    // Multiply to Find FFT of their correlation.
    tmp = ans[i-1];
    ans[i-1] = (fft[i-1] * tmp + fft[i] * ans[i]  ) / no2;
    ans[i]   = (fft[i]   * tmp - fft[i-1] * ans[i]) / no2;
  }
  ans[2] = ans[n+1]; // Pack First and last into one element.
  realft(ans, n, -1); // Inverse transform gives correlation.
  free(fft); // free the space
  for (i = 1; i <= n; ++i) {
    if (!r || ans[i] > ans[r]) {
      r = i;
    }
  }
  // r > n/2 means negative lag -- data1 ahead of data2
  //fprintf(stderr, "n = %d, r = %d, cor = %e\n", n, r, ans[r]);
  return(r > n / 2 ? (r - 1 - n) : r - 1);
} /* correl */


/*
 * Replaces data[1..2*nn] by its discrete Fourier transform if isign is 
 * input as 1; or replaces data[1..2*nn] by nn times its inverse discrete
 * Fourier transform,if isign is input as -1.
 * data is a complex array of length nn or,equivalently,
 * a real array of length 2*nn. nn MUST be an integer power of 2
 * (this is not checked for!).
 */
void four1(float data[], unsigned long nn, int isign)
{
  unsigned long n, mmax, m, j, istep, i;
  // Double precision for the trigonometric recurrences. 
  double wtemp, wr, wpr, wpi, wi, theta; 
  float tempr, tempi;

  n = nn << 1;
  j = 1;
  for (i = 1; i < n; i += 2) { 
    //This is the bit-reversal section of the routine. 
    if (j > i) {
      //Exchange the two complex numbers.
      tempr   = data[j];
      data[j] = data[i];
      data[i] = tempr;
      tempi     = data[j+1];
      data[j+1] = data[i+1];
      data[i+1] = tempi;
    }
    m = n >> 1;
    while (m >= 2 && j > m) {
      j -= m;
      m >>= 1;
    }
    j += m;
  }
  //Here begins the Danielson-Lanczos section of the routine.
  mmax = 2;
  while (n > mmax) { // Outer loop executed log 2 nn times.
    istep = mmax << 1;
    //Initialize the trigonometric recurrence.
    theta = isign * (6.28318530717959 / mmax); 
    wtemp = sin(0.5 * theta);
    wpr = -2.0 * wtemp * wtemp;
    wpi = sin(theta);
    wr = 1.0;
    wi = 0.0;
    for (m = 1; m < mmax; m += 2) { //Here are the two nested inner loops.
      for (i = m; i <= n; i += istep) {
	j = i + mmax; // This is the Danielson-Lanczos formula
	tempr = wr * data[j]   - wi * data[j+1];
	tempi = wr * data[j+1] + wi * data[j];
	data[j]   = data[i] - tempr;
	data[j+1] = data[i+1] - tempi;
	data[i]   += tempr;
	data[i+1] += tempi;
      }
      wtemp = wr;
      wr = wtemp * wpr - wi * wpi + wr; // Trigonometric recurrence.
      wi = wi * wpr + wtemp * wpi + wi;
    }
    mmax = istep;
  }
} /* four1 */


/*
 * Calculates the Fourier transform of a set of n real-valued data points.
 * Replaces this data (which is stored in array data[1..n]) by the positive
 * frequency half of its complex Fourier transform. The real-valued First
 * and last components of the complex transform are returned as elements
 * data[1] and data[2], respectively. n must be a power of 2. This routine
 * also calculates the inverse transform of a complex data array if it is 
 * the transform of real data. (Result in this case must be multiplied by 2/n.)
 */
void realft(float data[], unsigned long n, int isign)
{
  unsigned long i, i1, i2, i3, i4, np3;
  float c1 = 0.5, c2, h1r, h1i, h2r, h2i;
  // Double precision for the trigonometric recurrences.
  double wr, wi, wpr, wpi, wtemp, theta;

  theta = M_PI / (double)(n>>1); //Initialize the recurrence.
  if (1 == isign) {
    c2 = -0.5;
    four1(data, n>>1, 1); // The forward transform is here.
  } else {
    c2 = 0.5; // Otherwise set up for an inverse transform.
    theta = -theta;
  }
  wtemp = sin(0.5 * theta);
  wpr = -2.0 * wtemp * wtemp;
  wpi = sin(theta);
  wr = 1.0 + wpr;
  wi = wpi;
  np3 = n + 3;
  for (i = 2; i<= (n>>2); ++i) { // Case i = 1 done separately below.
    i4 = 1+(i3 = np3-(i2 = 1+(i1 = i+i-1)));
    // The two separate transforms are separated out of data. 
    h1r =  c1*(data[i1] + data[i3]); 
    h1i =  c1*(data[i2] - data[i4]);
    h2r = -c2*(data[i2] + data[i4]);
    h2i =  c2*(data[i1] - data[i3]);
    // Here they are recombined to form the true transform
    // of the original real data.
    data[i1] =  h1r + wr * h2r - wi * h2i; 
    data[i2] =  h1i + wr * h2i + wi * h2r;
    data[i3] =  h1r - wr * h2r + wi * h2i;
    data[i4] = -h1i + wr * h2i + wi * h2r;
    wtemp = wr;
    wr = wtemp * wpr - wi * wpi + wr; // The recurrence.
    wi = wi * wpr + wtemp * wpi + wi;
  }
  h1r = data[1];
  if (1 == isign) {
    // Squeeze the First and last data together to get them all within the
    // original array.
    data[1] = h1r + data[2]; 
    data[2] = h1r - data[2];
  } else {
    data[1] = c1 * (h1r + data[2]);
    data[2] = c1 * (h1r - data[2]);
    // This is the inverse transform for the case isign = -1.
    four1(data, n>>1, -1); 
  }
} /* realft */


/*
 * Given an array of float, set 'len' cells starting from 'start' as 0.0
 */
void setFloatZero(float data[], int start, int len)
{
  int i;
  for (i = start; i < start+len; ++i) {
    data[i] = 0.0;
  }
} /* setFloatZero */


/*
 * Given two real input arrays data1[1..n] and data2[1..n], this routine
 * calls four1 and returns two complex output arrays, fft1[1..2n] and 
 * fft2[1..2n], each of complex length n (i.e., real length 2*n), which
 * contain the discrete Fourier transforms of the respective data arrays.
 * n MUST be an integer power of 2.
 */
void twofft(const float data1[], const float data2[], 
	    float fft1[], float fft2[], 
	    unsigned long n)
{
  unsigned long nn2 = 2 + n + n;
  unsigned long nn3 = 1 + nn2;
  unsigned long i;
  float rep, rem, aip, aim;

  //Pack the two real arrays into one complex array.
  for (i = 1; i <= n; ++i) { 
    fft1[2*i-1] = data1[i];
    fft1[2*i]   = data2[i];
  }
  four1(fft1, n, 1); // Transform the complex array.
  fft2[1] = fft1[2];
  fft1[2] = fft2[2] = 0.0;
  for (i = 3; i <= n+1; i += 2) {
    //Use symmetries to separate the two transforms. 
    rep = 0.5*(fft1[i] + fft1[nn2-i]); 
    rem = 0.5*(fft1[i] - fft1[nn2-i]);
    aip = 0.5*(fft1[i+1] + fft1[nn3-i]);
    aim = 0.5*(fft1[i+1] - fft1[nn3-i]);
    fft1[i] = rep; //Ship them out in two complex arrays.
    fft1[i+1] = aim;
    fft1[nn2-i] = rep;
    fft1[nn3-i] = -aim;
    fft2[i] = aip;
    fft2[i+1] = -rem;
    fft2[nn2-i] = aip;
    fft2[nn3-i] = rem;
  }
} /* twofft */



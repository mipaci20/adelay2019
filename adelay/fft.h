/* ******************************************************************** 
 * File:        fft.h 
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

#ifndef H_FFT
#define H_FFT

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
	   unsigned long n, float ans[]);

/*
 * Replaces data[1..2*nn] by its discrete Fourier transform if isign is 
 * input as 1; or replaces data[1..2*nn] by nn times its inverse discrete
 * Fourier transform, if isign is input as -1.
 * data is a complex array of length nn or, equivalently, 
 * a real array of length 2*nn. nn MUST be an integer power of 2
 * (this is not checked for!).
 */
void four1(float data[], unsigned long nn, int isign);

/*
 * Calculates the Fourier transform of a set of n real-valued data points.
 * Replaces this data (which is stored in array data[1..n]) by the positive
 * frequency half of its complex Fourier transform. The real-valued First
 * and last components of the complex transform are returned as elements
 * data[1] and data[2], respectively. n must be a power of 2. This routine
 * also calculates the inverse transform of a complex data array if it is 
 * the transform of real data. (Result in this case must be multiplied by 2/n.)
 */
void realft(float data[], unsigned long n, int isign);

/*
 * Given an array of float, set 'len' cells starting from 'start' as 0.0
 */
void setFloatZero(float data[], int start, int len);

/*
 * Given two real input arrays data1[1..n] and data2[1..n], this routine
 * calls four1 and returns two complex output arrays, fft1[1..2n] and 
 * fft2[1..2n], each of complex length n (i.e., real length 2*n), which
 * contain the discrete Fourier transforms of the respective data arrays.
 * n MUST be an integer power of 2.
 */
void twofft(const float data1[], const float data2[], 
	    float fft1[], float fft2[],
	    unsigned long n);


#endif

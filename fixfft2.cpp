
#include "fix_fft2.h"
//#include <WProgram.h>

/* fix_fft.c - Fixed-point in-place Fast Fourier Transform  */
/*
  All data are fixed-point short integers, in which -32768
  to +32768 represent -1.0 to +1.0 respectively. Integer
  arithmetic is used for speed, instead of the more natural
  floating-point.

  For the forward FFT (time -> freq), fixed scaling is
  performed to prevent arithmetic overflow, and to map a 0dB
  sine/cosine wave (i.e. amplitude = 32767) to two -6dB freq
  coefficients. The return value is always 0.

  For the inverse FFT (freq -> time), fixed scaling cannot be
  done, as two 0dB coefficients would sum to a peak amplitude
  of 64K, overflowing the 32k range of the fixed-point integers.
  Thus, the fix_fft() routine performs variable scaling, and
  returns a value which is the number of bits LEFT by which
  the output must be shifted to get the actual amplitude
  (i.e. if fix_fft() returns 3, each value of fr[] and fi[]
  must be multiplied by 8 (2**3) for proper scaling.
  Clearly, this cannot be done within fixed-point short
  integers. In practice, if the result is to be used as a
  filter, the scale_shift can usually be ignored, as the
  result will be approximately correctly normalized as is.

  Written by:  Tom Roberts  11/8/89
  Made portable:  Malcolm Slaney 12/15/94 malcolm@interval.com
  Enhanced:  Dimitrios P. Bouras  14 Jun 2006 dbouras@ieee.org
  Modified for 8bit values David Keller  10.10.2010
*/


#define N_WAVE      256    /* full length of Sinewave[] */
#define LOG2_N_WAVE 8      /* log2(N_WAVE) */




/*
  Since we only use 3/4 of N_WAVE, we define only
  this many samples, in order to conserve data space.
*/



char Sinewave[N_WAVE - N_WAVE / 4]  = {
  0, 3, 6, 9, 12, 15, 18, 21,
  24, 28, 31, 34, 37, 40, 43, 46,
  48, 51, 54, 57, 60, 63, 65, 68,
  71, 73, 76, 78, 81, 83, 85, 88,
  90, 92, 94, 96, 98, 100, 102, 104,
  106, 108, 109, 111, 112, 114, 115, 117,
  118, 119, 120, 121, 122, 123, 124, 124,
  125, 126, 126, 127, 127, 127, 127, 127,

  127, 127, 127, 127, 127, 127, 126, 126,
  125, 124, 124, 123, 122, 121, 120, 119,
  118, 117, 115, 114, 112, 111, 109, 108,
  106, 104, 102, 100, 98, 96, 94, 92,
  90, 88, 85, 83, 81, 78, 76, 73,
  71, 68, 65, 63, 60, 57, 54, 51,
  48, 46, 43, 40, 37, 34, 31, 28,
  24, 21, 18, 15, 12, 9, 6, 3,

  0, -3, -6, -9, -12, -15, -18, -21,
  -24, -28, -31, -34, -37, -40, -43, -46,
  -48, -51, -54, -57, -60, -63, -65, -68,
  -71, -73, -76, -78, -81, -83, -85, -88,
  -90, -92, -94, -96, -98, -100, -102, -104,
  -106, -108, -109, -111, -112, -114, -115, -117,
  -118, -119, -120, -121, -122, -123, -124, -124,
  -125, -126, -126, -127, -127, -127, -127, -127,

  /*-127, -127, -127, -127, -127, -127, -126, -126,
    -125, -124, -124, -123, -122, -121, -120, -119,
    -118, -117, -115, -114, -112, -111, -109, -108,
    -106, -104, -102, -100, -98, -96, -94, -92,
    -90, -88, -85, -83, -81, -78, -76, -73,
    -71, -68, -65, -63, -60, -57, -54, -51,
    -48, -46, -43, -40, -37, -34, -31, -28,
    -24, -21, -18, -15, -12, -9, -6, -3, */
};






/*
  FIX_MPY() - fixed-point multiplication & scaling.
  Substitute inline assembly for hardware-specific
  optimization suited to a particluar DSP processor.
  Scaling ensures that result remains 16-bit.
*/
inline char FIX_MPY(char a, char b)
{
  
  int c = (int)a * (int)b + 64;
  a = (c >> 7);
   return a;
}

/*
  FIX_MPY1() and FIX_MPY2()

  FIX_MPY1():    FIX_MPY() - FIX_MPY()
  FIX_MPY2():    FIX_MPY() + FIX_MPY()
  slightly fast  FIX_MPY call.
   */
inline char FIX_MPY1(char a, char b, char c, char d)
{

  
 return( ((int)a * (int)b - (int)c *(int) d) +64 )>> 7;

 
}

inline char FIX_MPY2(char a, char b, char c, char d)
{

  
  return( ((int)a * (int)b + (int)c *(int) d) +64) >> 7;

  
}





/*
  fix_fft() - perform forward/inverse fast Fourier transform.
  fr[n],fi[n] are real and imaginary arrays, both INPUT AND
  RESULT (in-place FFT), with 0 <= n < 2**m; set inverse to
  0 for forward transform (FFT), or 1 for iFFT.


  Changed
changed only 128scale no inverse  for transaction speed improvement
*/


int fix_fft2(char fr[], char fi[])
{

  int m, mr,  i, j, l, k, istep;  
  char qr, qi, tr, ti, wr, wi;

 
  
  mr = 0;

  /* decimation in time - re-order data */
  for (m = 1; m <=127; ++m) {
    l = 128;
    do {
      l >>= 1;
    } while (mr + l >127);
    mr = (mr & (l - 1)) + l;

    if (mr <= m)
      continue;
    tr = fr[m];
    ti = fr[mr];
     
    fr[m] = ti;
    fr[mr] = tr;
    
    ti = fi[m];
    tr = fi[mr];
    
    fi[m] = tr;
    fi[mr] = ti;
  }

  l = 1;
  k = LOG2_N_WAVE - 1;
  while (l < 128) {

   
    istep = l << 1;
    for (m = 0; m < l; ++m) {
      j = m << k;
      /* 0 <= j < N_WAVE/2 */
  

      wi = -Sinewave[j];

      wr =  Sinewave [ j + N_WAVE / 4];

      wr >>= 1;
      wi >>= 1;

      for (i = m; i < 128; i += istep) {
        j = i + l;
        
//        tr = FIX_MPY(wr, fr[j]) - FIX_MPY(wi, fi[j]);
 //       ti = FIX_MPY(wr, fi[j]) + FIX_MPY(wi, fr[j]);

        tr = FIX_MPY1(wr, fr[j],wi, fi[j]);
        ti = FIX_MPY2(wr, fi[j],wi, fr[j]);
        
        qr = fr[i]>>1;
        qi = fi[i]>>1;
        //qr >>= 1;
        //qi >>= 1;
     
        fr[j] = qr - tr;
        fr[i] = qr + tr;        
        fi[j] = qi - ti;
        fi[i] = qi + ti;
      }
    }
    --k;
    l = istep;
  }
  return 0;
}









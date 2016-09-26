/****************
Copyright (c) 2005-2010 Antonio Pertusa Ibáñez <pertusa@dlsi.ua.es>
Copyright (c) 2010 Universidad de Alicante

This multiple fundamental frequency estimation method is free software: you
 can redistribute it and/or modify it under the terms of the GNU General
 Public License as published by the Free Software Foundation, either version
 3 of the License, or (at your option) any later version.

This multiple fundamental frequency estimation method is distributed in the
 hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code.  If not, see <http://www.gnu.org/licenses/>.

 Comments are welcomed
 
**************/

/**************************************************************

 myfft provides functions for handling the FFT. Main function is: 

 void fourier_spectrum(double *sf, double *fft_data, int fft_size, 
		     int data_len, double *window, int win_len)

 abs(FFT(sf)) is computed with this function. Requires FFTW 
 to be installed.

***************************************************************/

#include "myfft.h"
#include "params.h"
#include <iostream>

using namespace std;

static double *rdata = NULL, *idata = NULL;
static fftw_plan rplan;
static int last_fft_size = 0;
static double w_rate = (TWO_PI / 22050.0);


/*--------------------------------------------

 Conversion functions

---------------------------------------------*/

double mus_linear_to_db(double x) {
	if (x > 0.0) return(20.0 * log10(x)); 
	return(-200.0);
}

double mus_radians_to_hz(double rads) {return(rads / w_rate);}
double mus_hz_to_radians(double hz) {return(hz * w_rate);}
double mus_degrees_to_radians(double degree) {return(degree * TWO_PI / 360.0);}
double mus_radians_to_degrees(double rads) {return(rads * 360.0 / TWO_PI);}
double mus_db_to_linear(double x) {return(pow(10.0, x / 20.0));}


/*--------------------------------------------

 IsNaN returns 1 if the input number is a NaN.

-------------------------------------------*/

bool isNaN(double val) {
	return (isnan(val) || isinf(val));
}

/*---------------------------------------------

 Create Hanning window

----------------------------------------------*/

void Hanning(double *window, int size) {
	for (int i=1; i<=size; i++)
		window[i-1]=0.5*(1-cos(2*M_PI*((double)i/(double)(size+1))));
}

void Hamhan(double *window, int size) {
	for (int i=1; i<=size; i++)
		window[i-1]=pow(0.5-0.5*(cos(2*M_PI*((double)i/(double)(size)))),2.0);
}

/* Create BlackmanHarris92 dB */

void Blackman92(double *window, int size)
{
// function to create a backmanHarris window
	int     i;
	double fSum = 0;

	// for -92dB 
	float a0 = .35875, a1 = .48829, a2 = .14128, a3 = .01168;
	double fConst = TWO_PI / (size-1);
  
        // compute window
        for(i = 0; i < size; i++)
        {
                fSum += window[i] = a0 - a1 * cos(fConst * i) +
                        a2 * cos(fConst * 2 * i) - a3 * cos(fConst * 3 * i);
        }

        // mmm... I'm not sure whether this factor is needed
        fSum = fSum / 2.0; 

        // scale function
        for (i = 0; i < size; i++)
                window[i] = 500.0*window[i] / fSum;
}


/*--------------------------------------------

 Apply FFTW

---------------------------------------------*/

void mus_fftw(double *rl, int n, int dir)
{
  int i;
  if (n != last_fft_size)
    {
      if (rdata) {fftw_free(rdata); fftw_free(idata); fftw_destroy_plan(rplan); //fftw_destroy_plan(iplan);
      }
      rdata = (double *)fftw_malloc(n * sizeof(double));
      idata = (double *)fftw_malloc(n * sizeof(double));
      rplan = fftw_plan_r2r_1d(n, rdata, idata, FFTW_R2HC, FFTW_ESTIMATE); 
      last_fft_size = n;
    }
  memset((void *)idata, 0, n * sizeof(double));
  for (i = 0; i < n; i++) {rdata[i] = rl[i];}
    fftw_execute(rplan);
  for (i = 0; i < n; i++) rl[i] = idata[i];
}

/*---------------------------------------------
 
 Spectrum calculation, using mus_fftw

---------------------------------------------*/

void fourier_spectrum(double *sf, double *fft_data, int fft_size, int data_len, double *window, int win_len)
{
  int i;

  if (window)
  {
      for (i = 0; i < win_len; i++)
	fft_data[i] = window[i] * sf[i]; 

      for (i=win_len; i<data_len; i++)
	fft_data[i] = sf[i];
  }
  else
  {
      for (i = 0; i < data_len; i++)
	fft_data[i] = sf[i];
  }

  if (data_len < fft_size) 
      memset((void *)(fft_data + data_len), 0, (fft_size - data_len) * sizeof(double));

  int j;
  mus_fftw(fft_data, fft_size, 1);

  fft_data[0] = fabs(fft_data[0]);
  fft_data[fft_size / 2] = fabs(fft_data[fft_size / 2]);

  for (i = 1, j = fft_size - 1; i < fft_size / 2; i++, j--) {
     fft_data[i] = hypot(fft_data[i], fft_data[j]);
  }

  // Correct NaN values

  for (int i=0; i<fft_size/2; i++)
	if (isNaN(fft_data[i]))
		fft_data[i]=0.0;
}


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

#ifndef MYFFT
#define MYFFT
#include <fftw3.h>
#include <math.h>
#include <string.h>

#define TWO_PI (M_PI * 2.0)

/*-------------------------------- myfft functions  -------------------------------------*/

double mus_linear_to_db(double x);
double mus_radians_to_hz(double rads);
double mus_hz_to_radians(double hz);
double mus_degrees_to_radians(double degree);
double mus_radians_to_degrees(double rads);
double mus_db_to_linear(double x);

void Hanning(double *win, int n);
void Blackman92(double *win, int n);
void Hamhan(double *win, int n);
void mus_fftw(double *rl, int n, int dir);
void fourier_spectrum(double *sf, double *fft_data, int fft_size, int data_len, double *window, int win_len);
bool isNaN(double val);

#endif

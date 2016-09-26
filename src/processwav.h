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

#ifndef PROCESSWAV
#define PROCESSWAV

#include "defines.h"
#include "myfft.h"
#include "info.h"
#include "params.h"
#include "bands.h"
#include <sndfile.h>
#include <math.h>
#include <iostream>
#include <map>
#include <vector>
#include <stdlib.h>

using namespace std;

void open_wavfile(char* name);
int get_samples(int len, int dr);
int read_data(double *input);
void initialize_spectrum_parameters();
void rectangfilter(double *cumsumvector, int len);
void computesinusoidal(double *input, double Hnorm, double *spectrum, double *peaks, double *samplitudes);
void processSL(double *R, double *X, double *slikeness, double Hnorm, int width, double *samplitudes);
double initializeSinusoidal();
int input_data_wav(char *name , peaksvector& allpeaks, vectorbands& spectralbands, Tmatrix& outbandvalues, pinfo& tmpspectruminfo); 

#endif

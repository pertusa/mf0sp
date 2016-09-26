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

#ifndef PARAMS
#define PARAMS
#include <math.h>

#define kVERSION 4.53

/***************** Spectrum analysis parameters *****************/

#define WINDOW		0  	// 0: Hanning, 1: Blackman-Harris 92dB
#define kDR 	 	1       // Decimation ratio applied. When -onsets is used, kPERCENTAGE must be 50.0 and kN=0
#define kPERCENTAGE	87.5	// Overlap percentage
#define kN		1	// Input window size (0->43ms, 1->96ms, 2->192ms)
#define kRES	 	4       // winsize=winsize*res (Zero padding) for 44kHz. If sr=22kHz, this value is multiplied by two
#define DIVFACTOR	2147483000.0	// 2^16 * 2^15= 2^21 Divide input (time-signal) between this value before FFT computation
#define INTERPOLATION   false	// Quadratic interpolation

#define MFT		false	// Multiresolution fourier transform
#define SIZEMFTFREQS	2	// Size of mftfreqs
const double mftfreqs[]= {150,2000};	// Limit frequencies for MFT

/***************** Sinusoidal likeness parameters ***************/

#define kSINUSOIDAL	false   // If true, sinusoidal peak picking is activated. If false, normal spectrum peak piking will be computed 

#define kW		50.0	// Bandwidth (Hz) for sinusoidal likeness
#define kMINSINLIKEVAL   0.1	// 0.1 Minimum sinusoidal likeness value for considering peaks. Not used if kSINUSOIDAL=false 
				// when spectral whitening is activated, this value should be about 0.0001. Otherwise, about 0.1

/******************* Spectral whitening  **************************/

#define SPECTRALWHITENING 	false
#define COMPRESSIONCOEF		0.33

/****************** Processing parameters ************************/

#define kMINPEAKVALUE   0.1 	// 0.1 Minimum magnitude value for considering a peak
#define MAXPOLYPHONY    6       // Max number of notes detected in each frame
#define MAXHARMONICS   15 	// Max number of harmonics (for efficiency)
#define MAXCANDIDATES  10	// Max number of f0 candidates
#define MINHARMONICS   	0	// Minimum number of harmonics for a f0 candidate
#define STABLEFRAMES	3       // Peak picking is done n frames after onset (if possible) when -onset is used

#define kSMOOTHNESS	4.0	// Smoothness weight (kappa)
#define kLOUDNESS	0.15  	// Minimum note loudness relative to the maximum note in surrounding area
#define kMINLOUDNESS	5  	// Minimum intensity for a HPS
#define kDISCARDH	0.00	// If 0.1, harmonics lower to 10% of maximum spectral pattern value will be discarded
#define LOWESTNOTE	38.0	// Notes with f0 lower than this value will not be considered
#define HIGHESTNOTE	2100.0	// Notes with f0 higher than this value will not be considered

#define F0CANDIDATESCLOSETOPITCH   false	// To discard candidates that are not close to the western fundamental frequencies

#define PEAKLIMIT       11  	// If 0, the high and low limits to select a peak given a center frequency are relative to stepinterharm variable; otherwise, will be +-PEAKLIMIT Hz  
#define kLINEAR 	true	// If true, partials are assumed to be in phase when there is a partial overlap. If false, additivity of power spectrum is considered

/***************** Parameters for onset detection ****************/

#define CONTEXT		0	// Number of context frames for onset detection
#define kLSM		1.0 	// Silence threshold for onset detection
#define NEWPAR		0.18	// Threshold at the end of onset detection stage

/*************** Perceptual weighting (A-weighting curve) *********/

#define ProcessWeighting false  // Perceptual weighting (A-weighting curve)

/*************** Bands parameters **********************************/

#define kMINBANDFREQ	48.9994087219		// MidiNote= G0 (first pitch frequency)
#define MINBANDDISTANCE 15	// Only f0 candidates closer than MINBANDDISTANCE Hz to a center frequency band will be considered if F0CANDIDATESCLOSETOPITCH=true

const double stepinterbands=pow(2.0,1.0/24.0);	// Bands separation (Hz)
const double candrange=pow(2.0,1.0/24.0);	// Allows only f0 candidates in a range relative to freqrange from its closest band
const double stepinterharm=pow(2.0,1.0/24.0); 	// 1/96 = 1/4 semitone. Only used if PEAKLIMIT=0

/*********************** Gaussian smoothness **************/

const int tamgaussian=3;		      // Gaussian window size
static const double gaussian[]={0.21,0.58,0.21}; // Gaussian window values

/*********************** For -clean option **************/

#define kMINDUR		2	// Number of frames to clean non consecutive notes using cleannotes
#define kSIL		4

/*********************** Frame context params **************/

#define FRAMECONTEXT	4

/********************** Quantization grid for -beat option ***************/

#define QUANTIZE	8.0	// minimum note duration = beat/QUANTIZE (i.e., 4 means semiquaver)

/********************** For Mirex ************************************/

#define SHIFT		    3

#define SUBTRACTENERGY 	    false
#define EQUILIBRATELOUDNESS false	// if false, not used

/********************** For -fixsp -fixsp3 options ************************************/

#define kMAXMERGED		5 	// Max number of combinations (M) at each layer in the graph (if kMERGE=false, then it should be about 50)
#define kMERGE			true	// should be true for merging combination saliences when -fixsp or -fixsp3 options are used.

#endif

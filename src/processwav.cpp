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

/***********************************************************************
*  
* Processwav computes peaks from a wavefile. Main call to this process is:
*
* int input_data_wav(char *name , peaksvector& peaks, pinfo& tmpspectruminfo) 
*
* From a wav filename, a peaks vector is obtained. Information about
* the spectral parameters is stored in tmpspectruminfo
*
*************************************************************************/

#include "processwav.h"

/* Global variables, for efficiency */ 

SNDFILE *wavfile;
SF_INFO wavinfo;
int first_window;
int *buf;
int *bufaux;
double *data;
double maxval=-200.0;
double *window;	   	// Window applied for FFT 
double *windowpowered; 	// For the sinusoidal likeness
pinfo spectruminfo;


/*--------------------------------------------

  read wavefile

----------------------------------------------*/

void open_wavfile(char* name) 
{
	if (!(wavfile= sf_open(name, SFM_READ, &wavinfo))) 
	{
	 	printf("Error : File %s can not be processed.\n", name);
	 	sf_perror (NULL);
	 	exit(1);
	}
}
	


/*-------------------------------------------------

 Get input vector from wav file. Decimation ratio is applied

--------------------------------------------------*/

int get_samples(int len, int dr) 
{
	int i,j,k,n_samples,sum;

	if (dr==1) return sf_read_int(wavfile, buf, len);
	else {
		n_samples=sf_read_int(wavfile, bufaux,len*dr);

		for (i=0, j=0; i<len && j<n_samples; i++) {
			sum=0;
			for (k=0;k<dr;j++,k++) 
				sum+=bufaux[j];
			buf[i]=sum/dr;
		}
	        return n_samples/dr;
	}
}


/*----------------------------------------------

 Read wave data

-----------------------------------------------*/

int read_data(double *input) {

	long nread;
	int i,j;
	int overlapped=0;
	int len;
	
	if (spectruminfo.percentage!=0) 
		overlapped=(int)round(100.0/(100.0-(double)spectruminfo.percentage));

	// First window data
	
	if (first_window || spectruminfo.percentage==0) {
	
		nread = get_samples(spectruminfo.N,spectruminfo.dr);
	
		for (i=0; i<nread; i++) 
 		   data[i]=(buf[i]/DIVFACTOR);  // 2^16 + 2^15= 2^21
		len=nread;
	}
	
	// Other windows data (for overlapping)
		
	else {
		for (j=spectruminfo.N/overlapped,i=0; j<spectruminfo.N; j++,i++)
			data[i]=data[j];
		
		nread = get_samples((spectruminfo.N)-i,spectruminfo.dr);
		len=i+nread;
		
	  	for (j=0; i<len; i++,j++)	
			data[i]=(buf[j]/DIVFACTOR);
	}

	// Copy output data

	for (i=0; i<len; i++) 
		input[i]=data[i];
	
	if (len<spectruminfo.N) return 0;
	return len;
}

/*-------------------------------------------
 
 Initialize FFT parameters

------------------------------------------------*/

void initialize_spectrum_parameters() 
{
	/* Create info data */

	spectruminfo.dr=kDR;		// Decimation ratio (fs=fs/dr)
	spectruminfo.percentage=kPERCENTAGE; // Overlapping percentage

	if (wavinfo.samplerate==44100) {
		spectruminfo.res=kRES;

		// Input window size
		if (kN==0) 
			spectruminfo.N=2048;
		else if (kN==1) 
			spectruminfo.N=4096;		
		else if (kN==2) 
			spectruminfo.N=8192;
		else cerr << "Internal error! kN unsupported";
	}
	else if (wavinfo.samplerate==22050) {
		spectruminfo.res=kRES;	// Resolution. (winsize=winsize*res). Adds zeroes at the end of the window

		if (kN==0) 
			spectruminfo.N=1024;
		else if (kN==1) 
			spectruminfo.N=2048;		
		else if (kN==2) 
			spectruminfo.N=4096;
		else cerr << "Internal error! kN unsupported";
	}
	else {
		cerr << "Samplerate" << wavinfo.samplerate << "not supported. Use samplerate 22050 or 44100" << endl;
		exit(-1);
	}
	
	spectruminfo.W=kW;		// Bandwidth for sinsoidal likeness
	spectruminfo.first_band_freq=kMINBANDFREQ;	// Minimum frequency for band processing

	spectruminfo.samplerate= (wavinfo.samplerate)/(spectruminfo.dr);
	spectruminfo.freq_resolution=(double)(spectruminfo.samplerate)/((double)(spectruminfo.N)*spectruminfo.res);
	spectruminfo.winsize= spectruminfo.N*spectruminfo.res;
	spectruminfo.firstsample=(int)round(spectruminfo.W/spectruminfo.freq_resolution); 	// Convert bandwidth (Hz->Samples)
	if (spectruminfo.percentage!=0) spectruminfo.time_resolution=(1.0-spectruminfo.percentage/100.0)* spectruminfo.N / spectruminfo.samplerate;
	else spectruminfo.time_resolution=(double)(spectruminfo.N)/(double)(spectruminfo.samplerate);
	spectruminfo.min_sample=(int)floor(spectruminfo.first_band_freq/spectruminfo.freq_resolution);
}


/*********************************************************************************************

  Interpolation

*********************************************************************************************/

/**
*  Find the maximum of an interpolated quadratic polynomial function
*  giving the samples at three equidistant points at x=0, x=1 and x=2.
*  You can use it for any equidistant samples just by adding x0 to
*  the resulting xmax.
*
*  @pre The function will fail when y0>=y1 or y2>=y1, as it is supposed to
*  be applied after having located a peak in y1.
*  @returns A pair containing xmax,ymax
*  @author David Garcia-Garzon <dgarcia@iua.upf.es>
*/

std::pair<double,double> interpolate(double y0, double y1, double y2)
{
  double a = y0/2 - y1 + y2/2;
  double b = y1 -y0 -a; // = -3*y0/2 + 2*y1 -y2/2;
//  double c = y0;
      
  // From equating to zero the derivate of x*x*a + x*b + c
  double xmax = -b/(a*2);
  // ymax = xmax*xmax*a + b*xmax + c =
  //      = a*b*b/(4*a*a) -b*b/(2*a) + c =
  //      = b*b/(4*a) -b*b/(2*a) + c =
  //      = -b*b/(4*a) + c
  double ymax = b*xmax/2 + y0;
        
  return std::make_pair(xmax, ymax);
}


/*********************************************************************************************

  Functions for sinusoidal likeness

*********************************************************************************************/


void rectangfilter(double *cumsumvector, int len, int width) 
{
	int lenvector= len + spectruminfo.winsize/2 + width;

	double tmp[lenvector];

	// Do the cumulative sum across the vector
	for (int i=1; i< lenvector; i++)
		cumsumvector[i]+=cumsumvector[i-1];

	// Compute correlation
	for (int i=len; i< lenvector; i++)
		tmp[i]=cumsumvector[i]-cumsumvector[i-len];

	// Copy temporary vector into output
	for (int i=0; i< spectruminfo.winsize/2; i++)
		cumsumvector[i]=tmp[i+len];
}


void correctNaNValues(double *val)
{
	for (int i=0; i<spectruminfo.winsize/2; i++)
		if (isNaN(val[i])) {
			cerr << "NaN Found in pos " << i << "!\n";
			val[i]=0.0;
		}
}

void processSL(double *R, double *X, double *slikeness, double Hnorm, int width, double *samplitudes)
{
	int len=width*2+1;

	double cumsumvector[len + spectruminfo.winsize/2 + width];

	int i,j;
	for (i=0, j=spectruminfo.winsize/2-1 ; i< len; i++, j--) {
		if (X[j]!=0) cumsumvector[i]=pow(X[j],2);
		else cumsumvector[i]=0;
	}
	for (i=len; i<(spectruminfo.winsize/2)+len; i++) {
		if (X[i-len]!=0) cumsumvector[i]=pow(X[i-len],2);
		else cumsumvector[i]=0;
	}
	for (i=spectruminfo.winsize/2+len, j=spectruminfo.winsize/2-1; i < len+(spectruminfo.winsize/2)+width; i++, j--) {
		if (X[j]!=0) cumsumvector[i]=pow(X[j],2);
		else cumsumvector[i]=0;
	}

	// Norm of X is calculated using a rectangular FIR filter.
	// cumsumvector useful values after this function range [0...winsize/2]
	rectangfilter(cumsumvector,len,width);

	// Store values in Xnorm vector
	double Xnorm[spectruminfo.winsize];
	j= width;
	for (i=0; i<(spectruminfo.winsize/2); i++,j++)
		Xnorm[i]=cumsumvector[j];

	// Output is computed
	for (i=0; i<spectruminfo.winsize/2; i++) {
		if (Xnorm[i]!=0)
			slikeness[i]=R[i]/(Hnorm*sqrt(Xnorm[i]));
		else slikeness[i]=0.0;
	}
	correctNaNValues(slikeness);
	
	// Estimated amplitudes: UNUSED!!
	for (i=0; i<spectruminfo.winsize/2; i++) 
 		samplitudes[i]=R[i]/Hnorm;
}

// For the sinusoidal likeness

void computesinusoidal(double *input, double Hnorm, double *X, double *slikeness, double *samplitudes) 
{
	double R[spectruminfo.winsize]; 

	// abs(FFT(R))
	fourier_spectrum(input, R, spectruminfo.winsize, spectruminfo.winsize, windowpowered, spectruminfo.N);
	
	// Multiply R by window length
	for (int i=0; i<spectruminfo.winsize/2; i++)
		R[i]*=spectruminfo.winsize;

	// Create vector for cumulative sum (the power spectrum).
	// Cumsumvector contains: part of the negative frequencies (of size len),
	// the positive frequencies and again part of negative frequencies
	// (of size firstsample).

	int width=(int)round(spectruminfo.W/spectruminfo.freq_resolution); 	// Convert bandwidth (Hz->Samples)
	
	processSL(R,X,slikeness,Hnorm,width,samplitudes);
}

double initializeSinusoidal() 
{
	// Create windows (Hanning or Blackman) and powered window
	switch (WINDOW) { 
		case 0: Hanning(window,spectruminfo.N);
			break;
		case 1: Blackman92(window,spectruminfo.N);
			break;
		case 2: Hamhan(window,spectruminfo.N);
			break;
		default: cerr << "Error window: code " << WINDOW;
			exit(-1);
			break;
	}

	for (int i=spectruminfo.N; i<spectruminfo.winsize; i++)
		window[i]=0;

	for (int i=0; i<spectruminfo.winsize; i++)
		windowpowered[i]=window[i]*window[i];

	// Hnorm computation
	double Hnorm, tmp=0;
	for (int i=0; i<spectruminfo.winsize; i++) 
		tmp+=windowpowered[i];
	Hnorm=sqrt(tmp*spectruminfo.winsize);

	return Hnorm;
}

/*-----------

 Compute peaks from spectrum at a given time

------------*/

void computepeaksfromspectrum(mapa2 &orderedspectrumpeaks, double *spectrumvalues, double *sinusoidalvalues, double *sinusoidalamplitudes)
{
	/* The first column is the current time. Wanted frequencies start at the
	2nd column, wich will be the first one of orderedspectrumbands */

	for (int i=1; i< (spectruminfo.winsize / 2) -1; i++) {

        double spcurr= spectrumvalues[i];
	double spprev= spectrumvalues[i-1];
	double spnext= spectrumvalues[i+1];  
        std::pair<double,double> tmp;


	if (!kSINUSOIDAL) {
  
	   if (spcurr > kMINPEAKVALUE &&  spcurr > spprev && spcurr > spnext) {

		  if (INTERPOLATION) {
		  	// Insert interpolated frequency and amplitude for the given peak
		  	tmp=interpolate(spprev,spcurr,spnext);
		  	orderedspectrumpeaks.insert(make_pair((i-1+tmp.first)*spectruminfo.freq_resolution, tmp.second));

		  	if (tmp.second > spectruminfo.maxampval)
				spectruminfo.maxampval=tmp.second;
		  }
		  else {
		  	orderedspectrumpeaks.insert(make_pair((i)*spectruminfo.freq_resolution, spcurr));
			if (spcurr> spectruminfo.maxampval)
				spectruminfo.maxampval=spcurr;
		  }
	    }
	 }
	 else {	

	    if (sinusoidalvalues[i] > kMINSINLIKEVAL && 
		sinusoidalvalues[i] > sinusoidalvalues[i-1] &&
		sinusoidalvalues[i] > sinusoidalvalues[i+1] && spcurr>kMINPEAKVALUE) 
 		{
		  if (INTERPOLATION) {
		 	 // Insert interpolated frequency and amplitude for the given peak

		 	 if (spprev <= spcurr && spcurr >=spnext) {
		 	   tmp=interpolate(spprev, spcurr, spnext);
			   orderedspectrumpeaks.insert(make_pair((i+tmp.first)*spectruminfo.freq_resolution, tmp.second));
			 }
			 else if (spprev >= spcurr &&  spprev >= spnext && i!=0) {
			   tmp=interpolate(spectrumvalues[i-2], spectrumvalues[i-1], spectrumvalues[i]);
			   orderedspectrumpeaks.insert(make_pair((i-2+tmp.first)*spectruminfo.freq_resolution, tmp.second));
			 }
			 else if (spnext>=spprev && spnext>=spcurr && i<spectruminfo.N) {
		  	   tmp=interpolate(spectrumvalues[i], spectrumvalues[i+1], spectrumvalues[i+2]);
			   orderedspectrumpeaks.insert(make_pair((i+tmp.first)*spectruminfo.freq_resolution, tmp.second));
			 }

			 if (spcurr > spectruminfo.maxampval)
			 	spectruminfo.maxampval=tmp.second;
		  }
		  else {
		   	orderedspectrumpeaks.insert(make_pair((i)*spectruminfo.freq_resolution, spcurr));
			if (spcurr> spectruminfo.maxampval)
				spectruminfo.maxampval=spcurr;
		  
		  }
	    }
	 }	
	}
}


void memoryallocation() {
	buf = (int*)malloc(sizeof(int)*(spectruminfo.winsize));
	bufaux = (int*)malloc(sizeof(int)*(spectruminfo.winsize)*(spectruminfo.dr));
	data = (double*)malloc(sizeof(double)*(spectruminfo.winsize));
	window = (double*)malloc(sizeof(double)*(spectruminfo.winsize));
	windowpowered = (double*)malloc(sizeof(double)*(spectruminfo.winsize));
}


void computebands(double *fft, vectorbands& spectralbands, vector<double>& bandvalues, vector<double>& bandstdev, int
fftlen, double& maxbandval) 
{
	if (!bandvalues.empty()) bandvalues.clear();
	if (!bandstdev.empty()) bandstdev.clear();

	for (int i=0; i<(int)spectralbands.size(); i++) {
		double val=spectralbands[i].applybanddBNorm(fft,fftlen);
		if (val>maxbandval) maxbandval=val;
		bandvalues.push_back(val);
		
		if (SPECTRALWHITENING) {
			val=spectralbands[i].applyStDev(fft,fftlen);
			val=pow(val,COMPRESSIONCOEF-1.0);
			
			bandstdev.push_back(val);
		}
	}
}

void generateWeightingVector(double weightingVector[])
{
	weightingVector[0]=0.0;

	for (int i=1; i<spectruminfo.winsize/2; i++) {

		double f=i*spectruminfo.freq_resolution;

		double Af=
		    (pow(12200.0,2.0)*pow(f,4.0))/
		    ((pow(f,2.0)+pow(20.6,2.0))*
		     (pow(f,2.0)+pow(12200.0,2.0))*
		     (pow(pow(f,2.0)+pow(107.7,2.0),0.5))*
		     (pow(pow(f,2.0)+pow(737.9,2.0),0.5)));

		weightingVector[i]=Af/0.794346;
	}
}

void computeWeighting(double spectrum[], double weightingVector[]) 
{
	for (int i=0; i<spectruminfo.winsize/2; i++) {
		spectrum[i]*=weightingVector[i];
	}
}


void mft (mapa2 &orderedspectrumpeaks, double* input)
{
	for (int i=0; i<=SIZEMFTFREQS; i++) {

		int factor=(int)pow(2,i);
		int framesize=spectruminfo.winsize/factor;
		int winsize=framesize/spectruminfo.res;
		double freq_resolution=	(double)(spectruminfo.samplerate)/(double)(framesize);
		
		double mwindow[framesize+1];
		double spectrum[framesize+1];
		
		switch (WINDOW) { 
			case 0: Hanning(mwindow,winsize);
				break;
			case 1: Blackman92(mwindow,winsize);
				break;
			case 2: Hamhan(mwindow,winsize);
				break;
			default: cerr << "Error window: code " << WINDOW << endl;
				exit(-1);
				break;
		}
		int m,k;
		double data[framesize];
		for (m=0,k=spectruminfo.winsize-framesize; k!=spectruminfo.winsize; m++, k++)
			data[m]=input[k]; 


		fourier_spectrum(data, spectrum, framesize, framesize, mwindow, winsize);
	
		for (int j=1; j*freq_resolution < mftfreqs[i] || ( i==SIZEMFTFREQS && (j+1)*freq_resolution<spectruminfo.samplerate/2.0); j++) {
		  if (i==0 || j*freq_resolution > mftfreqs[i-1]) {	
		  	double spcurr=spectrum[j]*factor;  // amplitude correction
			double spprev=spectrum[j-1]*factor;
			double spnext=spectrum[j+1]*factor;
			
			if (spcurr > kMINPEAKVALUE && spcurr > spprev && spcurr > spnext) {
			  	orderedspectrumpeaks.insert(make_pair(j*freq_resolution, spcurr));
				if (spcurr> spectruminfo.maxampval)
					spectruminfo.maxampval=spcurr;
			}
		  }
		}		  
	}
}


int input_data_wav(char *name , peaksvector& peaks, vectorbands& spectralbands, Tmatrix& outbandvalues, pinfo& tmpspectruminfo) 
{
	// Open wavfile
	open_wavfile(name);

	// Initialize spectrum parameters (spectruminfo) 
	initialize_spectrum_parameters();

	// memory allocation for input data
	memoryallocation();
	
	double input[spectruminfo.winsize];
	double spectrum[spectruminfo.winsize+1];
	double weightingVector[spectruminfo.winsize/2];

	double slikeness[spectruminfo.winsize+1];
	double samplitudes[spectruminfo.winsize+1];

	if (!peaks.empty()) peaks.clear();

	// Time axes 
	double curr_time=(spectruminfo.time_resolution)/2.0;

	// Creation of the window 

	switch (WINDOW) { 
		case 0: Hanning(window,spectruminfo.N);
			break;
		case 1: Blackman92(window,spectruminfo.N);
			break;
		case 2: Hamhan(window,spectruminfo.N);
			break;
		default: cerr << "Error window: code " << WINDOW << endl;
			exit(-1);
			break;
	}

	// Initialize sinusoidal likeness 
	double Hnorm=initializeSinusoidal();

	// Bands generation
	generatebands(spectruminfo.first_band_freq, spectruminfo.samplerate/2, spectralbands, spectruminfo.freq_resolution);
	spectruminfo.numbands=spectralbands.size();

	// Weighting vector generation
	if (ProcessWeighting) {
		generateWeightingVector(weightingVector);
	}

	first_window=true;
	int len;
	int n_time=0;

	while ((len=(read_data(input)))) 
	{
		first_window=false;
		
		// Complete input window with zeroes 
		for (int i=len; i<spectruminfo.winsize; i++)
			input[i]=0.0;

		// Update current time 
		curr_time+=spectruminfo.time_resolution;

		// Compute abs(FFT(X))
		fourier_spectrum(input, spectrum, spectruminfo.winsize, spectruminfo.winsize, window, spectruminfo.N);

		// Bands computation, using the spectrum obtained in the previous step
		outbandvalues.resize(n_time+1);
		outbandvalues[n_time].resize(spectruminfo.numbands);
		vector<double> bandval(spectruminfo.numbands);
		vector<double> bandstdev(spectruminfo.numbands);
		computebands(spectrum, spectralbands, bandval, bandstdev, spectruminfo.winsize / 2, spectruminfo.maxbandsval);
		outbandvalues.push_back(bandval);
		bandval.clear();

		/** Spectral withening (by default, unused) **/
		// Linear interpolation of band compression coefficents for spectral whitening. Results are stored in the original spectrum array
		if (SPECTRALWHITENING)
		{
			int i,j;
			double compressioncoefficents[spectruminfo.winsize/2];
			
			for (i=0; i<spectruminfo.winsize/2; i++)
				compressioncoefficents[i]=0.0;

			cout << "Compression coefficents: " << endl;
			for (i=spectralbands[0].centersample, j=1; i<spectruminfo.winsize/2 && j<(int)spectralbands.size(); i++)
			{
				if (i<spectralbands[j].centersample)
				{
					double y1,y2,x1,x2;
					y1=bandstdev[j-1];
					y2=bandstdev[j];
					x1=spectralbands[j-1].centersample;
					x2=spectralbands[j].centersample;
				
					compressioncoefficents[i]= ((y2-y1)/(x2-x1))*(i-x1) + y1;
				}
				else { // i==spectralbands[j].centersample
					compressioncoefficents[i]=bandstdev[j];
					j++;
				}
				cout << compressioncoefficents[i] << " ";
			}
			cout << endl;

			for (int i=0; i<spectruminfo.winsize/2; i++) {
				spectrum[i]*=compressioncoefficents[i];	
			}
		}	

		/** Compute perceptual weighting (by default, unused) **/
		if (ProcessWeighting) 
			computeWeighting(spectrum,weightingVector);

		/** Sinusoidal likeness measure (by default, unused) **/
		if (kSINUSOIDAL)
			computesinusoidal(input,Hnorm,spectrum,slikeness,samplitudes);

		/** Compute peaks **/ 
		mapa2 orderedspectrumpeaks;
		if (!MFT) 
			computepeaksfromspectrum(orderedspectrumpeaks,(double*)spectrum,(double*)slikeness, (double*)samplitudes);
		else 
			mft(orderedspectrumpeaks,input);
		
		// Add peaks to main peaks vector 
		peaks.resize(n_time+1);
		peaks.push_back(orderedspectrumpeaks);
		n_time++;
	}

	free(buf);
	free(bufaux);
	sf_close(wavfile);
	tmpspectruminfo=spectruminfo;

	return n_time;
}

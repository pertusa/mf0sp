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

#include "onsets.h"
#include <math.h>

void selectpeaks (vector<double> &onsetsvect, int total_time) 
{
	vector<double> tmpvector(total_time);

	for (int i=0; i<total_time; i++)
		tmpvector[i]=0;

	// Detect peaks	
	for (int i=1; i< total_time-1; i++) {
		if (onsetsvect[i] > onsetsvect[i-1] && onsetsvect[i] > onsetsvect[i+1])
			tmpvector[i]=onsetsvect[i];
	}
	// Copy output
	onsetsvect=tmpvector;
}

void applyderivative(vector<double> &onebandvector) 
{
	int ntime=onebandvector.size();

	// Make derivative (context is considered if CONTEXT!=0)

	vector<double> tmpvector(onebandvector);
	if (CONTEXT==0) {
		for (int t=1; t<ntime; t++) onebandvector[t]=tmpvector[t]-tmpvector[t-1];
	}
	else {
		double denominator=0.0;
		for (int i=1; i<=CONTEXT; i++)
			denominator+=i*i;
		denominator*=2;

		for (int t=CONTEXT; t<ntime-CONTEXT; t++) 
		{
			double numerator=0.0;
			for (int i=1; i<=CONTEXT; i++) 
				numerator+=(tmpvector[t+i-1]-tmpvector[t-i])/(float)i;
			onebandvector[t]=numerator/denominator;
		}
		for (int t=0; t<CONTEXT; t++) onebandvector[t]=0;
		for (int t=ntime-CONTEXT; t<ntime; t++) onebandvector[t]=0;
	}
	
	// Keep only positive values
	for (int i=0; i<ntime; i++)
		if (onebandvector[i]<0) onebandvector[i]=0;

}

void processmatrix(Tmatrix bands, double maxbandsval, vector<double>& output) 
{

	int nbands=bands[0].size();
	int ntime=bands.size();

	// Create temporal matrix for storing results after relative difference function
	Tmatrix differential;
	differential.resize(ntime);
	for (int i=0; i<ntime; i++)
		differential[i].resize(nbands);

	// Compute relative difference function for each band in time

	vector<double> onebandvector(ntime);
	for (int i=0; i<nbands; i++) {

		for (int j=0; j<ntime; j++) 
			onebandvector[j]=bands[j][i];

		applyderivative(onebandvector);

		// Copy to temporal matrix 
		for (int j=0; j<ntime; j++)
			differential[j][i]=onebandvector[j];
	}

	// Peak picking at each time frame.

	output.resize(ntime);
	for (int i=0; i<ntime; i++) 
	{
		double sumabands=0, sumadiff=0;

		for (int j=0; j<nbands; j++) {
			sumadiff+=differential[i][j];
			sumabands+=bands[i][j];
		}

		if (sumabands>kLSM) 
			output[i]=(sumadiff/sumabands);
		else output[i]=0;
	}
}


void applythreshold(vector<double> &vectonsets, double maxbandsval)
{
	double threshold= NEWPAR; 

	for (int i=0; i<(int)vectonsets.size(); i++)
		if (vectonsets[i] < threshold)
			vectonsets[i]=0;
}

/*---------------------------------------------------------------------------
 Main onset detection function
----------------------------------------------------------------------------*/

void computeonsets(int total_time, double *onsetsvect, Tmatrix &bands,
double maxbandsval, double timeres, char* filename)
{
	/* Algorithm */

	for (int i=0; i<total_time; i++) 
		onsetsvect[i]=0;

	vector<double> vectonsets;
	processmatrix(bands,maxbandsval,vectonsets);

	selectpeaks(vectonsets, total_time);

	applythreshold(vectonsets,maxbandsval);

	for (int i=0; i<total_time; i++)
		onsetsvect[i]=vectonsets[i];

	generateorc(filename);
	generatesco(onsetsvect, total_time, timeres);
}


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

#ifndef SPECTRALPATTERN
#define SPECTRALPATTERN

#include "params.h"
#include "defines.h"
#include "partials.h"
#include <iostream>

using namespace std;

class spectralpattern {

	public:	
		spectralpattern();
		spectralpattern(double freq, int midinote);
		spectralpattern(const spectralpattern &sp);
		friend ostream &operator<<(ostream &output, const spectralpattern sp);
		double ComputeSalience();
		void Interpolate();
		double ComputeCentroid();
		
		// public variables
		double f0;
		int midinote;
		int numharmonics;

		double harmonics[MAXHARMONICS];
		partial* sharedharmonics[MAXHARMONICS];
		bool interpolate[MAXHARMONICS];
		double loudness;
		double sharpness;
		double salience;
		double centroid;

		spectralpattern& operator=(const spectralpattern &sp);
};

#endif

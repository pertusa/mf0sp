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

#ifndef PEAKSATT
#define PEAKSATT

#include "defines.h"
#include <iostream>

class peaksatt {
	public: 

	  peaksatt();
	  peaksatt(int t_ini, int t_end, const mapa2& spectrum_peaks); 
	  peaksatt(const peaksatt& patt);
	
	  mapa2 specpeaks;	// maybe should be a pointer for efficiency
	  int tini;
	  int tend;
};

ostream &operator<<(ostream &output, const peaksatt& patt);

#endif

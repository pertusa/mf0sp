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

#ifndef COMPLEXNOTES
#define COMPLEXNOTES

#include <iostream>
#include <vector>

#include "defines.h"

using namespace std;

class complexnotes {
  public:
	complexnotes() {
		bandf0=0;
		loudness=0;
		if (!harmonics.empty()) harmonics.clear();
	}

	complexnotes(int b, double l, vector<pair <double, double> > h) {
		bandf0= b;
		loudness= l;
		harmonics= h;
	}

	int bandf0;
	double loudness;
	vector<pair <double, double> > harmonics;

};

#define vectorcomplexnotes vector<complexnotes>

#endif

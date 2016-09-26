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

#include "peaksatt.h"

peaksatt::peaksatt() 
{
	tini=0;
	tend=0;
	specpeaks.clear();
}

peaksatt::peaksatt(int t_ini, int t_end, const mapa2 &spectrum_peaks) 
{
	tini=t_ini;
	tend=t_end;

	specpeaks.clear();
	for (mapa2::const_iterator curr=spectrum_peaks.begin(); curr!=spectrum_peaks.end(); curr++)
		specpeaks.insert(*curr);
}

peaksatt::peaksatt(const peaksatt &patt)
{
	tini=patt.tini;
	tend=patt.tend;

	specpeaks.clear();
	for (mapa2::const_iterator curr=(patt.specpeaks).begin(); curr!=(patt.specpeaks).end(); curr++)
		specpeaks.insert(*curr);
}

ostream &operator<<(ostream &output, peaksatt patt)
{
	output << "t_ini=" << patt.tini;
	output << "t_end=" << patt.tend;
	output << "Spectrum peaks=" << patt.specpeaks.size() << endl;
	for (itermapa2 curr=(patt.specpeaks).begin(); curr!=(patt.specpeaks).end(); curr++)
		output << curr->first << "= " << curr->second << endl;

	return output;
}

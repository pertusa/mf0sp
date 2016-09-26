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

#include "ocsound.h"

void generateorc(char *filename)
{
   ofstream fd(ORCFILE, ios::out | ios::trunc);

   if (!fd.is_open()) {
            cerr << "cannot open " << ORCFILE <<  endl;
            exit (-1);
   }

   fd << kORC1 << filename << kORC2;

   fd.close();
}

void generatesco(double *onsetsvect, int ntime, double time_res) 
{
   ofstream fd(SCOFILE, ios::out | ios::trunc);

   if (!fd.is_open()) {
            cerr << "cannot open " << SCOFILE <<  endl;
            exit (-1);
   }

   fd << kSCO1 << ntime*time_res << kSCO2;

   for (int i=0; i<ntime; i++)
   {
	if (onsetsvect[i]!=0)
		fd << "i 2 " << i*time_res << " 0.01\n";
   }
   fd.close();
}


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

#include "combination.h"

ostream& operator<<(ostream &o, Combination &c) {
	o << "Pitches: ";
	for (int i=0; i<(int)c.fundamentals.size(); i++)
		o << c.fundamentals[i] << " ";
	o << endl;
	
	return o;
}

ostream& operator<<(ostream &o, const SimpleCombination &sc) {
	o << "\nPitches: ";
	for (int i=0; i<sc.getSize(); i++)
		o << sc.getPitch(i) << " ";
	o << "\nLoudness: "; 
	for (int i=0; i<sc.getSize(); i++)
		o << sc.getLoudness(i) << " ";
	o << "\nSalience: ";
	o << sc.getSalience();
	o << endl;
	
	return o;
}


/*------------------------------------------------------------------------------------
 Combination gets all possible candidate combinations and returns a vector of 
 vectors (double) in combinations
-------------------------------------------------------------------------------------*/

void getCombination(vector<double> n, int n_column, double r[], int r_size, int r_column, int loop, vcombinations &comb)
{
	int localloop=loop;
	int local_n_column=n_column;
	
	if(r_column>(r_size-1))
	{
		//vdouble v;
                Combination c;

                c.reserve(r_size);
		for (int i=0; i<r_size; i++) { 
			c.addF0(r[i]);
			c.addPitch((int)round(69+12*log(r[i]/440.0)/log(2.0)));
		}

		comb.push_back(c);
		return;
	}
	
	for(int i=0;i<=loop;++i)
	{
		r[r_column]=n[n_column+i];
		++local_n_column;
		getCombination(n,local_n_column,r,r_size,r_column+1,localloop, comb);
		--localloop;
	}
}

/*------------------------------------------------------------------------------------
 GetAllCombinations gets all possible candidate combinations using the
 function Combination and it returns a map containing them
-------------------------------------------------------------------------------------*/

vcombinations GetAllCombinations(mapa2 candidates)
{
	vector<double> vcand;
	vcand.clear();

	for (itermapa2 curr=candidates.begin(); curr!=candidates.end(); curr++) 
		vcand.push_back(curr->first);
	vcombinations combinations;

	int ncand=candidates.size();
	
	if (ncand!=0) {	
		double r[ncand];
		for (int i=1; i<=(int)candidates.size() && i<=MAXPOLYPHONY; i++) 
			getCombination(vcand,0,r,i,0,ncand-i,combinations);
	}
	return combinations;
}

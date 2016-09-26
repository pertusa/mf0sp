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

#ifndef _PARTIALS_
#define _PARTIALS_

#include <iostream>
#include <map>

using namespace std;

class partial {

  public:

    double freq;
    double amp;
    set<double,less<double> > cand;

    partial() {
      freq=0;
      amp=0;
    }
  
    partial(double f0, double freq, double amp)
    {
        this->freq=freq;
        this->amp=amp;
        this->cand.insert(f0);
    }

    void insert(double f0)
    {
      cand.insert(f0);
    }

    void remove(double f0)
    {
      cand.erase(f0);
    }

    int size()
    {
      return cand.size();
    }
    
    bool iscand(double f0)
    {
      return (!(cand.find(f0)==cand.end()));
    }
};

#define mappartials map<double,partial,less<double> >  // Index: spectrum frequency

#endif

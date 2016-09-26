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

#ifndef CHORD
#define CHORD

#include <vector>
#include "defines.h"

using namespace std;

class chord {
 public:
	chord() {
	   t_ini=0;
	   t_end=0;
	   notes.clear();
	}
	
	~chord() {
	   notes.clear();
	}

	chord( notesvector cnotes, int ct_ini, int ct_end) {
		notes=cnotes;
		t_ini=ct_ini;
		t_end=ct_end;
	}

	chord(const chord &chordcopy) {
           t_ini=chordcopy.t_ini;
           t_end=chordcopy.t_end;
           notes=chordcopy.notes;
	}

	int duration() const {
		return t_end-t_ini+1;
	}
	int numnotes() const {
		return notes.size();
	}

	bool isinchord(int note) const {
		for (int i=0; i<(int)notes.size(); i++)
			if (notes[i].first==note)
				return true;
		return false;
	}
	
	notepair getnote(int note) const {
		notepair n;
		for (int i=0; i<(int)notes.size(); i++) {
			if (notes[i].first==note) {
				n=notes[i];
				break;
			}
		}
		return n;
	}

	int t_ini;
	int t_end;
	notesvector notes;  // Notes vector; first component is note number, second is intensity
};

#endif

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

#ifndef PR2MIDI
#define PR2MIDI

#include "midi.h"
#include "chord.h"
#include "gminstruments.h"
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <set>

using namespace std;

//#define INSTRUMENT	GM_CELESTA
#define INSTRUMENT	GM_ACOUSTIC_GRAND_PIANO
//#define INSTRUMENT GM_SYNTHSTRINGS_1

int pr2midi(int prnote);
bool findnote(int note, set<int> activenotes);
int calculatenumberofevents(vectorchords melodychords);
double computemaxloudness(vectorchords melodychords);
void values2dbs(vectorchords &melodychords);
void normalizeloudness(vectorchords &melodychords);
unsigned writenotes( int fd, double time_res, vectorchords melodychords, vectorchords prevchords, bool onsets);
void writemidi (const char *filename, double time_res, double freq_res, vectorchords melodychords, vectorchords prevchords, bool onsets);
void writemiditempo (const char *filename, double time_res, double freq_res, vectorchords melodychords, double tempochanges[], double halfwindowtime);
#endif

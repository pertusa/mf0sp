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

#include <fcntl.h>
#include <unistd.h>
#include "writemidi.h"

/*----------------------------------------------------------------

 Write notes. smf_note parameters: 
   	file
   	current time relative to the previous one
   	note number (0...127)
   	velocity (0...127) 

----------------------------------------------------------------*/

unsigned writenotes (int fd, double time_res, vectorchords melodychords, vectorchords prevchords, bool onsets) 
{
   int eventres= (int)round(2.0*1000*time_res);	// 0.5 seg (*2), milliseconds(*1000)

   set<int> activenotes;
   set<int> tmpactivenotes;
   activenotes.clear();

   long delta_time=0;
 
   unsigned length=0;

   for (int i=0; i<(int)melodychords.size(); i++) 
   {
	tmpactivenotes=activenotes;

	// Note offsets

	for (set<int>::iterator curr=activenotes.begin(); curr!=activenotes.end();curr++) {

		int note=*curr;

		if (onsets) 
		{
 		  if (!(melodychords[i].isinchord(note)) ||
 		       ( melodychords[i].isinchord(note) && !(prevchords[i].isinchord(note))))
                   {
			length+=smf_note_off (fd,delta_time, note, 64, 0);
			tmpactivenotes.erase(note);
			delta_time=0;
                   }
                }
                else {
                  if (!(melodychords[i].isinchord(note)) ||
 		       ( melodychords[i].isinchord(note) && !(melodychords[i-1].isinchord(note))))
                  {
			length+=smf_note_off (fd,delta_time, note, 64, 0);
			tmpactivenotes.erase(note);
			delta_time=0;
                  }
                }
  	}

	if (i!=0) delta_time+=eventres;

	// Note onsets

	for (int j=0; j<(int)melodychords[i].numnotes(); j++) {

		int note=melodychords[i].notes[j].first;
		int loudness=(int)(melodychords[i].notes[j].second);
		
		if (tmpactivenotes.count(note)==0) {
			length+=smf_note_on (fd, delta_time, note,loudness, 0);
			tmpactivenotes.insert(note);
			delta_time=0;
		}
	}

	delta_time+=(long)((melodychords[i].duration()*eventres) - eventres);
	activenotes=tmpactivenotes;
   }
   // Last frame (silent sounding notes)
   if (!activenotes.empty())
   {
     for (set<int>::iterator curr=activenotes.begin(); curr!=activenotes.end();curr++) 
     {
       int note=*curr;
       length+=smf_note_off (fd,delta_time, note, 64, 0);
       activenotes.erase(note);
       delta_time=0;
     }
   }
   
   return length;
}

unsigned writenotestempo (int fd, double time_res, vectorchords melodychords, double tempochanges[], double halfwindowtime) 
{
   set<int> activenotes;	// Sounding notes
   set<int> tmpactivenotes;
   activenotes.clear();

   long delta_time=0;
   int r=960;
   double tempo;
 
   unsigned length=0;

   if (tempochanges[0]!=0) {
     tempo=tempochanges[0];
     length+=smf_tempo_change(fd,tempo);
  }
  else {
    cerr << "INTERNAL ERROR WRITTING TEMPO" << endl;
    exit(-1);
  }
  
   // Advance half window duration (first time is the center of the first frame)
   double beatdur=60.0/tempo;
   double pulsedur=beatdur/(double)r;
   delta_time= (long int)round(halfwindowtime/pulsedur);
    
   for (int i=0; i<(int)melodychords.size(); i++) 
   {
	tmpactivenotes=activenotes;

        if (tempochanges[i]!=0) {
          tempo=tempochanges[i];
          length+=smf_tempo_change(fd,tempo);
        }

	// Note offsets

	for (set<int>::iterator curr=activenotes.begin(); curr!=activenotes.end();curr++) {

		int note=*curr;

                if (!(melodychords[i].isinchord(note)) ||
 		       ( melodychords[i].isinchord(note) && !(melodychords[i-1].isinchord(note))))
                {
			length+=smf_note_off (fd,delta_time, note, 64, 0);
			tmpactivenotes.erase(note);
			delta_time=0;
                }
	}

	// Note onsets
	for (int j=0; j<(int)melodychords[i].numnotes(); j++) {

		int note=melodychords[i].notes[j].first;
		int loudness=(int)(melodychords[i].notes[j].second);
		
		if (tmpactivenotes.count(note)==0) {
			length+=smf_note_on (fd, delta_time, note,loudness, 0);
			tmpactivenotes.insert(note);
			delta_time=0;
		}
	}

        beatdur=60.0/tempo;
        pulsedur=beatdur/(double)r;
        delta_time+=(long int)(round(time_res/pulsedur));
	
	activenotes=tmpactivenotes;
   }
   
   // Last frame (silent sounding notes)
   if (!activenotes.empty())
   {
     for (set<int>::iterator curr=activenotes.begin(); curr!=activenotes.end();curr++) 
     {
       int note=*curr;
       length+=smf_note_off (fd,delta_time, note, 64, 0);
       activenotes.erase(note);
       delta_time=0;
     }
   }
   return length;
}

int calculatenumberofevents(vectorchords melodychords) {
   int numevents=0;

   for (int i=0; i<(int)melodychords.size(); i++) 
    for (int j=0;j<(int)melodychords[i].numnotes(); j++) 
	numevents+=2;

  return numevents;
}


/************** Normalize loudness functions ****************************/

// Compute max loudness (in dBs)

double computemaxloudness(vectorchords melodychords) {
   double maxloudness=0.0;

   for (int i=0; i<(int)melodychords.size(); i++) 
	for (int j=0; j<melodychords[i].numnotes(); j++) 
		if (melodychords[i].notes[j].second > maxloudness)
			maxloudness=melodychords[i].notes[j].second;

   return maxloudness;
}

void values2dbs(vectorchords& melodychords) {
   for (int i=0; i<(int)melodychords.size(); i++) 
	for (int j=0; j<melodychords[i].numnotes(); j++) 
		melodychords[i].notes[j].second= 10*log10(melodychords[i].notes[j].second);
}


void normalizeloudness(vectorchords &melodychords)
{
	values2dbs(melodychords);

	double maxloudness=computemaxloudness(melodychords);

        for (int i=0; i<(int)melodychords.size(); i++) 
  	  for (int j=0; j<melodychords[i].numnotes(); j++) {
		melodychords[i].notes[j].second=(int)round(melodychords[i].notes[j].second *120.0 / maxloudness);
	  }
}


/****************** Main function for writing a MIDI file ***************/

void writemidi (const char *filename, double time_res, double freq_res, vectorchords
melodychords, vectorchords prevchords, bool onsets) 
{
   unsigned short div=0x03C0;	// Division = 1 millisecond

   int fd;
   int dh_midi;
   int p_midi = 0;

   /* open file */

   if ((fd = open (filename, O_RDWR| O_CREAT| O_TRUNC, S_IRUSR| S_IWUSR)) < 0)
   {
       cerr << "cannot open " << filename << endl;
       exit (-1);
   }  
   
   /* Write MIDI header */
   
   smf_header_fmt (fd, 0, 1, div);

   /* Write track header */

   int nmidi=calculatenumberofevents(melodychords);
   nmidi++;

//   int h_midi = p_midi; /* pointer of track-head  */
   smf_track_head (fd, (7+4*nmidi));
   dh_midi=p_midi;

   /* Write program change on channel 0 */
   p_midi+= smf_prog_change (fd, CH_1, INSTRUMENT);

   /* Write notes */
   normalizeloudness(melodychords);
   p_midi+=writenotes (fd, time_res, melodychords, prevchords, onsets);
  
   /* Write track end */
   p_midi+=smf_track_end (fd);

   /* Changes track header*/
   smf_change_head(fd,p_midi);	
   
   /* Close file */ 
   close(fd);                                                                               

}

void writemiditempo (const char *filename, double time_res, double freq_res, vectorchords melodychords, double tempochanges[], double halfwindowtime)
{
   unsigned short div=0x03C0;	// resolution=960

   int fd;
   int dh_midi;
   int p_midi = 0;

   /* open file */

   if ((fd = open (filename, O_RDWR| O_CREAT| O_TRUNC, S_IRUSR| S_IWUSR)) < 0)
   {
       cerr << "cannot open " << filename << endl;
       exit (-1);
   }  
   
   /* Write MIDI header */
   
   smf_header_fmt (fd, 0, 1, div);

   /* Write track header */

   int nmidi=calculatenumberofevents(melodychords);
   nmidi++;

//WriteDelta(fd,64);

//   int h_midi = p_midi; /* pointer of track-head  */
   smf_track_head (fd, (7+4*nmidi));
   dh_midi=p_midi;

   /* Write program change on channel */
   p_midi+= smf_prog_change (fd, CH_1, INSTRUMENT);

   /* Write notes */
   normalizeloudness(melodychords);
   p_midi+=writenotestempo (fd, time_res, melodychords, tempochanges, halfwindowtime);
  
   /* Write track end */
   p_midi+=smf_track_end (fd);

   /* Changes track header*/
   smf_change_head(fd,p_midi);	
   
   /* Close file */ 
   close(fd);                                                                               
} 

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

#ifndef ALG
#define ALG

#include "info.h"
#include "params.h"
#include "bands.h"
#include "chord.h"
#include "spectralpattern.h"
#include "defines.h"
#include "peaksatt.h"
#include "complexnotes.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <deque>

using namespace std;

/* Global variables (for efficiency and code clarity) */
pinfo info;
bool printlog=false;

/*************************** auxiliar functions  ************************************/

/*------------------------------------------------------------------------------------

  Error messages

------------------------------------------------------------------------------------*/

void error(int num, char *sn)
{
     switch (num)
     {
 	case 0: 
  cerr << "\nUsage: " << sn << " <input.wav> [-o <output.mid>]" << 
  " [-eval | -onsetdetection | -beat <beatsfile> | -loadonsets <onsetsfile> (-mergeIOI) | fixsp (-plotintensity | -printgraph <graph.dot>) | fixsp3 (-plotintensity | -printgraph <graph.dot>)]" <<
  " [-log] [-mirexfbf | -mirexntrack] [-writetempo <beatsfile>] [-clean] [--help] [--version]" << 
  "\n If -o option is missing, a file with name <input.mid> is created instead" << endl;
 	        break;
	case 1: cerr << "Filename " << sn << " not found\n";
		break;
	case 2: cerr << "Error frequency; not assigned to any band\n";
		break; 
     }
     exit(-1);
}

/*------------------------------------------------------------------------------------

  This function assumes .wav or .mp3 extension

------------------------------------------------------------------------------------*/

string getmidifilename(char filename[]) {
	string midifilename="";
	
	int i;
	for (i=strlen(filename)-1; i>=0 && filename[i]!='/'; i--);
	if (filename[i]=='/') i++;
	while (i<(int)strlen(filename)-4) {
 		midifilename+=filename[i];
 		i++;
	}
	midifilename+=".mid";
	return midifilename;
}

/*------------------------------------------------------------------------------------

  help option

------------------------------------------------------------------------------------*/

void printHelp(char sn[])
{
  cerr << "\nUsage: " << sn << " <input.wav> [-o <output.mid>]" << 
  " [-eval | -onsetdetection | -beat <beatsfile> | -loadonsets <onsetsfile> (-mergeIOI) | fixsp (-plotintensity | -printgraph <graph.dot>) | fixsp3 (-plotintensity | -printgraph <graph.dot>)]" << 
  " [-log] [-mirexfbf | -mirexntrack] [-writetempo <beatsfile>] [-clean] [--help] [--version]" << 
  "\n If -o option is missing, a file with name <input.mid> is created instead" << endl << endl;

  cerr << "-o <output.mid>\t\t Generates a midifile with name <output.mid>." << endl;
  cerr << "-eval\t\t\t Evaluation mode for a chord database." << endl;
  cerr << "-onsetdetection\t\t Performs the onset detection algorithm described in Pertusa's PhD. \n\t\t\t Important: this option requires that kN=0 and kPERCENTAGE=50.0 in params.h. If other values for these\n\t\t\t parameters are necessary, use any onset detection algorithm first and load the onsets with -loadonsets" << endl;
  cerr << "-beat <beatsfile>\t Loads a beats file (for instance, from beatroot), which is a text file in which each line\n\t\t\t represents a beat time in seconds. Then, the algorithm perform the detection in 1/QUANTIZED units, \n\t\t\t and tempo is estimated from beats and written into MIDI file." << endl;

  cerr << "-loadonsets <onsetsfile> (-mergeIOI)\n\t\t\t Loads an onsets file and performs the detection only in frames after each onset.\n\t\t\t The estimated pitches are yielded for all the IOI, making the algorithm very fast. If -mergeIOI option is\n\t\t\t also present, then the salience of the combinations are merged within all the IOI frames.\n\t\t\t These options are recommended for instruments with easy-detection onsets, like piano or guitar sounds." << endl;

  cerr << "-fixsp (-plotintensity | -printgraph <graph.dot>)\n\t\t\t Performs F0 tracking using a multilayer wDAG, as described in Pertusa PhD. \n\t\t\t -plotintensity plots the intensities of the estimated pitches to visualize them in 3D images using gnuplot. \n\t\t\t -printgraph <graph.dot> stores the wDAG into the specified file, which can be visualized using Graphviz." << endl;
  cerr << "-fixsp3 (-plotintensity | -printgraph <graph.dot>)\n\t\t\t Performs F0 tracking estimating expected note intensities using linear interpolation. \n\t\t\t Like in -fixsp option, the intensities or the graph can be printed." << endl;   

  cerr << "-log\t\t\t Prints log in stdout for debugging (use it only if necessary, it generates a very long output)." << endl;    
  cerr << "-mirexfbf\t\t Prints the detected pitches in stdout using MIREX08 frame by frame format." << endl;
  cerr << "-mirexntrack\t\t Prints the detected pitches in stdout using MIREX08 note tracking format." << endl;
  cerr << "-writetempo <beatsfile>\t Writes tempo into the MIDI file to make the score readable. It requires a beats file." << endl;
  
  cerr << "-clean\t\t\t Postprocessing stage to remove very short notes and to glue notes separated by very short silences. \n\t\t\t Incompatible with options -fixsp and -fixsp3." << endl;
 	       

  cerr << "--help\t\t\t Displays this message." << endl;
  cerr << "--version\t\t Displays the current software version." << endl;

  cerr << endl << "Usage examples: " << endl;
  cerr << "\t" << sn << " input.wav -o output.mid -clean\t\tMIREX08 without f0 tracking" << endl;
  cerr << "\t" << sn << " input.wav -o output.mid -fixsp\t\tMIREX08 with f0 tracking" << endl;
  cerr << "\t" << sn << " input.wav -writetempo beatsfile.txt -loadonsets onsetsfile.txt -mergeIOI -clean -o output.mid\tGets a readable score for piano sounds" << endl;
 
  cerr << endl;
}


/*------------------------------------------------------------------------------------

  getParams processes the program arguments

------------------------------------------------------------------------------------*/

void getParams(int argc, char* argv[], bool &detectonsets, bool &eval, 
string &midifilename, bool &printlog, bool &beat, string &beatfilename, bool
&fileonsets, string &onsetfilename, bool& mergeIOI, bool &fixsp, bool &mirexfbf, 
bool &plotintensity, bool &mirexntrack, bool &clean, bool& printgraph, string &graphfilename, bool &writetempo, bool& fixsp3) 
{
   bool filename=false;

   if (argc<2)
	error(0,argv[0]);
		
   detectonsets=false;
   eval=false;
   printlog=false;
   beat=false;
   fileonsets=false;
   mergeIOI=false;
   fixsp=false;
   mirexfbf=false;
   mirexntrack=false;
   plotintensity=false;
   clean=false;
   printgraph=false;
   writetempo=false;
   fixsp3=false;

   if (!strcmp(argv[1],"--help"))
   {
          printHelp(argv[0]);
          exit(0);
   }
   else if (!strcmp(argv[1],"--version"))
   {
         cout << "Code version: " << kVERSION << endl;
         exit(0);
   }
 
   for (int i=2; i<argc; i++)
   {
	if (!strcmp(argv[i],"-eval")) 
		eval=true;
	else if (!strcmp(argv[i],"-onsetdetection"))
		detectonsets=true;
	else if (!strcmp(argv[i],"-o"))
	{
		i++;
		if (i>=argc) error(0,argv[0]);
		else {
			midifilename=argv[i];
			filename=true;	
		}
	}
	else if (!strcmp(argv[i],"-log")) 
		printlog=true;

        else if (!strcmp(argv[i],"-beat")) 
        {
                i++;
                if (i>=argc) error(0,argv[0]);
                else {
                  beatfilename=argv[i];
                  beat=true;
                }              
	}

        else if (!strcmp(argv[i],"-writetempo")) 
        {
                i++;
                if (i>=argc) error(0,argv[0]);
                else {
                  beatfilename=argv[i];
                  writetempo=true;
                }
	}
	
	else if (!strcmp(argv[i],"-loadonsets")) 
        {
                i++;
                if (i>=argc) error(0,argv[0]);
                else {
                  onsetfilename=argv[i];
                  fileonsets=true;
                }
	}
	else if (!strcmp(argv[i],"-mergeIOI"))
	    mergeIOI=true;
	
        else if (!strcmp(argv[i],"-fixsp"))
            fixsp=true;

        else if (!strcmp(argv[i],"-fixsp3"))
            fixsp3=true;
        
        else if (!strcmp(argv[i],"-mirexfbf"))
            mirexfbf=true;

        else if (!strcmp(argv[i],"-mirexntrack"))
            mirexntrack=true;

        else if (!strcmp(argv[i],"-plotintensity"))
          plotintensity=true;
          
        else if (!strcmp(argv[i],"-printgraph"))
        {
          i++;
          if (i>=argc) error(0,argv[0]);
          else {
               graphfilename=argv[i];
               printgraph=true;
          }
        }
        else if (!strcmp(argv[i],"-clean"))
          clean=true;
     
        else 
  	  error(0,argv[0]);
   }
   // Comprobation of dependences
   if (!fileonsets && mergeIOI) error(0,argv[0]);
   if (printgraph && !fixsp && !fixsp3) error(0,argv[0]);
   if (plotintensity && !fixsp && !fixsp3) error(0,argv[0]);
   if (clean && (fixsp || fixsp3)) error(0,argv[0]);
   if (clean && eval) error(0,argv[0]); 
   
   // Default MIDI filename
   if (!filename) midifilename=getmidifilename(argv[1]);
}


/*------------------------------------------------------------------------------------

 midi2freq converts a midi note number into a frequency 

-------------------------------------------------------------------------------------*/

double midi2freq(int midinote) {
	return(8.1757989156*pow(2.0,((double)midinote/12.0)));
}


/*------------------------------------------------------------------------------------

 freq2midi converts a frequency into a midi note number

-------------------------------------------------------------------------------------*/

int freq2midi(double freq) {
	return ((int)round(69+12*log(freq/440.0)/log(2.0))); // EQUIVALENT TO: return (int)(round(12*log2(freq/440.0))+57);
}

/*------------------------------------------------------------------------------------

 centeredfreq assigns a frequency (in Hz) to the nearest midi note frequency (in Hz)

-------------------------------------------------------------------------------------*/

double centeredfreq(double f) {
	return(midi2freq(freq2midi(f)));
}

/*-----------------------------------------------------------------------------------

 isclosetoband: If target frequency is close to the center frequency of any note in
 the well-tempered scale it returns true. Otherwise, returns false.

-------------------------------------------------------------------------------------*/

bool isclosetoband(double freq)
{
	double bandfreq=centeredfreq(freq); // Center frequency of its closest note

	if (fabs(bandfreq-freq) <= MINBANDDISTANCE || (freq<bandfreq*candrange && freq>bandfreq/candrange)) 
		return true;
	else 
		return false;
}

/*-----------------------------------------------------------------------------------

 selectpeakcloseto selects a partial given a center frequency

-------------------------------------------------------------------------------------*/


double selectpeakcloseto(mapa2 &spectrumpeaks,double centerfreq, int nh, double &maxval) 
{
	// Select a partial in a given range
	
	double selected=-1;
	double lowlimit, highlimit;

	if (centerfreq < info.samplerate/2) 
	{
		if (PEAKLIMIT!=0) {
			lowlimit=centerfreq - PEAKLIMIT;
			highlimit=centerfreq + PEAKLIMIT;
		}
		else {
			lowlimit=(centerfreq / stepinterharm) - info.freq_resolution;
			highlimit=(centerfreq * stepinterharm) + info.freq_resolution;
		}

		maxval=-1;
		itermapa2 it;
		for (it=spectrumpeaks.begin(); it!=spectrumpeaks.end() && (it->first)<=lowlimit; it++);
		
		while (it!=spectrumpeaks.end() && it->first <= centerfreq)
		{
			double bandfactor=(it->first-lowlimit)/(centerfreq-lowlimit);
			double val=it->second*bandfactor; // amplitude * triangular band value
			if (val>maxval) {
				maxval=val;
				selected=it->first;
			}
			it++;	
		}
		
		while (it!=spectrumpeaks.end() && (it->first)<=highlimit)
		{
			double bandfactor=((highlimit-centerfreq)-(it->first-centerfreq))/(highlimit-centerfreq);
			double val=it->second*bandfactor; // amplitude * triangular band value
			if (val > maxval) {
				maxval=val;
				selected=it->first;
			}
		        it++;
		}
	}
	return selected;
}


/*-----------------------------------------------------------------------------------

 printmapa2 prints a mapa2

-------------------------------------------------------------------------------------*/

void printmapa2(mapa2 mymap) 
{
	for (itermapa2 curr=mymap.begin(); curr!=mymap.end(); curr++) {
		cerr << curr->first << "= " << curr->second << endl;
	}
}

/*-----------------------------------------------------------------------------------

 printonsetsvector prints a vector of onsets

-------------------------------------------------------------------------------------*/

void printonsetsvector(double *onsetsvector, int ntime) 
{
   for (int i=0; i<ntime; i++)
             printf("%.4f\n", onsetsvector[i]);

}


#endif

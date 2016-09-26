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

/**************************************************************************************
*  Joint estimation method II							      *
***************************************************************************************/

#include "processwav.h"
#include "onsets.h"
#include "alg.h"
#include "writemidi.h"
#include "partials.h"
#include "combination.h"
#include "graph.h"

/*------------------------------------------------------------------------------

 Get f0 candidates from a set of peaks matching a given criterion 

-------------------------------------------------------------------------------*/

mapa2 GetF0Candidates(mapa2 spectrumpeaks) 
{
    mapa2 candidates;
    mapa3 prob;
    candidates.clear();
    
    for (itermapa2 curr=spectrumpeaks.begin(); curr!=spectrumpeaks.end(); curr++)
    {
	double f0=curr->first;
	double ampf0=curr->second;

        if (f0 >= LOWESTNOTE && f0 <= HIGHESTNOTE && (!F0CANDIDATESCLOSETOPITCH || isclosetoband(f0)))
    	{
    		double last=f0;
		int missingharmonics=0, foundharmonics=0;
    		double amp=ampf0;
    		bool iscandidate=false;

    		if (printlog) cout << "GETCAND: " << f0 << " ";    		
		for (int j=1; last+f0<info.samplerate/2 && j<MAXHARMONICS; j++) 
		{
		 
		        double peakbandamp=0;
			double peak=selectpeakcloseto(spectrumpeaks,last+f0,j+1,peakbandamp);

		    	if (peak==-1.0 || peak<=last) { 
				missingharmonics++;
				last+=f0;
			}
			else { 
			if (printlog)  cout << "FOUND AT " << peak << endl;

				last=peak;
				amp+=peakbandamp;
				foundharmonics++;
				if (!iscandidate && foundharmonics>MINHARMONICS) 
				  iscandidate=true;
			}
		}
		if (iscandidate && amp>kMINLOUDNESS) 
		    prob.insert(make_pair(amp,f0));

                if (printlog) 
                {
                    cout << " IS CANDIDATE:";
                    if (iscandidate) cout << 1 << endl;
                    else cout << 0 << endl;
                    cout << "Intensity: " << amp << endl;
                }
    	}
    }
   
    if (printlog)  cout << "F0 CANDIDATES: " << endl;

    itermapa3 curr;
    int i; 
    for (i=0, curr=prob.begin(); curr!=prob.end() && i<MAXCANDIDATES; curr++, i++) 
    {
    	candidates.insert(make_pair(curr->second,spectrumpeaks[curr->second]));
    	if (printlog) cout << curr->second << " ";
    }
    if (printlog) cout << endl;
    
    return candidates;
}

/*------------------------------------------------------------------------------

 Function used only if EQUILIBRATELOUDNESS=true to normalize the candidate saliences in a combination

-------------------------------------------------------------------------------*/

double equilibrateloudness(Combination &comb)
{
  double mean=0;
  double diff=0;

  int i=0;
  for (vector<spectralpattern>::const_iterator it=comb.spectralpatterns.begin(); it!=comb.spectralpatterns.end(); it++) {
    mean+=it->loudness;
    i++;
  }
  mean/=(double)i;
  
  for (vector<spectralpattern>::const_iterator it=comb.spectralpatterns.begin(); it!=comb.spectralpatterns.end(); it++) 
      diff+=fabs(it->loudness);

  comb.salience=comb.salience-diff;
  if (comb.salience<0) comb.salience=0;

  return comb.salience;
}

/*------------------------------------------------------------------------------

 Salience of a combination is computed using smoothness and intensity for each HPS

-------------------------------------------------------------------------------*/

void ComputeCombinationsSalience (mapa2 &spectrumpeaks, mapa2 &f0candidates, vcombinations &comb) 
{
	// Analyse each possible combination
	double bestsalience=-1000000;
	vector<spectralpattern> vsp;
	mappartials partials;

	vsp.reserve(f0candidates.size());

        // Search harmonics for all the candidates
	for (itermapa2 cand=f0candidates.begin(); cand!=f0candidates.end(); cand++)
	{
	        double f0=cand->first;
	        spectralpattern sp(f0,freq2midi(f0));
                double last=0.0;
	             
                // partials are computed from all the candidates          
                for (int j=0; last+f0<info.winsize/2 && j<MAXHARMONICS; j++) 
                {
                        double amppeakband=0; // Unused, but necessary to use next function
                        double peak=selectpeakcloseto(spectrumpeaks,last+f0,j+1,amppeakband);

                        if (peak==-1.0) { 
                                  sp.harmonics[j]=0;
                                  sp.sharedharmonics[j]=NULL;
                                  last+=f0;
                        }
                        else {
                                  sp.numharmonics=j+1;
                                  double amp=spectrumpeaks[peak];

           		          if (partials.find(peak)==partials.end()) 
                                  {
            		              partial p(f0,peak,amp);
		                      partials[peak]=p;  // The index is the frequency of the peak
                                      sp.harmonics[j]=amp;
                                      sp.sharedharmonics[j]=&partials[peak];
                                  }
                                  else
                                  {
                                      // Update partial info
                                      partials[peak].insert(f0);

                                      // Set spectral pattern values
                                      sp.harmonics[j]=amp; 
                                      sp.sharedharmonics[j]=&partials[peak];
                                   }
                                   last=peak;
                          }
                }
                vsp.push_back(sp);
	} // end partials search

	if (printlog) {
	  for (vector<spectralpattern>::iterator it=vsp.begin(); it!=vsp.end(); it++)
	    cout << "CANDIDATE: " << *it << endl;
        }

        // Search best combination
        mappartials partialsorig=partials;	      
        
        double energyframe=0;
        for (itermapa2 it=spectrumpeaks.begin(); it!=spectrumpeaks.end(); it++)
          energyframe+=it->second;
        
	for (vcombinations::iterator c=comb.begin(); c!=comb.end(); c++) // loop for each combination
	{
	        double energycaptured=0;
                vector<spectralpattern> vspcand;
                
                vspcand.reserve(c->size());
                
                // copy candidates of the combination into a candidates vector vspcand
	        for (int ncand=0; ncand<(int)(c->size()); ncand++)
	        {
                  // search candidate in vector (must always exists)
                  vector<spectralpattern>::const_iterator index;
                  for (index=vsp.begin(); c->getF0(ncand)!=index->f0; index++);
                  vspcand.push_back(*index);
	        }
	        	        
	        if (printlog) {	        
	            cout << "Combination: ";	      
                    for (int i=0; i<(int)vspcand.size(); i++)
                        cout << vspcand[i].f0 << " ";
                    cout << endl;
                }
                
	        // For each harmonic, if it is shared by any of the candidates, then interpolate  
                for (vector<spectralpattern>::iterator it=vspcand.begin(); it!=vspcand.end(); it++)
                {
                  // Mark shared partials of vspcand[i] (for this combination)
                  for (int j=0; j<it->numharmonics; j++)
                  {
                      // Mark shared
                      bool shared=false;
                      if (it->sharedharmonics[j]!=NULL && (int)(it->sharedharmonics[j]->size())!=1)
                      {
                        for (vector<spectralpattern>::iterator k=it+1; k!=vspcand.end() && !shared; k++)
                          if (it->sharedharmonics[j]->iscand(k->f0)) 
                              shared=true;
                      } 
                      if (shared) it->interpolate[j]=true;
                      else it->interpolate[j]=false;
                  }
                }
                          
		// Compute sharpness, intensity and salience
                double salience=0;
                double maxloud=0;
                bool out=false;
                for (vector<spectralpattern>::iterator it=vspcand.begin(); it!=vspcand.end() && !out; it++) {
	    	      // Update harmonics (in previous stages they might be changed)
	    	      for (int j=0; j<it->numharmonics; j++)
	    	      {
                          if (it->sharedharmonics[j]!=NULL)
                               it->harmonics[j]=it->sharedharmonics[j]->amp;
                          else it->harmonics[j]=0;
                      }
                
                      it->Interpolate();
	  	      salience+=it->ComputeSalience();
	  	      
	  	      if (it->loudness<kMINLOUDNESS) 
                      {
                        salience=0;
                        out=true;
                      }
                      else {
  	  	        if (printlog) cout << "Sharpness of " << it->f0 << "= " << it->sharpness << " - Intensity= " << it->loudness << endl;

                        if (it->loudness > maxloud)
                          maxloud=it->loudness;

                        energycaptured+=it->loudness;
                      }
                }
                if (salience!=0) 
                {
                  for (vector<spectralpattern>::iterator it=vspcand.begin(); it!=vspcand.end(); it++)
                  {
                   if (it->loudness < kLOUDNESS*maxloud)
                     salience=0;
                  }

                  if (salience!=0)
                  {
                    if (SUBTRACTENERGY)
                      salience-=(energyframe-energycaptured);

                    if (printlog) cout << "Salience: " << salience << endl;

                    // Update combination data
                    c->salience=salience;
                    c->addsp(vspcand);

                    if (EQUILIBRATELOUDNESS)
                      salience=equilibrateloudness(*c);

                   // If it's the best salience, store the notes
                  if (salience>bestsalience) {
                        if (printlog) cout << " (BEST)" << endl;
			bestsalience=salience;
			//notes=vspcand;
                  }
                  else if (printlog) cout << endl;
                  if (printlog) cout << "--------------------------------------" << endl;
           
                }
                else c->salience=0;
              }  
              // restore amplitudes
              for (mappartials::iterator it=partialsorig.begin(); it!=partialsorig.end(); it++)
                     partials[it->first].amp=(it->second).amp;
	} // endfor combination loop

} // end ComputeCombinationSalience


/*------------------------------------------------------------------------------

 Best combination is added to the song

-------------------------------------------------------------------------------*/

void AddBestCombination(Combination &bestcombination, vectorchords &bestmelodynotes, int tini, int tend)
{
    // Convert combination into midi notes for writemidi function
    notesvector notesok;
    notesok.reserve(bestcombination.size());
	
    for (int k=0; k<bestcombination.size(); k++) {
            notepair note(make_pair(bestcombination.getPitch(k), bestcombination.spectralpatterns[k].loudness));
            notesok.push_back(note);
    }

    // Store chord detected iÂnto a vector of meÃlody chords.
    chord currentchord(notesok, tini, tend);
    bestmelodynotes.push_back(currentchord);
}

/*------------------------------------------------------------------------------

 Best combination is added to the song (as previous function, but when combinations are merged)

-------------------------------------------------------------------------------*/

void AddBestCombination(const vint &bestcombination, vectorchords &bestmelodynotes, int tini, int tend)
{
    // Convert combination into midi notes for writemidi function
    notesvector notesok;
    notesok.reserve(bestcombination.size());
	
    for (int k=0; k<(int)bestcombination.size(); k++) {
            notepair note(make_pair(bestcombination[k], 127));
            notesok.push_back(note);
    }

    // Store chord detected into a vector of melody chords.

    chord currentchord(notesok, tini, tend);
    bestmelodynotes.push_back(currentchord);
}

/*------------------------------------------------------------------------------

   Pitch duplicates within a combination are removed, keeping the one with best salience.
   Combinations with salience==0 are also removed for efficiency.

-------------------------------------------------------------------------------*/

mapa4 CleanFrameCombinations(vcombinations &vc) // by reference for efficiency
{
      mapa4 cleaned;
      for (vcombinations::const_iterator itvc=vc.begin(); itvc!=vc.end(); itvc++)
      {
        if (itvc->salience!=0) 
        {
          itermapa4 it=cleaned.find(itvc->pitches);

          // found
          if (it != cleaned.end()) 
          {
            if (printlog) {
                for (int k=0; k<(int)(it->first).size(); k++) 
                  cout << it->first[k] << " ";
                  cout << endl;
            }
            // With the same pitches, keep only the combination with highest salience
            if (itvc->salience > it->second) 
            {
              cleaned.erase(it);
              cleaned.insert(make_pair(itvc->pitches,itvc->salience));
            }

          }
          // not found
          else 
            if (itvc->salience!=0)
                cleaned.insert(make_pair(itvc->pitches,itvc->salience));
        }
      }
      return cleaned;
}

/*------------------------------------------------------------------------------

 Same function than the previous one but also returning the intensity of each candidate

-------------------------------------------------------------------------------*/

vsimplecombinations CleanFrameCombinationsLoudness(vcombinations &vc) // by reference for efficiency
{
      vsimplecombinations cleaned;

      for (vcombinations::iterator itvc=vc.begin(); itvc!=vc.end(); itvc++)
      {
        bool insert=true;

        if (itvc->salience!=0) 
        {
          bool found=false;

          for (itervsimplecombinations it=cleaned.begin(); it!=cleaned.end() && !found; it++)
          { 
            if (it->getPitches()==itvc->pitches)
            {
                found=true;
                
                if (itvc->salience > it->getSalience())
                {  
                  cleaned.erase(it);
                  insert=true;
                }
                else 
                  insert=false;
            }
          }
        }
        else insert=false;

        if (insert) 
              cleaned.push_back(*itvc);
      }
      return cleaned;
}

/*------------------------------------------------------------------------------

 Returns combination with highest salience from a set of combinations

-------------------------------------------------------------------------------*/

double selectHighestSalience(mapa4 &m, itermapa4 &bestcomb) // by reference for eficiency
{
    double bestsalience=0;

    bestcomb=m.begin();    
    for (itermapa4 it=m.begin(); it!=m.end(); it++)
    {
      if (printlog) {
        cout << "combination: "; 
        for (int i=0; i<(int)(it->first).size(); i++) 
            cout << it->first[i] << " ";
        cout << "salience: " << it->second << endl;
      }
      if (it->second > bestsalience) 
      {
        bestcomb=it;
        bestsalience=it->second;
      }
    }
    return bestsalience;
}


/*------------------------------------------------------------------------------

 Function for merging the saliences of adjacent combinations

-------------------------------------------------------------------------------*/

vint MergeFramesCombinations(const deque<mapa4> &qc)
{
    mapa4 m;
    double bestsalience;
    itermapa4 bestcomb;
    
    // silence at the center frame
    if (qc[(int)qc.size()/2].size()==0) {
      bestsalience=0;
    }
    // otherwise
    else {
      for (int i=0; i<(int)qc.size(); i++)
      {
        // Merge and sum all frames saliences for every frame
        for (mapa4::const_iterator it=qc[i].begin(); it!=qc[i].end(); it++)
        {
          itermapa4 it2=m.find(it->first);
          if (it2 != m.end()) 
            (it2->second)+=it->second;
          else if (it->second!=0)
            m.insert(make_pair(it->first, it->second)); 
        }
      }
      bestsalience= selectHighestSalience(m,bestcomb);
    }
    // Search again the combination to compute note intensity (sum of note intensities) and f0 (average)
   // By the moment, it just returns the first combination found that matches (to make tests)
   if (bestsalience!=0) return bestcomb->first;
   else {
     vint empty;
     return empty;
   }
}

/*------------------------------------------------------------------------------

 The same than previous function, but adding intensities of each candidate accross frames

-------------------------------------------------------------------------------*/

vsimplecombinations MergeFramesCombinationsLoudness(const deque<vsimplecombinations> &qc, double &maxenergy) // qc by reference for efficiency
{
    vsimplecombinations m;
    double bestsalience=0;
    vsimplecombinations::iterator itm;

    maxenergy=0;
    // silence at the center frame
    if (qc[(int)qc.size()/2].size()==0) {
       bestsalience=0;
    }

    // otherwise
    else {
      for (deque<vsimplecombinations>::const_iterator itframe=qc.begin(); itframe!=qc.end(); itframe++)
      {
        // Merge and sum all frames saliences and intensities for every frame
        for (vsimplecombinations::const_iterator itvsc=itframe->begin(); itvsc!=itframe->end(); itvsc++)
        {
          bool found=false;
          for (itm=m.begin(); itm!=m.end() && ! found; itm++)
          {
            if (itvsc->getPitches()==itm->getPitches()) {

                // update intensity
                for (int i=0; i<itm->getSize(); i++) {
                  itm->setLoudness(i,itm->getLoudness(i)+itvsc->getLoudness(i));
                }

                // update salience
                itm->setSalience(itm->getSalience()+itvsc->getSalience());

                if (itm->getSalience()>bestsalience) 
                  bestsalience=itm->getSalience();   
                   
                double energy=itm->energy();
                if (energy>maxenergy)
                  maxenergy=energy;

                found=true;
            }
          }
          if (!found && itvsc->getSalience()!=0) {
              m.push_back(*itvsc);
              if (itvsc->getSalience()>bestsalience) 
                  bestsalience=itvsc->getSalience(); 
              double energy=itvsc->energy();
              if (energy>maxenergy)
                  maxenergy=energy;
          } 
        }
      }
   }

   if (bestsalience!=0) {
     sort(m.begin(), m.end()); 
     reverse(m.begin(), m.end());
     if ((int)m.size()>kMAXMERGED) 
       m.resize(kMAXMERGED);
     return m;
   }
   else {
     vsimplecombinations empty;
     SimpleCombination scempty;
     empty.push_back(scempty);
     return empty;
  }
}

/*-----------------------------------------------------------------------------
 
 Main function for onset detection mode or when onsets are loaded from a file

------------------------------------------------------------------------------*/

void applyhypothesisonsets(peaksattvector &peaks, vectorchords &bestmelodynotes) 
{
	for (iterpeaksattvector curr=peaks.begin(); curr!=peaks.end(); curr++)
	{
		if (printlog) {
		  cout << "TIME: " << curr->tini << endl;
		  cout << "PEAKS: " << endl;
		  for (itermapa2 it=curr->specpeaks.begin(); it!=curr->specpeaks.end(); it++)
			cout << "F= " << it->first << " A= " << it->second << endl;
                }
                
                // Get f0 candidates
		mapa2 f0candidates=GetF0Candidates(curr->specpeaks);		
		if (printlog) {
		  for (itermapa2 imp=f0candidates.begin(); imp!=f0candidates.end(); imp++)
		  cout << "Candidate: " << imp->first << " (" << freq2midi(imp->first) << ")" << endl;
                }
   
                // Get combinations of candidates
		vcombinations comb= GetAllCombinations(f0candidates);
		
		// Compute saliences of each combination at the target frame
		if (!comb.empty()) 
			ComputeCombinationsSalience(curr->specpeaks, f0candidates, comb);
                
                double bestsalience=0;
                Combination *bestcombination=NULL;
                for (int j=0; j<(int)comb.size(); j++)
                {
                  if (comb[j].salience > bestsalience)
                  {
                    bestsalience=comb[j].salience;
                    bestcombination=&comb[j]; 
                  }
                }
                // Add best combination to melody notes
                if (bestcombination!=NULL)
                  AddBestCombination(*bestcombination, bestmelodynotes, curr->tini, curr->tend);
                else {
                  Combination empty;
                  AddBestCombination(empty,bestmelodynotes, curr->tini, curr->tend);
                }
	}
}

/*------------------------------------------------------------------------------

 Function to estimate the tempo changes using the beats vector

-------------------------------------------------------------------------------*/

void computeTempoChanges(vector<double> beatsvector, double tempochanges[], int size)
{
  for (int i=0; i<size; i++)
    tempochanges[i]=0;
  
  int index1=0;
  int index2=0;
  double inittempo=0;
  double tempo=120;
  int numtempochanges=0;
  
  for (int i=0; i<(int)beatsvector.size(); i++)
  {
       index2=(int)round(beatsvector[i]/info.time_resolution); 
       tempo=60.0/((index2-index1)*info.time_resolution);
       tempochanges[index1]=tempo;
       index1=index2;
       numtempochanges++;
       if (numtempochanges==2) 
         inittempo=tempo; 
  }
  // last tempo (equals to the previous one)
  tempochanges[index1]=tempo;

  // First tempo change adjusted
  if (tempochanges[0]>400)
  {
    if (inittempo!=0)
      tempochanges[0]=inittempo;
    else tempochanges[0]=120;
  }
}

/*------------------------------------------------------------------------------

 Function to add marks at the positions of the quantization grid given a beats vector

-------------------------------------------------------------------------------*/

void quantizebeats(vector<double> beatsvector, bool quantized[], int sizequantized)
{
  for (int i=0; i<sizequantized; i++)
    quantized[i]=false;

  for (int i=0; i<(int)beatsvector.size(); i++)
  {
    int index=(int)round(beatsvector[i]/info.time_resolution);
    quantized[index]=true;	// mark beat

    if (i+1<(int)beatsvector.size()) {	// mark half, semiquavers, etc.
    
      double q=((beatsvector[i+1]-beatsvector[i])/(double)QUANTIZE);
      double lastq=q;

      for (int j=2; j<=QUANTIZE; j++) {
        index=(int)round((beatsvector[i]+lastq)/info.time_resolution);
        quantized[index]=true;
        lastq+=q;
      }
    }
    else quantized[(int)round(beatsvector[i]/info.time_resolution)]=true;
  } 
  
  if (printlog) {
    cout << "SIZEQ=" << sizequantized << endl;
    for (int i=0; i<sizequantized; i++)
      if (quantized[i]) cout << i*info.time_resolution << endl;
  }
}

/*------------------------------------------------------------------------------

 Function to convert onset times into a binary vector in which each position corresponds with a frame

-------------------------------------------------------------------------------*/

void quantizeonsets(vector<double> onsetsvector, bool quantized[], int sizequantized)
{
  for (int i=0; i<sizequantized; i++)
    quantized[i]=false;

  for (int i=0; i<(int)onsetsvector.size(); i++)
  {
    int index=(int)round(onsetsvector[i]/info.time_resolution);
    quantized[index]=true;
  } 
  
  if (printlog) {
    cout << "SIZEQ=" << sizequantized << endl;
    for (int i=0; i<sizequantized; i++)
      if (quantized[i]) cout << i*info.time_resolution << endl;
  }
}


/*-----------------------------------------------------------------------------

 Postprocessing stage (-clean option)

------------------------------------------------------------------------------*/

void cleannotes(vectorchords& melodychords)
{
  vectorchords tmpchords;
  map<int,int> activenotes;

  int silences[127];

  for (int i=0; i<127; i++)
    silences[i]=-1;

  // connect notes separated by a single frame
  for (int i=0; i<(int)melodychords.size(); i++)
  {
    for (int j=0; j<127; j++) 
    {
      bool found=melodychords[i].isinchord(j);
      if (!found && silences[j]!=-1) {
          silences[j]++;
      }
      else if (found) {
        int pitch=j;

        if (silences[pitch]<=kSIL && silences[pitch]!=-1)  // short silence
        {
           for (int k=1; k<=silences[pitch]; k++) 
             melodychords[i-k].notes.push_back(make_pair(pitch,127));
        }
        silences[pitch]=0;
      }
    }
  }

  // delete notes with a duration shorter than kMINDUR
  activenotes.clear();
  for (int i=0; i<(int)melodychords.size(); i++)
  {
    for (int j=0; j<melodychords[i].numnotes(); j++) 
    {
      if (activenotes.count(melodychords[i].notes[j].first)==0)
        activenotes[melodychords[i].notes[j].first]=i; // note on
    }  
   
    notesvector tmpnv;
    chord tmp(tmpnv,melodychords[i].t_ini+kMINDUR, melodychords[i].t_end+kMINDUR);
    tmpchords.push_back(tmp);

    for (map<int,int>::iterator it=activenotes.begin(); it!=activenotes.end(); it++)
    {
      if (!melodychords[i].isinchord(it->first)) // note off
      {
        if (i-(it->second) > kMINDUR || (melodychords[i].isinchord((it->first)+1) && !melodychords[i-1].isinchord((it->first)+1)) || (melodychords[i].isinchord((it->first)-1) && !melodychords[i-1].isinchord((it->first)-1))) {
          for (int j=it->second; j<i; j++) {
            if (!tmp.isinchord(it->first)) 
              tmpchords[j].notes.push_back(make_pair(it->first, 127));
          }
        } 
        activenotes.erase(it->first);
     }
    }
  }
  
  melodychords=tmpchords;
}

/*-----------------------------------------------------------------------------
 
 Main function for -beat and -mergeIOI modes. Combinations are merged given a beats (or onsets) grid.

------------------------------------------------------------------------------*/

void applyhypothesisbeat(peaksattvector &peaks, vectorchords &bestmelodynotes, bool quantized[])
{
        deque<mapa4> allcombinations;
        int nframes=0;

        int lastframe=0;

        int halfwindow= (int)round(((double)info.N/(double)info.samplerate)/info.time_resolution)/2;
	for (iterpeaksattvector curr=peaks.begin(); curr!=peaks.end(); curr++)
	{
		if (printlog) {
		  cout << "TIME: " << curr->tini << endl;
		  cout << "PEAKS: " << endl;
		  for (itermapa2 it=curr->specpeaks.begin(); it!=curr->specpeaks.end(); it++)
			cout << "F= " << it->first << " A= " << it->second << endl;
                }
                
                // Get f0 candidates
		mapa2 f0candidates=GetF0Candidates(curr->specpeaks);		
		if (printlog) {
		  for (itermapa2 imp=f0candidates.begin(); imp!=f0candidates.end(); imp++)
		  cout << "Candidate: " << imp->first << " (" << freq2midi(imp->first) << ")" << endl;
                }
   
                // Get combinations of candidates
		vcombinations comb= GetAllCombinations(f0candidates);
		
		mapa4 cleaned;
		
		// Compute saliences of each combination
		
		if (!comb.empty()) { 
			ComputeCombinationsSalience(curr->specpeaks, f0candidates, comb);

                        // combinations within a frame are cleaned, removing duplicates (ej: 46-44, 46-44) and keeping the 
                        // combination with highest salience
                        cleaned=CleanFrameCombinations(comb);
                }
                
                if (quantized[nframes+halfwindow]==true) 
                {
                    vint bestcomb=MergeFramesCombinations(allcombinations);
                    for (int i=lastframe; i<nframes; i++)
                      AddBestCombination(bestcomb, bestmelodynotes, i, i);
                
                    allcombinations.clear();
                    allcombinations.push_back(cleaned);
                    lastframe=nframes;
                }
                else {
                    allcombinations.push_back(cleaned);
                }
                nframes++;
	}
}

/*------------------------------------------------------------------------------

 Function to print the intensities of the estimated pitches (-plotintensity)

-------------------------------------------------------------------------------*/

void printloudnessmatrix(vsimplecombinations &vbestcomb)
{
  int minnote=26, maxnote=98;
  int maxtime=500;
  
  for (int i=minnote; i<maxnote; i++)
  {
    int k=0;
    for (vsimplecombinations::iterator it=vbestcomb.begin(); it!=vbestcomb.end() && k<maxtime; it++)
    {
        double value=0;
        for (int j=0; j<it->getSize(); j++)
          if (i==it->getPitch(j))
            value=it->getLoudness(j);
        cout << value << " ";
        k++;
    }
    cout << endl;
  }
}

/*------------------------------------------------------------------------------

 Function to compute the intensity difference D(vi,vj) between two combinations (for -fixsp, -fixsp3 options)

-------------------------------------------------------------------------------*/

double noteenergydiff(const SimpleCombination& sc1, const SimpleCombination &sc2)
{
  double energydiff=0;
  vector<int> all;
  int notfound=0;

  if (sc1.getSize()==0 || sc2.getSize()==0) return 0;
  
  // ->
  vector<double>::const_iterator itsc1loud=sc1.getLoudness().begin();
  for (vector<int>::const_iterator itsc1pitch=sc1.getPitches().begin(); itsc1pitch!=sc1.getPitches().end(); itsc1pitch++, itsc1loud++)
  {
    bool found=false;
    vector<double>::const_iterator itsc2loud=sc2.getLoudness().begin();
    for (vint::const_iterator itsc2pitch=sc2.getPitches().begin(); !found && itsc2pitch!=sc2.getPitches().end(); itsc2pitch++, itsc2loud++)
    {
      if (*itsc1pitch==*itsc2pitch)
      {
        all.push_back(*itsc1pitch);
        found=true;
        energydiff+=fabs(*itsc1loud-*itsc2loud);
      }
    }
    if (!found) {
      // search for pitch (accepted)
      itsc2loud=sc2.getLoudness().begin();
      for (vint::const_iterator itsc2pitch=sc2.getPitches().begin(); !found && itsc2pitch!=sc2.getPitches().end(); itsc2pitch++, itsc2loud++)
      {
        if (*itsc2pitch==(*itsc1pitch)+1 || *itsc2pitch==(*itsc1pitch)-1) 
        {
          found=true; // found pitch
          energydiff+=fabs(*itsc1loud-*itsc2loud);
          all.push_back(*itsc1pitch);
          all.push_back(*itsc2pitch);
        }
      }
      if (!found) {
        energydiff+=fabs(*itsc1loud);
        notfound++;
      }
    }
  }

  // <-
  vector<double>::const_iterator itsc2loud=sc2.getLoudness().begin();
  for (vector<int>::const_iterator itsc2pitch=sc2.getPitches().begin(); itsc2pitch!=sc2.getPitches().end(); itsc2pitch++, itsc2loud++)
  {
    if (find(all.begin(), all.end(), *itsc2pitch) == all.end()) {
        energydiff+= *itsc2loud;
        notfound++;
    }
  }

  return energydiff;
}

/*------------------------------------------------------------------------------

 Function to get the weight between two combinations in the wDAG

-------------------------------------------------------------------------------*/

double getEdgeWeight(const SimpleCombination& sp1, const SimpleCombination& sp2)
{
  double weight;
  
   // energy differences sum
  double energydiff=noteenergydiff(sp1,sp2);

  // weigth is multiplied by 10^6 for clarity of the graph weigths (it is a constant value that does not affect the results).
  weight=(1000000.0/(sp2.getSalience()+1.0))*(energydiff+1.0);
  
  return weight;
}

/*-----------------------------------------------------------------------------
 
 Main function for f0 tracking (-fixsp option), used in MIREX08

------------------------------------------------------------------------------*/

void applyhypothesisfixsp(peaksattvector &peaks, vectorchords &bestmelodynotes, bool plotintensity, bool printgraph, const char *graphfilename)
{
        deque<vsimplecombinations> combinations;
        vector<vsimplecombinations> allmergedcombinations;
        
        int nframes=0;
        unsigned long numnodes=0;
        vector<string> nodenames;
        vector<Edge> edges;
        vector<float> weights;

        vsimplecombinations vbestcomb;
        vbestcomb.reserve(peaks.size());

        // first frames (empty detection)
        for (int i=0; i<(int)(FRAMECONTEXT/2); i++) {
    	    SimpleCombination vempty;
       	    vbestcomb.push_back(vempty);
        }

        nodenames.push_back("init");
        numnodes++;
        bool first=true;

	for (iterpeaksattvector curr=peaks.begin(); curr!=peaks.end(); curr++)
	{
	        // Get candidates
		mapa2 f0candidates=GetF0Candidates(curr->specpeaks);

                // Get combinations of candidates
		vcombinations comb= GetAllCombinations(f0candidates);
		
		vsimplecombinations cleaned;

		// Compute saliences of each combination
		if (!comb.empty()) { 
			ComputeCombinationsSalience(curr->specpeaks, f0candidates, comb);
                        // combinations within a frame are cleaned, removing duplicates (ej: 46-44, 46-44) and keeping the 
                        // combination with highest salience
                        cleaned=CleanFrameCombinationsLoudness(comb);
                }
                
                if (nframes>=FRAMECONTEXT) 
                {
                    double energy=0;
                    vsimplecombinations sc;
                    
                    if (kMERGE) sc=MergeFramesCombinationsLoudness(combinations,energy);

                    else {
                      sort(cleaned.begin(), cleaned.end());
                      reverse(cleaned.begin(), cleaned.end());
                      cleaned.resize(kMAXMERGED);
                      sc=cleaned;
                    }
                                    
                    combinations.pop_front();
                    combinations.push_back(cleaned); 

                    for (vsimplecombinations::iterator it=sc.begin(); it!=sc.end(); it++)
                    {
                      it->setid(numnodes);
                      numnodes++;
                      stringstream os;
                      os << nframes;
                      nodenames.push_back("t"+os.str()+"n"+it->getname());
                      
                      if (first)	// first node -> connect with init
                      {
                            Edge e(0, it->getid());
                            edges.push_back(e);
                            weights.push_back(0);
                      }

                      else if (sc.begin()->getSize()!=0) {

                        for (vsimplecombinations::const_iterator itvs=(allmergedcombinations.end()-1)->begin(); itvs!=(allmergedcombinations.end()-1)->end(); itvs++) 
                        {
                            // compare itsc with itvs
                            double w=getEdgeWeight(*itvs,*it);

                            // add edge_array value
                            Edge e(itvs->getid(), it->getid());
                            edges.push_back(e);
          
                            // add weight to weights vector
                            weights.push_back(w);
                        }
                      }
                      else {	// silence detected
                          for (vsimplecombinations::const_iterator itvs=(allmergedcombinations.end()-1)->begin(); itvs!=(allmergedcombinations.end()-1)->end(); itvs++) 
                          {
                            Edge e(itvs->getid(), it->getid());
                            edges.push_back(e);
                            weights.push_back(0);
                          }
                      }
                    }
                    if (first) first=false;
                    allmergedcombinations.push_back(sc);

                }
                else {
                    combinations.push_back(cleaned);
                    allmergedcombinations.push_back(cleaned);
                }
                nframes++;
	}

        nodenames.push_back("end");
        for (vsimplecombinations::const_iterator itvs=(allmergedcombinations.end()-1)->begin(); itvs!=(allmergedcombinations.end()-1)->end(); itvs++) 
        {
          //  cout << "IDITVS=" << itvs->getid() << endl;
          Edge e(itvs->getid(),numnodes);
          edges.push_back(e);
          weights.push_back(0);
        }
        numnodes++;



        // Intensity computation (best combinations only)
	cerr << "Postprocessing..." << endl;

	if (plotintensity) printloudnessmatrix(vbestcomb);

        vector<int> shortestpath=buildgraph(&nodenames[0], numnodes, &edges[0], &weights[0], (int)edges.size(), printgraph, graphfilename);
        
        vector<vsimplecombinations>::iterator it2=allmergedcombinations.begin()+FRAMECONTEXT;
        for (vector<int>::iterator it=shortestpath.begin(); it!=shortestpath.end() && it2!=allmergedcombinations.end(); it++, it2++)
        {
          bool found=false;
          vsimplecombinations::iterator itsc;
          for (itsc=it2->begin(); itsc!=it2->end() && !found; itsc++)
          {
            if (itsc->getid()==*it) {
              vbestcomb.push_back(*itsc);              
              found=true;
            }
          }
          if (!found) {
                    cerr << "INTERNAL ERROR!!!!\n";
                    exit(-1);
          }
        }
        
	// last frames (empty detection)
	for (int i=0; i<(int)(FRAMECONTEXT/2); i++) {
    	    SimpleCombination s;
       	    vbestcomb.push_back(s);
        }

	// 3 - write best combinations
	int i=FRAMECONTEXT;
        for (vsimplecombinations::const_iterator it=vbestcomb.begin(); it!=vbestcomb.end(); it++, i++) 
            AddBestCombination(it->getPitches(),bestmelodynotes,i,i);
            
}	


/*-----------------------------------------------------------------------------
 
 Function to create a wDAG node (-fixsp3 option only)

------------------------------------------------------------------------------*/

void createNode(SimpleCombination &sc, unsigned long &numnodes, int nframes, vector<string> &nodenames, vector<map <int, SimpleCombination> > &allmergedcombinations)
{
        sc.setid(numnodes);
                      
        stringstream os;
        os << nframes << "a" << numnodes; 
        
        nodenames.push_back("t"+os.str()+"n"+sc.getname());

        allmergedcombinations[nframes].insert(make_pair(numnodes,sc));
        numnodes++;
}


/*-----------------------------------------------------------------------------
 
 Function to interpolate a candidate intensity using previous detections (-fixsp3 option only)

------------------------------------------------------------------------------*/

SimpleCombination predictCombination(const SimpleCombination& sc1, const SimpleCombination& sc2)
{
  vector<int> pitches;
  vector<double> loudness;

  for (vector<int>::const_iterator it2=sc2.getPitches().begin(); it2!=sc2.getPitches().end(); it2++)
  {
       double predictedloudness;

       int j=it2-sc2.getPitches().begin();

       vector<int>::const_iterator it=find(sc1.getPitches().begin(), sc1.getPitches().end(), *it2);
       if (it != sc1.getPitches().end())
       {
           int i=it-sc1.getPitches().begin();
           predictedloudness=2*sc2.getLoudness(j)-sc1.getLoudness(i);
       }
       else predictedloudness=2*sc2.getLoudness(j);
       
       if (predictedloudness>0)  {       
         pitches.push_back(*it2);
         loudness.push_back(predictedloudness);
       }
  }

  for (vector<int>::const_iterator it=sc1.getPitches().begin(); it!=sc1.getPitches().end(); it++)
  {
      
      if (find(sc2.getPitches().begin(), sc2.getPitches().end(), *it)==sc2.getPitches().end())
      {
        int i=it-sc1.getPitches().begin();
        
        pitches.push_back(*it);
        loudness.push_back(0-sc1.getLoudness(i));
      } 
  }

  SimpleCombination sc(pitches,loudness);

  return sc;  
}


/*-----------------------------------------------------------------------------
 
 Main function for f0 tracking (-fixsp3 option) using linear interpolation of HPS intensities

------------------------------------------------------------------------------*/

void applyhypothesisfixsp3(peaksattvector &peaks, vectorchords &bestmelodynotes, bool plotintensity, bool printgraph, const char *graphfilename)
{
        deque<vsimplecombinations> combinations;
        vector <map <int, SimpleCombination> > allmergedcombinations(peaks.size());
        map<int, SimpleCombination> predictedinframe, prevpredictedinframe;
        
        int nframes=0;
        unsigned long numnodes=0;
        vector<string> nodenames;
        vector<Edge> edges;
        vector<float> weights;

        vsimplecombinations vbestcomb;
        vbestcomb.reserve(peaks.size());

        // first frames (empty detection)
        for (int i=0; i<(int)(FRAMECONTEXT/2); i++) {
    	    SimpleCombination vempty;
       	    vbestcomb.push_back(vempty);
        }

        nodenames.push_back("init");
        numnodes++;
        bool first=true;

	for (iterpeaksattvector curr=peaks.begin(); curr!=peaks.end(); curr++)
	{

	        // Get candidates
		mapa2 f0candidates=GetF0Candidates(curr->specpeaks);

                // Get combinations of candidates
		vcombinations comb= GetAllCombinations(f0candidates);
		
		vsimplecombinations cleaned;

		// Compute saliences of each combination
		if (!comb.empty()) { 
			ComputeCombinationsSalience(curr->specpeaks, f0candidates, comb);
                        // combinations within a frame are cleaned, removing duplicates (ej: 46-44, 46-44) and keeping the 
                        // combination with highest salience
                        cleaned=CleanFrameCombinationsLoudness(comb);
                }
                
                if (nframes>=FRAMECONTEXT) 
                {
                    double energy=0;
                    vsimplecombinations sc;
                    
                    if (kMERGE) sc=MergeFramesCombinationsLoudness(combinations,energy);

                    else {
                        sort(cleaned.begin(), cleaned.end());
                        reverse(cleaned.begin(), cleaned.end());
                        cleaned.resize(kMAXMERGED);
                        sc=cleaned;
                    }
                                    
                    combinations.pop_front();
                    combinations.push_back(cleaned); 

                    for (vsimplecombinations::iterator it=sc.begin(); it!=sc.end(); it++)
                    {
                      if (first)	// first frame -> connect all nodes with init
                      {
                            createNode(*it,numnodes,nframes, nodenames, allmergedcombinations);

                            Edge e(0, it->getid());
                            edges.push_back(e);
                            weights.push_back(1000000.0/(it->getSalience()+1.0));

                            predictedinframe.insert(make_pair(it->getid(), *it));
                      }

                      else if (sc.begin()->getSize()!=0) {

                        map<SimpleCombination,int> actualcomb;
                        
                        for (map<int,SimpleCombination>::const_iterator itvs=allmergedcombinations[nframes-1].begin(); itvs!=allmergedcombinations[nframes-1].end(); itvs++) 
                        {
                            map<SimpleCombination, int>::iterator actualcombit=actualcomb.find(itvs->second);

                            if (actualcombit==actualcomb.end()) // if node was not found at previous frame
                            {
                              actualcomb.insert(make_pair(itvs->second,itvs->first));
                              createNode(*it,numnodes,nframes, nodenames,allmergedcombinations);

                              // get weight
                              
                              map<int, SimpleCombination>::const_iterator itpf=prevpredictedinframe.find(itvs->first);
                              if (itpf==prevpredictedinframe.end()) {
                                cout << "INTERNAL ERROR - 1 !\n FRAME= " << nframes-1 << " ITVS ID=" << itvs->first << "\n" << itvs->second << endl;
                                for (map<int,SimpleCombination>::const_iterator itborrame=prevpredictedinframe.begin(); itborrame!=prevpredictedinframe.end(); itborrame++)
                                    cout << "ID: " << itborrame->first << "\n" << itborrame->second << endl;
                                cout << "ALLMERGEDCOMBINATIONS SIZE=" << allmergedcombinations[nframes-1].size() << endl;
                                for (map<int,SimpleCombination>::const_iterator itborrame=allmergedcombinations[nframes-1].begin(); itborrame!=allmergedcombinations[nframes-1].end(); itborrame++)
                                    cout << "ID: " << itborrame->first << "\n" << itborrame->second << endl;
                                exit(-1);
                              }
                              double w=getEdgeWeight(itpf->second,*it);
  
                              // add edge_array value
                              Edge e(itvs->first, it->getid());
                              edges.push_back(e);
          
                              // add weight to weights vector
                              weights.push_back(w);
                              
                              predictedinframe.insert(make_pair(it->getid(), predictCombination(itpf->second,*it)));
                            }
                            else { // connect existing nodes but don't create a new node

                              // get weight
                              map<int, SimpleCombination>::const_iterator itpf=prevpredictedinframe.find(itvs->first);
                              if (itpf==prevpredictedinframe.end()) {
                                cout << "INTERNAL ERROR - 2!\n";
                                exit(-1);
                              }
                              double w=getEdgeWeight(itpf->second,*it);
                              
                              // get edge_array value
                              Edge e(itvs->first ,it->getid());
                              edges.push_back(e);
                              
                              // add weight to weights vector
                              weights.push_back(w);
                            }
                        }
                      }
                      else {	// silence detected
                          createNode(*it,numnodes,nframes, nodenames, allmergedcombinations);

                          for (map<int,SimpleCombination>::const_iterator itvs=allmergedcombinations[nframes-1].begin(); itvs!=allmergedcombinations[nframes-1].end(); itvs++) 
                          {
                            Edge e(itvs->first, it->getid());
                            edges.push_back(e);
                            weights.push_back(0);
                          }
                          SimpleCombination empty;
                          predictedinframe.insert(make_pair(it->getid(),empty));
                      }
                    }
                    if (first) first=false;
                }
                else {
                    combinations.push_back(cleaned);
                }
                nframes++;
                
                prevpredictedinframe=predictedinframe;
                predictedinframe.clear();
	}

        nodenames.push_back("end");
        for (map<int, SimpleCombination>::const_iterator itvs=allmergedcombinations[nframes-1].begin(); itvs!=allmergedcombinations[nframes-1].end(); itvs++) 
        {
          Edge e(itvs->first,numnodes);
          edges.push_back(e);
          weights.push_back(0);
        }
        numnodes++;

        // Intensity computation (only for best combinations)
	cerr << "Postprocessing..." << endl;

        vector<int> shortestpath=buildgraph(&nodenames[0], numnodes, &edges[0], &weights[0], (int)edges.size(), printgraph, graphfilename);
        
        vector<map <int, SimpleCombination> >::const_iterator it2=allmergedcombinations.begin()+FRAMECONTEXT;
                
        for (vector<int>::iterator it=shortestpath.begin(); it!=shortestpath.end() && it2!=allmergedcombinations.end(); it++, it2++)
        {
          map<int, SimpleCombination>::const_iterator it3=it2->find(*it);
          if (it3==it2->end())
          {
            cerr << "INTERNAL ERROR 2!!!\n";
            exit(-1);
          }
          vbestcomb.push_back(it3->second);
        }
        
	// last frames (empty detection)
	for (int i=0; i<(int)(FRAMECONTEXT/2); i++) {
    	    SimpleCombination s;
       	    vbestcomb.push_back(s);
        }

	if (plotintensity) printloudnessmatrix(vbestcomb);

	// 3 - write best combinations

        for (vsimplecombinations::const_iterator it=vbestcomb.begin(); it!=vbestcomb.end(); it++)
            AddBestCombination(it->getPitches(),bestmelodynotes,nframes, nframes);

}	

/*-----------------------------------------------------------------------------
 
 Main function for frame by frame estimation (default option), used for MIREX08

------------------------------------------------------------------------------*/

void applyhypothesisfbf(peaksattvector &peaks, vectorchords &bestmelodynotes, bool eval)
{
        deque<mapa4> allcombinations;
        int nframes=0;

        // first frames (empty detection)
        if (!eval) 
        for (int i=0; i<(int)(FRAMECONTEXT/2); i++) {
       	    Combination empty;
       	    AddBestCombination(empty, bestmelodynotes, i, i);
        }
	for (iterpeaksattvector curr=peaks.begin(); curr!=peaks.end(); curr++)
	{
		if (printlog) {
		  cout << "TIME: " << curr->tini << endl;
		  cout << "PEAKS: " << endl;
		  for (itermapa2 it=curr->specpeaks.begin(); it!=curr->specpeaks.end(); it++)
			cout << "F= " << it->first << " A= " << it->second << endl;
                }
                
                // Get f0 candidates
		mapa2 f0candidates=GetF0Candidates(curr->specpeaks);		
		if (printlog) {
		  for (itermapa2 imp=f0candidates.begin(); imp!=f0candidates.end(); imp++)
		  cout << "Candidate: " << imp->first << " (" << freq2midi(imp->first) << ")" << endl;
                }
                else if (eval && nframes==STABLEFRAMES) // Only candidates in one frame are reported, for comparison with method I.
                {
                  for (itermapa2 imp=f0candidates.begin(); imp!=f0candidates.end(); imp++)
                     cout << freq2midi(imp->first) << " ";  
                  cout << endl;
                }
   
                // Get combinations of candidates
		vcombinations comb= GetAllCombinations(f0candidates);
		
		mapa4 cleaned;
		
		// Compute saliences of each combination
		
		if (!comb.empty()) { 
			ComputeCombinationsSalience(curr->specpeaks, f0candidates, comb);

                        // combinations within a frame are cleaned, removing duplicates (ej: 46-44, 46-44) and keeping the 
                        // one with highest salience
                        cleaned=CleanFrameCombinations(comb);
                }
	
                if (!eval) 
                {	
                  if (nframes>=FRAMECONTEXT) 
                  {
                    int tframe=curr->tini-FRAMECONTEXT/2;

                    vint bestcomb=MergeFramesCombinations(allcombinations);
                    AddBestCombination(bestcomb, bestmelodynotes, tframe, tframe);
                
                    allcombinations.pop_front();
                    allcombinations.push_back(cleaned);
                  }
                  else {
                    allcombinations.push_back(cleaned);
                  }
                }
                // For evaluation option 
                else {
                  allcombinations.push_back(cleaned);
                }
                nframes++;
	}
	// last frames (empty detection)
	if (!eval)
	for (int i=0; i<(int)(FRAMECONTEXT/2); i++) {
       	    Combination empty;
       	    AddBestCombination(empty, bestmelodynotes, nframes-(FRAMECONTEXT/2)+i, nframes-(FRAMECONTEXT/2)+i);
        }
	if (eval) {
	  vint bestcomb=MergeFramesCombinations(allcombinations);
          AddBestCombination(bestcomb, bestmelodynotes, 0, nframes);
	}
}


/*----------------------------------------------------------------------------

 Get prominent peaks vector from the current frame, adding it to the whole peaks vector

----------------------------------------------------------------------------*/

void getpeaksvector (int t_ini, int t_end, peaksvector &allpeaks, peaksattvector &peaks, peaksattvector &previouspeaks) // allpeaks by reference for efficency
{
    int t;

    mapa2 orderedspectrumpeaks;

    if (t_ini!=t_end) 
    {
    	if (t_ini+STABLEFRAMES <= t_end)
		t=t_ini+STABLEFRAMES;
    	else t=(int)floor((double)(t_ini+t_end)/2.0);
    }
    else t=t_ini;

    peaksatt currentpeaks(t_ini,t_end,allpeaks[t]);
    peaks.push_back(currentpeaks);
    if (t_ini!=0) {
      peaksatt prevpeaks(t_ini,t_end,allpeaks[t_ini-1]);
      previouspeaks.push_back(prevpeaks);
    }
}

/*----------------------------------------------------------------------------

 Print notes from bestmelodynotes vector (option -log)

-----------------------------------------------------------------------------*/

void printbestnotes(const vectorchords &bestmelodynotes) {
	
	int i=0;
	for (i=0; i<(int)bestmelodynotes.size(); i++)
	{
		cout << "t=" << bestmelodynotes[i].t_ini << "..." << bestmelodynotes[i].t_end << "=" << endl;
		for (int j=0; j<(int)bestmelodynotes[i].numnotes(); j++)
			cout << bestmelodynotes[i].notes[j].first << "-" << bestmelodynotes[i].notes[j].second << ",";	
		cout << endl;
	}
}

/*----------------------------------------------------------------------------

 Print detected pitches in the MIREX08 frame by frame format

-----------------------------------------------------------------------------*/

void printMirexFBF(const vectorchords &melodychords) 
{
        map<int,double> activenotes;	// Detected pitches
        map<int,double> tmpactivenotes;

        double eventres=info.time_resolution;
        double relative_time=0;
 
        for (int i=0; i<SHIFT; i++)  // correct slight mismatch due to window length
        {
          cout << i*0.01 << endl;
          relative_time+=eventres;
          
        }
        double lasttime=-1; 
        for (int i=0; i<(int)melodychords.size(); i++) 
        {
             double curr_time=round(relative_time/0.01)*0.01;
             if (curr_time != lasttime) {
               if (lasttime!=-1 && curr_time-lasttime>0.015)
               {
                 cout << curr_time-0.01 << "\t";
                 for (int j=0; j<melodychords[i].numnotes(); j++)
                   cout << midi2freq(melodychords[i].notes[j].first) << "\t";
                 cout << endl;
               }
               cout << curr_time << "\t";
 
               for (int j=0; j<melodychords[i].numnotes(); j++)
                 cout << midi2freq(melodychords[i].notes[j].first) << "\t";
               cout << endl;
             }
             lasttime=curr_time;
             relative_time+=eventres;
            
	}
}

/*----------------------------------------------------------------------------

 Print detected pitches in the MIREX08 note tracking format

-----------------------------------------------------------------------------*/

void printMirexNTrack(const vectorchords &melodychords) 
{
        map<int,double> activenotes;	// Detected pitches
        map<int,double> tmpactivenotes;

        double eventres=info.time_resolution;
        double relative_time=0;

        for (int i=0; i<(int)melodychords.size(); i++) 
        {
	  tmpactivenotes=activenotes;
  	  relative_time=(melodychords[i].t_ini)*eventres;

	  // Note offsets

	  for (map<int,double>::iterator curr=activenotes.begin(); curr!=activenotes.end();curr++) {

		int note=curr->first;

 		if (!(melodychords[i].isinchord(note)))
		{
		        cout << curr->second << "\t" << relative_time << "\t" << midi2freq(curr->first) << endl;
			tmpactivenotes.erase(note);
  		}
  	  }

	  // Note onsets

	  for (int j=0; j<(int)melodychords[i].numnotes(); j++) {

		int note=melodychords[i].notes[j].first;
		
		if (tmpactivenotes.count(note)==0) 
		{
			tmpactivenotes[note]=relative_time;
		}
	  }
	  activenotes=tmpactivenotes;
      }
}

/*----------------------------------------------------------------------------

 Print results for evaluation option (-eval)

-----------------------------------------------------------------------------*/

void printeval(const vectorchords &bestmelodynotes)
{
	for (int i=0; i<(int)bestmelodynotes[0].notes.size(); i++)
		cerr << bestmelodynotes[0].notes[i].first << " ";
        if (printlog) cout << "NUMNOTES=" << bestmelodynotes[0].numnotes() <<endl;
	cerr << endl;
}

/*-------------------------------------------------------------------------------

 Get peaks only after each onset, for -loadonsets and -onsetdetection options

-------------------------------------------------------------------------------*/

void processonsets(int t_end, double *onsetsvector, peaksvector &allpeaks, peaksattvector &peaks, peaksattvector &previouspeaks)  // allpeaks by reference for efficency
{	
	int lasttime=0;

	for (int i=0; i<t_end; i++)
	{
		if (onsetsvector[i]!=0) {
                        getpeaksvector(lasttime+1,i,allpeaks, peaks, previouspeaks);
			lasttime=i;		
		}
	} 
}

/*-------------------------------------------------------------------------------

 Function to load beats or onset data from a file where each column has a beat/onset time in secs.

-------------------------------------------------------------------------------*/

vector<double> LoadBeatsFile(string filename)
{
  ifstream f;
  double data;
  vector<double> beats;
  
  f.open(filename.c_str());
  if (f.is_open()) 
  {
    f >> data;
    while (!f.eof())
    {
      beats.push_back(data);
      f >> data;
    }
    f.close();
  }
  return beats;
}

/*-------------------------------------------------------------------------------

 Function to convert onset times

-------------------------------------------------------------------------------*/

void writeonsetsvector(vector<double> in, double out[], int n_frames, double timeres)
{
  for (int i=0; i<n_frames; i++)
    out[i]=0;
  
  for (vector<double>::iterator it=in.begin(); it!=in.end(); it++)
  {
      int pos=(int)round(*(it)/timeres);
      if (pos>=n_frames) {
        cerr << "ERROR: Onsets file and audio file lengths do not match";
        exit(-1);
      }
      else out[pos]=1.0;
  }
}

/*------------------------------------------------------------------------------

	MAIN

-------------------------------------------------------------------------------*/

int main(int argc, char *argv[]) 
{
   vectorchords bestmelodynotes, prevmelodynotes;
   peaksattvector peaks, previouspeaks;
   peaksvector allpeaks;
   vectorbands spectralbands;
   bool detectonsets, eval, beat, fileonsets, mergeIOI, fixsp, mirexfbf, plotintensity, mirexntrack, clean, printgraph, writetempo, fixsp3;
   string midifilename, beatfilename, onsetsfilename, graphfilename;
   vector<double> beatsvector; // for beat option
   vector<double> bronsetsvector;

   // Process parameters
   getParams(argc, argv, detectonsets, eval, midifilename, printlog, beat, beatfilename, fileonsets, onsetsfilename, mergeIOI, fixsp, mirexfbf, plotintensity, mirexntrack, clean, printgraph, graphfilename, writetempo, fixsp3);
   if (!eval) cerr << "Scanning input data..." << endl;

   // Output from input_data_wav: the peaks vector, information about spectral bands, the 1/12 bands spectrum and 
   // information about the spectrum parameters.
   Tmatrix outbandmatrix(1);
   int n_frames=input_data_wav(argv[1], allpeaks, spectralbands, outbandmatrix, info);

   // Preprocessing for -writetempo option (beats are loaded and tempo is estimated and written into the MIDI file) and beat options
   double tempochanges[n_frames];
   bool quantized[n_frames];
   if (writetempo || beat) {
            beatsvector=LoadBeatsFile(beatfilename);
            quantizebeats(beatsvector, quantized, (int)peaks.size());
            computeTempoChanges(beatsvector, tempochanges, (int)peaks.size());
    }

   // Evaluation mode to get results using a database of random pitches
   if (eval) 
   {
        // All frames are considered
        for (int i=0; i<n_frames; i++) {
          peaksatt currentpeaks(i,i+1,allpeaks[i]);
          peaks.push_back(currentpeaks);
        }
        applyhypothesisfbf(peaks,bestmelodynotes,true);
        printeval(bestmelodynotes);
   }

   // Onset detection mode. Onsets are detected, and only one frame after each detected onset is 
   // used to yield the pitches in the IOI
   else if (detectonsets)
   {
       double onsetsvector[n_frames];
       
       if (kPERCENTAGE!=50.0 || kN!=0) {
         cerr << "For onset detection, params.h must have kPERCENTAGE=50.0 and kN=0" << endl;
         exit(-1);
       }

       cerr << "Computing onsets (files onsets.orc and onsets.sco are generated)..." << endl;
       computeonsets(n_frames,onsetsvector,outbandmatrix,info.maxbandsval,info.time_resolution,argv[1]);
       if (printlog) printonsetsvector(onsetsvector, n_frames);
       cerr << "Get stable peaks..." << endl;
       processonsets(n_frames, onsetsvector, allpeaks,peaks,previouspeaks);
       cerr << "Applying algorithm..." << endl;
       applyhypothesisonsets(peaks, bestmelodynotes);
       applyhypothesisonsets(previouspeaks, prevmelodynotes);
   }
   
   // Load onsets mode. Each line of the input file corresponds with an onset time in seconds.
   // Then, only one frame after each onset is used to yield the pitches in the IOI.
   else if (fileonsets && !mergeIOI)
   {
        bronsetsvector=LoadBeatsFile(onsetsfilename);
        double onsetsvector[n_frames];

        writeonsetsvector(bronsetsvector,onsetsvector,n_frames,info.time_resolution);

        cerr << "Get stable peaks..." << endl;
        processonsets(n_frames, onsetsvector, allpeaks,peaks,previouspeaks);
      
        cerr << "Applying algorithm..." << endl;
        applyhypothesisonsets(peaks, bestmelodynotes);
        applyhypothesisonsets(previouspeaks, prevmelodynotes);

        detectonsets=true; // For writting MIDI file
   }

   // Frame by frame mode (default). 
   else
   {
         cerr << "Applying algorithm...." << endl;
 
        // All frames are analyzed
       	for (int i=0; i<n_frames; i++) {
                peaksatt currentpeaks(i,i,allpeaks[i]);
                peaks.push_back(currentpeaks);
        }

        // With -beat option, beat information is used to merge combination saliences for each 1/QUANTIZE beat.
        // Tempo is estimated from beats and it is written in the MIDI file. No need to specify writetempo param, it is set by default.
        if (beat) 
          applyhypothesisbeat(peaks,bestmelodynotes,quantized);

        // When mergeIOI is active, the combinations whithin each IOI are merged accross frames to compute their salience
        else if (mergeIOI)
        {
          bool quantizedonsets[peaks.size()];
      
          // The salience of the combinations whithin each IOI are merged 
          bronsetsvector=LoadBeatsFile(onsetsfilename);
          quantizeonsets(bronsetsvector, quantizedonsets, n_frames);

          // The function is the same as with beat option, but using the onset vector instead
          applyhypothesisbeat(peaks,bestmelodynotes,quantizedonsets);
        }

        // fixsp: MIREX08 with f0 tracking
        else if (fixsp)
           applyhypothesisfixsp(peaks, bestmelodynotes, plotintensity, printgraph, graphfilename.c_str());

        // fixsp3: Similar to fixsp, but the intensity of a candidate is predicted by linear interpolation of the two
        //         previous candidates, and the difference from the expected to the obtained values are summed.
        else if (fixsp3)
           applyhypothesisfixsp3(peaks, bestmelodynotes, plotintensity, printgraph, graphfilename.c_str());
        
        // Without beat information (beats file information can be used if -writetempo is active to write the tempo into the MIDI file)
        // This is the algorithm evaluated in MIREX08 without f0 tracking, with -clean option.
        else 
          applyhypothesisfbf(peaks,bestmelodynotes, false);
   }
   
   // -clean: postprocessing without tracking (remove short notes and glue pitches separated by short silences)
   if (clean) 
     cleannotes(bestmelodynotes);

   // Write MIDI file
   if (!eval)
   {
     cerr << "Creating midifile (" << midifilename << ")"<< endl;
     if (beat || writetempo)
     {
        double halfwindowtime=((double)info.N/(double)info.samplerate)/2.0; 
        writemiditempo(midifilename.c_str(), info.time_resolution, info.freq_resolution, bestmelodynotes, tempochanges, halfwindowtime);
     }   
     else writemidi(midifilename.c_str(), info.time_resolution, info.freq_resolution, bestmelodynotes, prevmelodynotes, detectonsets); // if onsets==false, prevmelodynotes is not used
   }
   
   // Write results in text files
   if (printlog) printbestnotes(bestmelodynotes);
   if (mirexfbf) printMirexFBF(bestmelodynotes);
   if (mirexntrack) printMirexNTrack(bestmelodynotes);
}

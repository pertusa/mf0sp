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

#include "spectralpattern.h"

spectralpattern::spectralpattern() {
	f0=0;
	midinote=0;
	loudness=0;
	sharpness=0;
	salience=0;
	numharmonics=0;
}

spectralpattern::spectralpattern(double freq, int midinote) {
	this->midinote=midinote;
	f0=freq;
	loudness=0;
	sharpness=0;
	salience=0;
	numharmonics=0;
}

spectralpattern& spectralpattern::operator=(const spectralpattern &sp) 
{
	for (int i=0; i<MAXHARMONICS; i++) {
		harmonics[i]=sp.harmonics[i];
		sharedharmonics[i]=sp.sharedharmonics[i];
		interpolate[i]=sp.interpolate[i];
	}

	f0=sp.f0;
	midinote=sp.midinote;
	loudness=sp.loudness;
	sharpness=sp.sharpness;
	salience=sp.salience;
	numharmonics=sp.numharmonics;
	
	return *this;
}

spectralpattern::spectralpattern(const spectralpattern &sp) {
	*this=sp;
}

double spectralpattern::ComputeCentroid()
{
	double num=0;
	double den=0;
	for (int i=0; i<numharmonics; i++)
	{
		num+=f0*(i-1)*harmonics[i];
		den+=harmonics[i];
	}
	centroid=(num/den)/f0;  // relative centroid
	return centroid;
}

double spectralpattern::ComputeSalience()
{
        double normalizedvalues[numharmonics];

        int shift=(int)floor(tamgaussian/2.0);
        sharpness=0;
	loudness=0;
        double gp=0;
        double maxhval=0;

        for (int i=0; i<numharmonics; i++) {
          if (harmonics[i]>maxhval)
            maxhval=harmonics[i];
          loudness+=harmonics[i];
        }
        
        for (int i=0; i<numharmonics; i++)
          normalizedvalues[i]=harmonics[i]/maxhval;
        
        for (int i=shift; i<numharmonics-1-shift; i++)
        {
          gp=0;
          for (int j=0; j<tamgaussian; j++) {
                gp+=normalizedvalues[i-shift+j]*gaussian[j];
          
          }
          sharpness+=fabs(gp-normalizedvalues[i]);
        }
        // sharpness must be relative to the number of points
        sharpness/=(double)numharmonics*(1.0-gaussian[shift+1]);
        
        // salience computation
        salience= pow(loudness*(pow(1.0-sharpness,kSMOOTHNESS)),2.0);

	return salience;
}
 
void spectralpattern::Interpolate()
{
   bool onlyonepoint=true;

   for (int i=1; i<numharmonics && onlyonepoint; i++)
       if (interpolate[i]==0)
           onlyonepoint=false;

   if (onlyonepoint) { // No interpolation possible
       for (int i=1; i<numharmonics; i++)
         if (interpolate[i]) 
           harmonics[i]=0;
   }
   else
   {
    for (int i=0; i<numharmonics; i++) {
     if (interpolate[i])
     {
       // Linear interpolation; search previous and next non shared harmonics
      int x1=i;
      int x2=i;
      double y1,y2=0;
     
      while (x1>0 && interpolate[x1])
        x1--;
      if (sharedharmonics[x1]!=NULL)
        y1=sharedharmonics[x1]->amp;
      else y1=0;

      while (x2<numharmonics && interpolate[x2])
        x2++;
      if (x2==numharmonics) {
        x2=x1-1;
        while (x2>0 && interpolate[x2])
          x2--;
        if (x2==x1) onlyonepoint=true;
      }
      
      double obtained=0;
      double interpolatedvalue=0;

      if (!onlyonepoint) {
         if (sharedharmonics[x2]!=NULL)
             y2=sharedharmonics[x2]->amp;
         else y2=0;
        
         if (sharedharmonics[i]!=NULL)
            obtained=sharedharmonics[i]->amp;
         else obtained=0;
         interpolatedvalue=((y2-y1)/((double)x2-(double)x1))*((double)i-(double)x1)+y1;
      }
      else interpolatedvalue=0;
      
      if (kLINEAR) {
	      if (interpolatedvalue > obtained) {
        	 harmonics[i]=obtained;
	         if (sharedharmonics[i]!=NULL)
        	   sharedharmonics[i]->amp=0;
	      }
	      else {
        	 harmonics[i]=interpolatedvalue;
	         if (sharedharmonics[i]!=NULL)
        	   sharedharmonics[i]->amp= obtained-interpolatedvalue;
	      }
      }
      else {
      	     if (interpolatedvalue > obtained) {
        	 harmonics[i]=obtained;
	         if (sharedharmonics[i]!=NULL)
        	   sharedharmonics[i]->amp=0;
	     }	
	     else {
	     	float A2=sqrt(pow(obtained,2.0)-pow(interpolatedvalue,2.0));
	     
	     	harmonics[i]=interpolatedvalue;
	     	if (sharedharmonics[i]!=NULL)
	     		sharedharmonics[i]->amp=A2;
	     }
      }
     }
    } 
  }
}

ostream &operator<<(ostream &output, const spectralpattern sp)
{
	output << "frec:" << sp.f0 << endl;
	output << "Number of harmonics= " << sp.numharmonics << endl;
	output << "Harmonics= ";
	for (int i=0; i< sp.numharmonics; i++) {
		output << sp.harmonics[i] << " ";
	}
	output << "Shared:" << endl;
	for (int i=0; i<sp.numharmonics; i++) {
	
		if (sp.sharedharmonics[i]!=NULL) {
		
			output << "sf[" << i << "]= " <<  sp.sharedharmonics[i]->freq << endl;
			output << "scand: "; 
			for (set<double,less<double> >::iterator it=(sp.sharedharmonics[i]->cand).begin(); it!=(sp.sharedharmonics[i]->cand).end(); it++)
				output << *it << " ";
			output << endl;
		}
	}
	return output;
}

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

#ifndef _COMBINATION_H_
#define _COMBINATION_H_

#include "defines.h"
#include "params.h"
#include "spectralpattern.h"
#include <sstream>
#include <string>

// inline functions for efficency
class Combination
{
  public:
  
    Combination() 		{ salience=0; }
    ~Combination()		{ fundamentals.clear(); pitches.clear(); salience=0; }
    Combination& operator=(const Combination &c) { fundamentals=c.fundamentals; spectralpatterns=c.spectralpatterns; salience=c.salience; pitches=c.pitches; return *this; }
    Combination(const Combination &c)  { *this=c; }

    void addF0(double f0) { fundamentals.push_back(f0); }
    void addPitch(int pitch) { pitches.push_back(pitch); }
    double getF0(int i) const	{ return fundamentals[i]; }
    int getPitch(int i) const	{ return pitches[i]; }
    int size() const		{ return (int)(fundamentals.size()); }
    void addsp(const vector<spectralpattern> &sp) { spectralpatterns=sp; }
    void reserve(int tam) { spectralpatterns.reserve(tam); }
    
    friend ostream& operator<<(ostream &o, Combination &c);

    vdouble fundamentals; // f0 candidates
    vint pitches;	// Midi pitch
    vector<spectralpattern> spectralpatterns;
    double salience;
};

class SimpleCombination 
{
  public:
    SimpleCombination() { salience=0; }
    SimpleCombination(const Combination &c)
    {
      pitches=c.pitches;
      salience=c.salience;
      id=0;
      loudness.reserve(c.size());
      for (int i=0; i<c.size(); i++)
        loudness.push_back(c.spectralpatterns[i].loudness);
    }
    SimpleCombination& operator=(const SimpleCombination &c) { 
      pitches.reserve(c.pitches.size());
      loudness.reserve(c.loudness.size());
      pitches=c.pitches;
      loudness=c.loudness;
      id=c.id;
      salience=c.salience; 
      return *this; 
    }
    SimpleCombination(const SimpleCombination &c) { *this=c; }

    SimpleCombination(const vector<int> &pitches, const vector<double> &loudness)
    {
      this->pitches=pitches;
      this->loudness=loudness;
      salience=0;
      id=0;
      maxloudness=0;
    }

    
    const vector<int>& getPitches() const { return pitches; }
    const vector<double>& getLoudness() const { return loudness; }
    double getSalience() const { return salience; }
    void setSalience(double s) { salience=s; }
    const int& getPitch(int i) const { return pitches[i]; }
    double getLoudness(int i) const { return loudness[i]; }
    void setLoudness(int index, double l) { loudness[index]=l; }
    int getSize() const { return pitches.size(); }
    double energy() const { 
      double l=0;
      for (vector<double>::const_iterator it=loudness.begin(); it!=loudness.end(); it++)
          l+=*it;        
      return l;
    }

    bool operator<(const SimpleCombination &c) const
    { return (this->getSalience() < c.getSalience()); }
    
    bool operator==(const SimpleCombination &c) const
    {
      return (pitches==c.pitches);
    }
    string getname() const
    {
      string name="";
      
      if ((int)pitches.size()==0) name="0";
      else for (vector<int>::const_iterator it=pitches.begin(); it!=pitches.end(); it++)
      {
          if (it!=pitches.begin())
            name+="n";
          int pitch=*it;
          std::ostringstream s;
          s<<pitch;      
          name+=s.str();
      }
      
      return name;
    }
    const int& getid() const {
      return id;
    }
    void setid(const int ident)
    {
      this->id=ident;
    }
    friend ostream& operator<<(ostream &o, const SimpleCombination &c);

  private:
    vector<int> pitches;
    vector<double> loudness;
    double maxloudness;
    double salience;
    int id;
};



#define vcombinations vector<Combination>
#define vsimplecombinations vector<SimpleCombination>
#define itervsimplecombinations vector<SimpleCombination>::iterator

/*
class Combinationsatt
{
    public:

      vcombinations vc;
      int tini;
      int tend;
    
      Combinationsatt(vcombinations comb, int tini, int tend) 
      { 
        vc=comb;
        this->tini=tini;
        this->tend=tend;
      }
      ~Combinationsatt() {
        vc.clear();
        tini=0;
        tend=0;
      }
      Combinationsatt(const Combinationsatt &c)
      {
        vc=c.vc;
        tini=c.tini;
        tend=c.tend;
      }
};

#define vcombinationsatt vector<Combinationsatt>
*/
#define vvcombinations vector<vector <Combination> >

#define setcombinations set<Combination>
vcombinations GetAllCombinations(mapa2 candidates);
void getCombination(vector<double> n, int n_column, double r[], int r_size, int r_column, int loop, vcombinations &comb);

#define mapa4 map< vector<int> , double >
#define itermapa4 mapa4::iterator

#endif

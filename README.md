# Multiple fundamental frequency estimation system

This is method described in:

```
A. Pertusa and J.M. Iñesta, "Efficient methods for joint estimation of
multiple fundamental frequencies in music signals", EURASIP
Journal on Advances in Signal Processing, vol. 2012, pp. 27 (2012)
```

It takes an input WAV file and outputs a MIDI file. The input file must be WAV,
MONO, and with 22kHz or 44kHz sampling rate

## Compilation

To compile the source code in Linux or MacOS, type from the command line:

```
make
```

The software requires libsndfile and fftw3 libraries. Both
are standard in Linux distributions and they can be obtained from the
corresponding package installer. In MacOS, they can be downloaded from:

http://www.mega-nerd.com/libsndfile/
http://www.fftw.org/

Boost library is also required. It is standard in Linux distributions, and
in MacOSX it can be downloaded from:

http://www.boost.org/

Boost needs to be extracted, but it is not necessary to compile it.
If it was downloaded from boost webpage, then change the path in the Makefile. If
it was installed via a package installer, the Makefile does not need to be
changed.

## Usage

The basic syntax once compiled is:

```
./alg input.wav -o output.mid
```

Some parameters can be controlled from command line.
To view the syntax, please type:

```
./alg --help
```

These are the calls that were used to evaluate the methods in the results
section:

### Extended method without tracking:
```
	alg input.wav -o output.mid -clean
```
### Extended method with tracking:    
```
	alg input.wav -o output.mid -fixsp
```
### Extended method merging information within inter-onset frames.
In this case, onset times must be previously extracted and stored into
onsetsfile.txt:
```
	alg input.wav -loadonsets onsetsfile.txt -mergeIOI -clean -o output.mid
```

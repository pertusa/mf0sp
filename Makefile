####################
# Requires libsnd, fftw and boost
# Change BOOSTPATH if necessary (only if it was downloaded from boost webpage, not via package-installer)
####################
BOOSTPATH=~/boost_path/boost_1_42_0
####################

TARGET=./bin/alg

OBJS= \
objs/myfft.o \
objs/bands.o \
objs/processwav.o \
objs/ocsound.o \
objs/onsets.o \
objs/midi.o \
objs/writemidi.o \
objs/spectralpattern.o \
objs/peaksatt.o \
objs/combination.o \
objs/alg.o

CC=g++
FLAGS_COMPILER= -c -O3 -Wall -Wno-deprecated -I $(BOOSTPATH)
FLAGS_LINKER= -lm -lsndfile -lfftw3

all: $(OBJS) 
	@echo 'Linking: $@'
	$(CC) $(OBJS) $(FLAGS_LINKER) -o $(TARGET)


$(OBJS): ./objs/%.o : ./src/%.cpp
	@echo 'Compiling $< ...'
	$(CC) $(FLAGS_COMPILER) -o "./$@" "./$<"

clean:
	@echo 'Cleaning '
	rm -f ./objs/*
	@echo 'Cleaning '
	rm -f $(TARGET)

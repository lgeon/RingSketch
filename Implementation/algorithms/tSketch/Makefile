CFLAGS= -std=c++17 -O2
SSEFLAGS = -msse2 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2
CXX ?= g++
CC ?= gcc
OBJS = PreCount.o HashPart.o SketchPart.o TSketch.o

all: $(OBJS) objmove

PreCount.o: preCount.cpp preCount.h
	$(CXX) $(CFLAGS) -c preCount.cpp

HashPart.o: hashPart.cpp hashPart.h
	$(CXX) $(CFLAGS) -c hashPart.cpp

SketchPart.o: sketchPart.cpp sketchPart.h
	$(CXX) $(CFLAGS) -c sketchPart.cpp $(SSEFLAGS)

TSketch.o: tSketch.cpp tSketch.h
	$(CXX) $(CFLAGS) -c tSketch.cpp

objmove:
	cp $(OBJS) ../../	

clean:
	rm -rf $(OBJS)
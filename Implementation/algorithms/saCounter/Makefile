CFLAGS= -std=c++17 -O2
CXX ?= g++
CC ?= gcc
OBJS = saCounter.o

all: $(OBJS) objmove

saCounter.o: saCounter.cpp saCounter.h
	$(CXX) $(CFLAGS) -g -c saCounter.cpp
	
objmove:
	cp $(OBJS) ../../	

clean:
	rm -rf $(OBJS)
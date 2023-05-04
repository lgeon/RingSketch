CFLAGS= -std=c++17 -O2
CXX ?= g++
CC ?= gcc
OBJS = slidingWindow.o

all: $(OBJS) objmove

cmsketch.o: slidingWindow.cpp slidingWindow.h
	$(CXX) $(CFLAGS) -c slidingWindow.cpp
	
objmove:
	cp $(OBJS) ../../	

clean:
	rm -rf $(OBJS)
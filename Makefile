CPP      = g++
CC       = gcc
OBJ      = main.o s3mContainer.o ALSAPlayer.o
LINKOBJ  = main.o s3mContainer.o ALSAPlayer.o
LIBS     = -lasound
CXXFLAGS = -std=c++14 -Wall
BIN	 = playd

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

main.o: main.cpp
	$(CPP) -c main.cpp -o main.o $(CXXFLAGS)

s3mContainer.o: s3mContainer.cpp
	$(CPP) -c s3mContainer.cpp -o s3mContainer.o $(CXXFLAGS)

ALSAPlayer.o: ALSAPlayer.cpp
	$(CPP) -c ALSAPlayer.cpp -o ALSAPlayer.o $(CXXFLAGS)

.PHONY : clean
clean : 
	-rm *.o $(objects) playd


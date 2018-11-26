CPP      = g++
CC       = gcc
OBJ      = main.o s3mContainer.o ALSAPlayer.o
LIBS     = -lasound -lpthread
CXXFLAGS = -c -std=c++14 -Wall -g
BIN	 = playd

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN) $(LIBS)

./%.o: ./%.cpp
	$(CPP) $(CXXFLAGS) $< -o $@

.PHONY : clean
clean : 
	-rm *.o playd



#compilers
CXX		= g++
CXXFLAGS	= -std=c++17 -fopenmp -ggdb -Wall

COMMON=  mainPBFS.cpp bfsSpMVM.cpp

ALL= mainPBFS

all: $(ALL)

% : %.C $(COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $^ 

.PHONY : clean

clean :
	rm -f *.o $(ALL)

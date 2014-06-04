CXX=g++
CXXFLAGS=-Wall -g -std=c++11 -mwindows -static
NAME=EengineLauncher.exe

CXXSRC=md5.cpp\
	utils.cpp\
	main.cpp

CXXOBJ=$(CXXSRC:.cpp=.o)

$(NAME): $(CXXOBJ)
	windres ee.rc -O coff -o ee.res
	$(CXX) $(CXXFLAGS) $? ee.res -o $@
	
$(CXXOBJ): $(CXXSRC)
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $@
	
clean:
	rm -f *.o
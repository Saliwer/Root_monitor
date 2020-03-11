ALL_C    = $(wildcard *.cpp)
ALL_O    = $(ALL_C:.cpp=.o)

ROOTCFLAGS	= `root-config --cflags`
ROOTLIBS	= `root-config --libs`
ROOTGLIBS	= `root-config --glibs`

CXX		= @g++
LD		= g++

DEBUGFLAG	= -O2
STDLIBS		= -lpthread $(ROOTLIBS) $(ROOTGLIBS) -lzmq -lreadline

INCFLAGS	= -I. -I/home/karakhan/work/ZmqDialog/include
DEFFLAGS	= 

CXXFLAGS	= $(DEBUGFLAG) -Wall -O -fexceptions $(INCFLAGS) -Wno-deprecated $(ROOTCFLAGS)

LIBS		= -L/home/karakhan/work/ZmqDialog/lib -lZmqDlg


default:	publisher root_monitor

show:		
		@echo $(ALL_O)

depend.mk:
		@echo $(WD) "Create dependencies file"
		$(CXX) $(CXXFLAGS) *.cc  -MM > depend.mk

./%.o:          ./%.cxx depend.mk
		@echo "Compilation: "$<
		$(CXX) $(CXXFLAGS) $(DEFFLAGS) -c $< -o $@ -std=c++11
		@echo "$@ done..."

clean:
		@echo $(WD) "Cleaning ..."
		@rm -f ./*.o  ./depend.mk ./*~ ./root_monitor ./publisher ./Monitor_decodeDict.cxx ./Monitor_decodeDict_rdict.pcm

-include depend.mk

root_monitor:	Monitor_decodeDict.o GlobalFuncs.h Monitor_decodeLinkDef.h $(ALL_O) main.cc
		$(CXX) -o $@ $(CXXFLAGS) $(DEFFLAGS) \
		Monitor_decodeDict.o $(ALL_O) $(STDLIBS) $(LIBS) main.cc
		@echo "$@ done..."

publisher:	DlgPublisher.cc
		$(CXX) -o $@ $(CXXFLAGS) $(DEFFLAGS) $(LIBS) $(STDLIBS) DlgPublisher.cc 

Monitor_decodeDict.cxx:	GlobalFuncs.h Monitor_decodeLinkDef.h 
		rootcint -f $@ -c $^
		@echo "$@ done..."


Monitor_decodeDict.o:	Monitor_decodeDict.cxx
		$(CXX) $(DEBUGFLAG) -o $@ $(ROOTCFLAGS) -c $^
		@echo "$@ done..."

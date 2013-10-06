PIN_HOME ?= ../../..
PIN_KIT=$(PIN_HOME)

include $(PIN_HOME)/source/tools/makefile.gnu.config

# -Wno-unused-function

LINKER?=${CXX}
CXXFLAGS ?= -Wall -Werror -Wno-unknown-pragmas $(DBG) $(OPT)

CXX=g++

all: $(OBJDIR) mempin

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)mempin_inscount.o: mempin.h mempin_inscount.h mempin_inscount.cpp
	$(CXX) -g -c $(CXXFLAGS) $(PIN_CXXFLAGS) mempin_inscount.cpp -o $(OBJDIR)mempin_inscount.o

$(OBJDIR)mempin_proccount.o: mempin.h mempin_proccount.h mempin_proccount.cpp
	$(CXX) -g -c $(CXXFLAGS) $(PIN_CXXFLAGS) mempin_proccount.cpp -o $(OBJDIR)mempin_proccount.o

$(OBJDIR)mempin_malloctrace.o: mempin.h mempin_malloctrace.h mempin_malloctrace.cpp
	$(CXX) -g -c $(CXXFLAGS) $(PIN_CXXFLAGS) mempin_malloctrace.cpp -o $(OBJDIR)mempin_malloctrace.o

$(OBJDIR)mempin.o: mempin.h mempin.cpp mempin_tools.h mempin_utils.h
	$(CXX) -g -c $(CXXFLAGS) $(PIN_CXXFLAGS) mempin.cpp -o $(OBJDIR)mempin.o

mempin: $(OBJDIR)mempin.o $(OBJDIR)mempin_inscount.o $(OBJDIR)mempin_proccount.o $(OBJDIR)mempin_malloctrace.o
	$(CXX) -g $(PIN_LDFLAGS) $(LINK_DEBUG) $(OBJDIR)mempin.o $(OBJDIR)mempin_inscount.o $(OBJDIR)mempin_proccount.o $(OBJDIR)mempin_malloctrace.o -o $(OBJDIR)mempin.so $(PIN_LPATHS) $(PIN_LIBS) $(DBG)


clean:
	rm -f $(OBJDIR)*

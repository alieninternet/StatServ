
CXX=c++

CXXFLAGS=-O3 -Wall

TARGET=versionserv

SOURCES=\
	daemon.cpp \
	main.cpp \
	parser.cpp \
	sender.cpp \
	str.cpp

OBJECTS=$(SOURCES:.cpp=.o)

########################################################################

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFS) $(INCDIRS) -c $<
	
clean:
	$(RM) *~ *.o $(TARGET)


CXX=c++

CXXFLAGS=-Wall -Wstrict-prototypes #-O3

TARGET=statserv

SNMP_LIBS=\
	-L/usr/local/lib -lucdagent -lucdmibs -lsnmp \
	-L/usr/lib -ldl \
	-L/usr/local/ssl/lib -lcrypto

SOURCES=\
	daemon.cpp \
	main.cpp \
	parser.cpp \
	sender.cpp \
	str.cpp #\
#	snmp.cpp

OBJECTS=$(SOURCES:.cpp=.o)

########################################################################

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) #$(SNMP_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFS) $(INCDIRS) -c $<
	
clean:
	$(RM) *~ *.o $(TARGET)

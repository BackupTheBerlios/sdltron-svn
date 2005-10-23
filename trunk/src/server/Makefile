TOP=../..
include $(TOP)/config.mak

all: tronserver

tronserver: server.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
tronserver: ../protocol/libtronnet.a

server.o: server.cc

clean:
	rm -f tronserver *.o

new: clean all

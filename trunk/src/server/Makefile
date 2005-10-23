CPPFLAGS=$(shell sdl-config --cflags) \
		 -I../../../SDL_net++/src \
		 -I../../../pack++
CXXFLAGS=-Wall -g -ansi -pedantic
LDFLAGS=\
		-L../protocol -ltronnet \
		-L../../../pack++ -lpack++ \
		-L../../../SDL_net++/src -lSDL_net++ \
		$(shell sdl-config --libs) \
		-lSDL_net

all: tronserver

tronserver: server.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
tronserver: ../protocol/libtronnet.a

server.o: server.cc

clean:
	rm -f tronserver *.o

new: clean all

CPPFLAGS=$(shell sdl-config --cflags) \
		 -I../../../SDL_net++/src \
		 -I../../../pack++ \
		 -I../../../hexstream
CXXFLAGS=-Wall -g -ansi -pedantic

all: libtronnet.a

libtronnet.a: tron.o tronnet.o client.o ../../../hexstream/hexstream.o
	ar crv $@ $?

tron.cc tron.h: tron.pack ../../../pack++/genpack
	../../../pack++/genpack tron.pack

tron.o: tron.cc
tron.o: tron.h ../../../pack++/pack++.h

clean:
	rm -f libtronnet.a *.o

new: clean all

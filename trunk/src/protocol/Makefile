TOP=../..
include $(TOP)/config.mak

all: libtronnet.a

libtronnet.a: tron.o tronnet.o client.o $(HEXSTREAM)/hexstream.o
	$(AR) crv $@ $?
	$(RANLIB) $@

#tron.cc tron.h: tron.pack $(SDLPACKPP)/genpack
#	$(SDLPACKPP)/genpack tron.pack

tron.cc tron.h: tron.pack

tron.o: tron.cc
tron.o: tron.h $(SDLPACKPP)/pack++.h

clean:
	rm -f libtronnet.a *.o tron.cc tron.h tron.html

new: clean all

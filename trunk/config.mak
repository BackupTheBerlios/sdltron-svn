# Generally this is a relative dir set by subdirs' Makefiles
TOP?=.

# Set these to the proper locations!
SDLNETPP=$(TOP)/../sdlnetpp
SDLPACKPP=$(TOP)/../sdlpackpp

################################################################################
# Don't edit below this!

HEXSTREAM=$(SDLPACKPP)/hexstream

CPPFLAGS=$(shell sdl-config --cflags) \
		 -I$(SDLNETPP)/src \
		 -I$(SDLPACKPP) \
		 -I$(HEXSTREAM)

CXXFLAGS=-Wall -g -ansi -pedantic

LDFLAGS=\
		-L../protocol -ltronnet \
		-L$(SDLPACKPP) -lpack++ \
		-L$(SDLNETPP)/src -lSDL_net++ \
		$(shell sdl-config --libs) \
		-lSDL_net

RANLIB?=ranlib

all:

$(SDLPACKPP)/genpack:
	make -C $(SDLPACKPP) genpack

%.cc %.h: %.pack $(SDLPACKPP)/genpack
	$(SDLPACKPP)/genpack tron.pack

# Copyright 2005 Robin H. Johnson <robbat2@gentoo.org>
# Distributed under the terms of the GNU General Public License v2
# $Header: /code/convert/cvsroot/infrastructure/readahead-list/Attic/Makefile,v 1.7 2005/03/22 07:08:21 robbat2 Exp $

PN = readahead-list
PV = 0.20050320.2320
P = $(PN)-$(PV)
SORTER = file-order-block
BIN = $(PN)   $(SORTER)
SRC = $(PN).c $(SORTER).cxx
OBJ = $(PN).o 

#CFLAGS = -ggdb3 -Wall -W -save-temps
CFLAGS = -O3 -Wall -W -save-temps -funroll-loops
CXXFLAGS += $(CFLAGS)

all: $(BIN)


$(SORTER): $(SORTER).cxx
	g++ $(CXXFLAGS) -I/usr/src/linux/include -o $@ $<

$(PN): $(PN).o

clean:
	rm -f $(OBJ) $(BIN) *.o $(SORTER) core *.s *.i *.ii

D=/tmp/$(P)
F=/tmp/$(P).tar.bz2
dist: clean
	rm -rf $(D)
	cp -ra $(PWD) $(D)
	tar cvjf $(F) -C /tmp --exclude=CVS $(P)
	@echo "Build distfile as $(F)"

# vim: ts=4 sw=4:

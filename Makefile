# Copyright 2005 Robin H. Johnson <robbat2@gentoo.org>
# Distributed under the terms of the GNU General Public License v2
# $Header: /code/convert/cvsroot/infrastructure/readahead-list/Attic/Makefile,v 1.10 2005/03/23 03:41:31 robbat2 Exp $

PN = readahead-list
PV = 0.20050322.1945
P = $(PN)-$(PV)
SORTER = filelist-order
BIN = $(PN)   $(SORTER)
SRC = $(PN).c $(SORTER).cxx
OBJ = $(PN).o 

CFLAGS = -ggdb3
CFLAGS += -Wall -W -Wextra
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

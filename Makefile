# Copyright 2005 Robin H. Johnson <robbat2@gentoo.org>
# Distributed under the terms of the GNU General Public License v2
# $Header: /code/convert/cvsroot/infrastructure/readahead-list/Attic/Makefile,v 1.3 2005/03/21 07:12:51 robbat2 Exp $

PN = readahead-list
PV = 0.20050320.2311
P = $(PN)-$(PV)
BIN = $(PN)
SRC = $(BIN).c
OBJ = $(SRC:%.c=%.o)

$(BIN): $(OBJ)

clean:
	rm -f $(OBJ) $(BIN)

D=/tmp/$(P)
F=/tmp/$(P).tar.bz2
dist:
	rm -rf $(D)
	cp -ra $(PWD) $(D)
	tar cvjf $(F) -C /tmp --exclude=CVS $(P)
	@echo "Build distfile as $(F)"

# vim: ts=4 sw=4:

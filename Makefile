# Copyright 2005 Robin H. Johnson <robbat2@gentoo.org>
# Distributed under the terms of the GNU General Public License v2
# $Header: /code/convert/cvsroot/infrastructure/readahead-list/Attic/Makefile,v 1.2 2005/03/21 04:54:10 robbat2 Exp $

BIN = readahead-list
SRC = $(BIN).c
OBJ = $(SRC:%.c=%.o)

$(BIN): $(OBJ)

clean:
	rm -f $(OBJ) $(BIN)

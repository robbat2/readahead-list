BIN = readahead-list
SRC = $(BIN).c
OBJ = $(SRC:%.c=%.o)

$(BIN): $(OBJ)

clean:
	rm -f $(OBJ) $(BIN)

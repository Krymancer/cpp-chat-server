IPATH=-Iinc/
SRC=src/
OBJ=obj/
BIN=bin/
ARGS=-pthread

all:server

server:
	@g++ $(IPATH) $(ARGS) $(SRC)server.cpp -o $(BIN)server

clean:
	rm -rf $(OBJ)*.o
	rm -rf $(BIN)*
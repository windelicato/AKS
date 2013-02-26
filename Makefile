CC = gcc -I ./include
CFLAGS =  -g

INC = ./include
SRC = ./src
OBJ = ./obj
BIN = ./bin

vpath %.h ./include
vpath %.c ./src

EXECS = bin_client olp_client

all: $(EXECS)


$(OBJ)/comm.o: $(INC)/comm.h $(SRC)/comm.c
	$(CC) $(CFLAGS) -c $(SRC)/comm.c -o $(OBJ)/comm.o

$(OBJ)/serial.o: $(INC)/serial.h $(SRC)/serial.c
	$(CC) $(CFLAGS) -c $(SRC)/serial.c -o $(OBJ)/serial.o

bin_client: $(OBJ)/comm.o $(OBJ)/serial.o
	$(CC) $(CFLAGS) $(SRC)/bin_client.c $(OBJ)/serial.o $(OBJ)/comm.o -o $(BIN)/bin_client

olp_client: $(OBJ)/comm.o 
	$(CC) $(CFLAGS) $(SRC)/olp_client.c $(OBJ)/comm.o -o $(BIN)/olp_client

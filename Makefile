CC = gcc -I ./include
CFLAGS =  -g -O2
MYSQLFLAGS = `mysql_config --cflags --libs`

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
	$(CC) $(CFLAGS) -c $(SRC)/serial.c -o $(OBJ)/serial.o -lpthread

$(OBJ)/scan.o: $(INC)/scan.h $(SRC)/scan.c
	$(CC) $(CFLAGS) -c  $(SRC)/scan.c -o $(OBJ)/scan.o $(MYSQLFLAGS)
 
bin_client: $(OBJ)/comm.o $(OBJ)/serial.o
	$(CC) $(CFLAGS) $(SRC)/bin_client.c  $(OBJ)/serial.o $(OBJ)/comm.o -o $(BIN)/bin_client -lpthread

olp_client: $(OBJ)/comm.o ${OBJ}/scan.o
	$(CC) $(CFLAGS) $(SRC)/olp_client.c $(OBJ)/scan.o $(OBJ)/comm.o -o $(BIN)/olp_client $(MYSQLFLAGS)


clean:
	rm ./bin/*
	rm ./obj/*

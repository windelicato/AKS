CC = gcc -I ./include
CFLAGS =  -g

INC = ./include
SRC = ./src
OBJ = ./obj
BIN = ./bin
TEST = ./tests

vpath %.h ./include
vpath %.c ./src

EXECS = bin_client olp_client

all: $(EXECS)


$(OBJ)/comm.o: $(INC)/comm.h $(SRC)/comm.c
	$(CC) $(CFLAGS) -c $(SRC)/comm.c -o $(OBJ)/comm.o

$(OBJ)/serial.o: $(INC)/serial.h $(SRC)/serial.c
	$(CC) $(CFLAGS) -c $(SRC)/serial.c -o $(OBJ)/serial.o -lpthread

$(OBJ)/i2c_functions.o: $(INC)/i2c_functions.h $(SRC)/i2c_functions.c 
	$(CC) $(CFLAGS)  -c $(SRC)/i2c_functions.c -o $(OBJ)/i2c_functions.o 

bin_client: $(OBJ)/comm.o $(OBJ)/serial.o
	$(CC) $(CFLAGS) $(SRC)/bin_client.c $(OBJ)/serial.o $(OBJ)/comm.o -o $(BIN)/bin_client -lpthread

olp_client: $(OBJ)/comm.o 
	$(CC) $(CFLAGS) $(SRC)/olp_client.c $(OBJ)/comm.o -o $(BIN)/olp_client

i2c_test: $(OBJ)/i2c_functions.o 
	$(CC) $(CFLAGS) $(TEST)/i2c_test.c $(OBJ)/i2c_functions.o -o $(BIN)/i2c_test -I/usr/local/include -L/usr/local/lib -lwiringPi

clean:
	rm ./bin/*
	rm ./obj/*

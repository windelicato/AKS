CC = gcc -I ./include
CFLAGS =  -g -O2
MYSQLFLAGS = `mysql_config --cflags --libs`

INC = ./include
SRC = ./src
OBJ = ./obj
BIN = ./bin
TEST = ./tests

vpath %.h ./include
vpath %.c ./src

EXECS = bin_client 

all: $(EXECS)

$(OBJ)/AKS_errors.o: $(INC)/AKS_errors.h $(SRC)/AKS_errors.c
	$(CC) $(CFLAGS) -c $(SRC)/AKS_errors.c -o $(OBJ)/AKS_errors.o

$(OBJ)/i2c_functions.o: $(INC)/i2c_functions.h $(SRC)/i2c_functions.c 
	$(CC) $(CFLAGS)  -c $(SRC)/i2c_functions.c -o $(OBJ)/i2c_functions.o 

$(OBJ)/lightbar_functions.o: $(INC)/lightbar_functions.h $(SRC)/lightbar_functions.c 
	$(CC) $(CFLAGS)  -c  $(SRC)/lightbar_functions.c -o $(OBJ)/lightbar_functions.o 

$(OBJ)/scan.o: $(INC)/scan.h $(SRC)/scan.c
	$(CC) $(CFLAGS) -c  $(SRC)/scan.c -o $(OBJ)/scan.o $(MYSQLFLAGS)

$(OBJ)/comm.o: $(INC)/comm.h $(SRC)/comm.c
	$(CC) $(CFLAGS) -c $(SRC)/comm.c -o $(OBJ)/comm.o

$(OBJ)/serial.o: $(INC)/serial.h $(SRC)/serial.c
	$(CC) $(CFLAGS) -c $(SRC)/serial.c -o $(OBJ)/serial.o -lpthread

 
bin_client: $(OBJ)/AKS_errors.o $(OBJ)/scan.o $(OBJ)/comm.o  $(OBJ)/serial.o $(OBJ)/lightbar_functions.o $(OBJ)/i2c_functions.o 
	$(CC) $(CFLAGS) $(SRC)/bin_client.c $(OBJ)/scan.o $(OBJ)/serial.o $(OBJ)/AKS_errors.o $(OBJ)/comm.o $(OBJ)/i2c_functions.o $(OBJ)/lightbar_functions.o -o $(BIN)/bin_client $(MYSQLFLAGS) -lpthread -lm

olp_client: $(OBJ)/comm.o $(OBJ)/scan.o
	$(CC) $(CFLAGS) $(SRC)/olp_client.c $(OBJ)/scan.o $(OBJ)/comm.o -o $(BIN)/olp_client $(MYSQLFLAGS)

olp_recv_test: $(OBJ)/comm.o 
	$(CC) $(CFLAGS) $(TEST)/olp_recv_test.c $(OBJ)/comm.o -o $(TEST)/olp_recv_test

i2c_test: $(OBJ)/i2c_functions.o 
	$(CC) $(CFLAGS) $(TEST)/i2c_test.c $(OBJ)/i2c_functions.o -o $(BIN)/i2c_test -I/usr/local/include -L/usr/local/lib -lwiringPi

clean:
	rm ./bin/*
	rm ./obj/*

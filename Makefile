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

$(OBJ)/led_functions.o: $(INC)/led_functions.h $(SRC)/led_functions.c
	$(CC) $(CFLAGS) -c $(SRC)/led_functions.c -o $(OBJ)/led_functions.o

$(OBJ)/button_functions.o: $(INC)/button_functions.h $(SRC)/button_functions.c
	$(CC) $(CFLAGS) -c $(SRC)/button_functions.c -o $(OBJ)/button_functions.o

$(OBJ)/i2c_devices.o: $(INC)/i2c_devices.h $(SRC)/i2c_devices.c 
	$(CC) $(CFLAGS) -c $(SRC)/i2c_devices.c -o $(OBJ)/i2c_devices.o

$(OBJ)/scan.o: $(INC)/scan.h $(SRC)/scan.c
	$(CC) $(CFLAGS) -c  $(SRC)/scan.c -o $(OBJ)/scan.o $(MYSQLFLAGS)

$(OBJ)/comm.o: $(INC)/comm.h $(SRC)/comm.c
	$(CC) $(CFLAGS) -c $(SRC)/comm.c -o $(OBJ)/comm.o

$(OBJ)/serial.o: $(INC)/serial.h $(SRC)/serial.c
	$(CC) $(CFLAGS) -c $(SRC)/serial.c -o $(OBJ)/serial.o -lpthread

$(OBJ)/configuration.o: $(INC)/configuration.h $(SRC)/configuration.c
	$(CC) $(CFLAGS) -c $(SRC)/configuration.c -o $(OBJ)/configuration.o 

 
bin_client: $(OBJ)/configuration.o $(OBJ)/AKS_errors.o $(OBJ)/scan.o $(OBJ)/comm.o  $(OBJ)/serial.o $(OBJ)/lightbar_functions.o $(OBJ)/i2c_functions.o $(OBJ)/led_functions.o $(OBJ)/button_functions.o $(OBJ)/i2c_devices.o
	$(CC) $(CFLAGS) $(SRC)/bin_client.c $(OBJ)/scan.o $(OBJ)/serial.o $(OBJ)/configuration.o $(OBJ)/AKS_errors.o $(OBJ)/comm.o $(OBJ)/i2c_functions.o $(OBJ)/lightbar_functions.o $(OBJ)/led_functions.o $(OBJ)/button_functions.o $(OBJ)/i2c_devices.o -o $(BIN)/bin_client $(MYSQLFLAGS) -lpthread -lm

olp_client: $(OBJ)/comm.o $(OBJ)/scan.o
	$(CC) $(CFLAGS) $(SRC)/olp_client.c $(OBJ)/scan.o $(OBJ)/comm.o -o $(BIN)/olp_client $(MYSQLFLAGS)

olp_recv_test: $(OBJ)/comm.o 
	$(CC) $(CFLAGS) $(TEST)/olp_recv_test.c $(OBJ)/comm.o -o $(TEST)/olp_recv_test

i2c_test: $(OBJ)/i2c_functions.o 
	$(CC) $(CFLAGS) $(TEST)/i2c_test.c $(OBJ)/i2c_functions.o -o $(BIN)/i2c_test -I/usr/local/include -L/usr/local/lib -lwiringPi

log_test: $(OBJ)/configuration.o $(OBJ)/AKS_errors.o $(OBJ)/scan.o $(OBJ)/comm.o  $(OBJ)/serial.o $(OBJ)/lightbar_functions.o $(OBJ)/i2c_functions.o 
	$(CC) $(CFLAGS) $(TEST)/log_test.c $(OBJ)/configuration.o $(OBJ)/scan.o $(OBJ)/serial.o $(OBJ)/AKS_errors.o $(OBJ)/comm.o $(OBJ)/i2c_functions.o $(OBJ)/lightbar_functions.o -o $(TEST)/log_test $(MYSQLFLAGS) -lpthread -lm

clean:
	rm ./bin/*
	rm ./obj/*

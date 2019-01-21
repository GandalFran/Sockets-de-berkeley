CC=gcc
CFLAGS=-g
COFLAGS=-g -c
OBJ= frameConstruction.o

all: tcpServer tcpClient clean
	
tcpServer: tcpServer.c header.h $(OBJ)
	$(CC) $(CFLAGS) tcpServer.c $(OBJ) -o tcpServer
tcpClient: tcpClient.c header.h $(OBJ) 
	$(CC) $(CFLAGS) tcpClient.c $(OBJ) -o tcpClient
	
frameConstruction.o: frameConstruction.c frameConstruction.h header.h
	$(CC) -c frameConstruction.c

clean:
	rm *.o

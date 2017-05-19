OBJ1 = client.o
OBJ2 = server.o
SOURCE1 = clientConnection.h client.cpp
SOURCE2 = serverFunction.h server.cpp
CC = g++-4.9
DEBUG = -g
CFLAGS =  -Wall -std=c++11 -ltbb -c -Wl,-rpath=/usr/local/gcc481/lib64 $(DEBUG)
LFLAGS =  -Wall -std=c++11 -ltbb -Wl,-rpath=/usr/local/gcc481/lib64 $(DEBUG)

all: client server

client: $(OBJ1)
	$(CC) $(LFLAGS) $(OBJ1) -o client

server: $(OBJ2)
	$(CC) $(LFLAGS) $(OBJ2) -o server

client.o: $(SOURCE1)
	$(CC) $(CFLAGS) client.cpp

server.o: $(SOURCE2)
	$(CC) $(CFLAGS) server.cpp

clean:
	\rm *.o *~ client
	\rm *.o *~ server

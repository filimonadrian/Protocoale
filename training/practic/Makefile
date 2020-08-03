# Protocoale de comunicatii:
# Laborator 8: Multiplexare
# Makefile

CFLAGS = -Wall -g
CC = g++

# Portul pe care asculta serverul (de completat)
PORT = 12345

# Adresa IP a serverului (de completat)
IP_SERVER = 127.0.0.1

all: server client

# Compileaza server.c
server: server.cpp
	$(CC) $(CFLAGS) server.cpp -o server
# Compileaza client.c
client: client.cpp
	$(CC) $(CFLAGS) client.cpp -o client

.PHONY: clean run_server run_client	client

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul
run_client:
	./client ${IP_SERVER} ${PORT}

clean:
	rm -f server client

.DEFAULT_GOAL := all
FLAGS = -Wall -lcryptopp -lrocksdb -g3
GEN_FLAGS = -Wall
CC = g++

CLIENT_SOURCES = src/client/*.cpp
SERVER_SOURCES = src/server/*.cpp

CLIENT_INCLUDE_FOLDER = include/client
SERVER_INCLUDE_FOLDER = include/server

CLIENT_NAME = MITRAXOXT_client
SERVER_NAME = MITRAXOXT_server

GENERATION_NAME = generateInputFile
GENERATION_SOURCES = generateInputFile.cpp

clean:
	rm -f $(CLIENT_NAME)
	rm -f $(SERVER_NAME)

client:
	$(CC) -std=c++17 -o $(CLIENT_NAME) $(CLIENT_SOURCES) -I $(CLIENT_INCLUDE_FOLDER) $(FLAGS)

server:
	$(CC) -std=c++17 -o $(SERVER_NAME) $(SERVER_SOURCES) -I $(SERVER_INCLUDE_FOLDER) $(FLAGS)

all: client server

re:	clean all 

generateInputFile:
	rm -f $(GENERATION_NAME)
	$(CC) -std=c++17 -o $(GENERATION_NAME) $(GENERATION_SOURCES) $(GEN_FLAGS)

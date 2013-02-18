server: server.o
	gcc -o server server.o -Wall

server.o: server.c
	gcc -o server.o server.c -c -ansi -pedantic -Wall


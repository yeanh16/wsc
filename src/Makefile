CC=gcc
CFLAGS= -I

all: main_three.c
	gcc -o webserver -pthread main_three.c

main: main_three.c
	gcc -o webserver -pthread main_three.c 
	
ssl: main_threeSSL.c
	gcc -o webserverSSL -pthread main_threeSSL.c -L/usr/local/ssl/lib -lssl -lcrypto

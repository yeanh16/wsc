CC=gcc
CFLAGS= -I

all: main_three.c
	gcc -o main -pthread main_three.c -L/usr/lib -lssl -lcrypto

main: main_three.c
	gcc -o main -pthread main_three.c

CC=gcc
CFLAGS= -I


main: main_three.c
	gcc -o main  -pthread main_three.c 
all:
	clear
	gcc *.c -o pseudontfs -pthread -lm -Wall

all: 
	clear
	gcc -w -Wall tetris.c -o tetris -lintelfpgaup -lpthread -std=c99
	sudo ./tetris
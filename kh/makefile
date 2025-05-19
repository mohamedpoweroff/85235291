prog: main.o fonction.o
	gcc main.o fonction.o -o prog -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf -g

main.o: main.c header.h
	gcc -c main.c -g

fonction.o: fonction.c header.h
	gcc -c fonction.c -g


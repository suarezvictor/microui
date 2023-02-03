CFLAGS=-Wall -std=c11 -pedantic -O3 -g
LIBS=-lGL `sdl2-config --libs` -lm
INC=-I../src 

all: microui
	./microui

microui: main.c renderer.c renderer.h ../src/microui.c ../src/microui.h
	gcc $(INC) $(CFLAGS) main.c renderer.c ../src/microui.c $(LIBS) -o microui

clean:
	rm -f *.o microui

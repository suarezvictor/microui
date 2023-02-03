CFLAGS=-Wall -std=c11 -pedantic -O3 -g
LIBS=-lGL `sdl2-config --libs` -lm
INC=-I../src 

all: microui
	./microui

microui:
	gcc $(INC) $(CFLAGS) main.c renderer.c ../src/microui.c $(LIBS) -o microui

clean:
	rm -f *.o microui

CFLAGS=-Wall -std=c11 -pedantic -O3 -g -DRENDERER_SOFTWARE
LIBS=-lGL `sdl2-config --libs` -lm
INC=-I../src -I../../CflexHDL/include

all: microui
	./microui

microui: main.c renderer.c renderer.h ../src/microui.c ../src/microui.h renderer_sdl.c
	gcc $(INC) $(CFLAGS) main.c renderer.c ../src/microui.c $(LIBS) -o microui

clean:
	rm -f *.o microui

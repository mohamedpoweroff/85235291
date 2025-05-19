CC = gcc
CFLAGS = -Wall -g `sdl-config --cflags` -I/usr/include/SDL
LDFLAGS = `sdl-config --libs` -lSDL_image -lSDL_ttf

SRC = menu.c
TARGET = menu_app

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o menu/choices.txt

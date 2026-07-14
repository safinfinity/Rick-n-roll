SHELL = cmd.exe
CC = C:/raylib/w64devkit/bin/gcc.exe
CFLAGS = -Wall -O2 -IC:/raylib/raylib/src -IC:/raylib/w64devkit/include
LDFLAGS = -LC:/raylib/w64devkit/lib -lraylib -lopengl32 -lgdi32 -lwinmm -lm
SRC = $(wildcard src/*.c)
OUT = Rick-n-roll.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OUT)

run: $(OUT)
	$(OUT)

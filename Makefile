SHELL = C:/msys64/usr/bin/bash.exe
export PATH := /usr/bin:/mingw64/bin:$(PATH)
CC = C:/msys64/mingw64/bin/gcc.exe
CFLAGS = -Wall -O2
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm -lm
SRC = $(wildcard src/*.c)
OUT = Rick-n-roll.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OUT)

run: $(OUT)
	./$(OUT)

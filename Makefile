CC = C:/msys64/mingw64/bin/gcc.exe
CFLAGS = -Wall -O2
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm -lm
SRC = $(wildcard src/*.c)
OUT = Rick-n-roll.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LDFLAGS)

clean:
	-del /Q $(OUT) 2>nul

run: $(OUT)
	./$(OUT)

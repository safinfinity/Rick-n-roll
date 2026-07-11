CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm -lm
SRC = $(wildcard src/*.c)
OUT = poku-doku.exe

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LDFLAGS)

clean:
	del /Q $(OUT) 2>nul

run: $(OUT)
	./$(OUT)

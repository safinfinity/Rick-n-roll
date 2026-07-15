CC = gcc
CFLAGS = -Wall -O2 //shows warning or flags at optimize lvl 2
LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm -lm // tells location
SRC = $(wildcard src/*.c) // returns all src files
OUT = Rick-n-roll.exe // outputs

all: $(OUT)

$(OUT): $(SRC) //rule-output dependent on src files
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LDFLAGS)//how it works

clean:
	rm -f $(OUT)

run: $(OUT)
	./$(OUT)

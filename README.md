# Rick-n-roll

A digital board game combining classic Ludo with Pokemon battles.

## Features

- Classic Mode: Battle opponents, reach home, HP-based scoring
- Ladder Mode: Capture wild Pokemon, evolve, mystery squares
- 2-4 players (human + AI configurable)
- 6 Pokemon types with type advantages
- Evolution system (3 stages)
- GameBoy-style battle scenes

## How to Build

```
gcc src/*.c -o poku-doku.exe -Wall -lraylib -lopengl32 -lgdi32 -lwinmm -lm
```

Or with Makefile (if `make` is installed):

```
make
```

## Controls

- SPACE: Roll dice / Advance
- R: Restart (game over)
- Mouse: Click buttons

## Project Structure

```
src/
  game.h          - Shared types, structs, constants
  game.c          - State machine and turn logic
  main.c          - Window + game loop
  pokemon.c       - Type system and data
  board.c         - Board layout and movement
  board_render.c  - Board drawing
  dice.c          - Dice rolling and display
  battle.c        - Battle scene
  menu.c          - Menu system
assets/
  sprites/        - Pokemon sprite images
```

## Team

- Person A: Core logic, state machine, battle system
- Person B: Visuals, UI, battle scenes
- Person C: Board, menu, testing
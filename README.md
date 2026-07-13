# Rick-n-roll

A digital board game combining classic Ludo with Pokemon battles. Built in C with raylib 6.

## Features

- **Classic Mode** - Battle opponents, reach home, HP-based scoring
- **Ladder Mode** - Capture wild Pokemon, evolve, mystery squares
- 2-4 players (human + AI configurable)
- 6 Pokemon types with type advantages
- Evolution system (3 stages)
- GameBoy-style battle scenes

## Pokemon Types

| Type | Number | Strong Against |
|------|--------|----------------|
| Fire | 1 | Grass |
| Water | 2 | Fire |
| Grass | 3 | Water |
| Electric | 4 | Grass |
| Psychic | 5 | Dragon |
| Dragon | 6 | Electric |

## How to Build

### Prerequisites
- GCC compiler (MinGW or similar)
- raylib 6 installed

### Build
```bash
gcc src/*.c -o Rick-n-roll.exe -Wall -lraylib -lopengl32 -lgdi32 -lwinmm -lm
```

Or using make:
```bash
make
```

## Controls

- **SPACE** - Roll dice / Advance dialogue
- **R** - Restart (game over screen)
- **Mouse** - Click buttons

## Project Structure

```
Rick-n-roll/
├── src/
│   ├── main.c          # Entry point, game loop
│   ├── game.h          # Core types and structs
│   ├── game.c          # Game logic, state machine
│   ├── pokemon.c/h     # Pokemon types and data
│   ├── board.c/h       # Board initialization
│   ├── board_render.c/h # Board drawing
│   ├── dice.c/h        # Dice rolling and display
│   ├── battle.c/h      # Battle scene
│   ├── menu.c/h        # Menu screens
│   ├── ai.c/h          # AI player logic
│   ├── draft.c/h       # Pokemon selection
│   ├── party.c/h       # Party management
│   └── mystery.c/h     # Mystery square effects
├── assets/
│   └── sprites/        # Pokemon sprites (PNG)
├── Makefile
└── README.md
```

## Game Rules

### Classic Mode
1. Players take turns rolling a dice
2. Move forward along the serpentine path
3. Landing on another player triggers a BATTLE
4. Battles use 3 dice rolls with type bonuses
5. Winner stays, loser goes back to start
6. First to reach HOME wins
7. Final ranking based on HP + position + wins

### Ladder Mode
1. Same movement as Classic
2. Special squares trigger encounters:
   - **Habitat** - Battle wild Pokemon
   - **Evolution** - Evolve your Pokemon
   - **Stone** - Instant evolution
   - **Mystery** - Random effect
   - **Safe** - No encounter
3. Capture wild Pokemon to grow your party (max 3)
4. Winner determined by party power + finish order

## Team

- Person A: Core logic, state machine, battle system
- Person B: Visuals, UI, battle scenes
- Person C: Board, menu, testing

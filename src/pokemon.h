#ifndef POKEMON_H
#define POKEMON_H

#include "game.h"

Pokemon poke_create(const char *name, PokeType type);
void poke_assign_random(Player *players, int count);      // Ladder mode: 1 pokemon per player
void poke_assign_party(Player *players, int count);       // Classic mode: 4 distinct tokens per player

#endif // POKEMON_H

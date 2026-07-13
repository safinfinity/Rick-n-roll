#ifndef POKEMON_H
#define POKEMON_H

#include "game.h"

const char* poke_type_name(PokeType t);
Color poke_type_color(PokeType t);
bool poke_type_advantage(PokeType attacker, PokeType defender);
Pokemon poke_create(const char *name, PokeType type);
void poke_assign_random(Player *players, int count);

#endif

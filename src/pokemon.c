#include "game.h"
#include "pokemon.h"

static const char* type_names[] = {
    "None", "Fire", "Water", "Grass", "Electric", "Psychic", "Dragon"
};

static const Color type_colors[] = {
    {0, 0, 0, 0},
    {255, 87, 34, 255},    // Fire
    {33, 150, 243, 255},   // Water
    {76, 175, 80, 255},    // Grass
    {255, 235, 59, 255},   // Electric
    {156, 39, 176, 255},   // Psychic
    {121, 85, 72, 255}     // Dragon
};

// TODO: fill in type advantage chart
static int advantage[7][7] = {0};

const char* poke_type_name(PokeType t) {
    // TODO: return type name
    return "Unknown";
}

Color poke_type_color(PokeType t) {
    // TODO: return type color
    return GRAY;
}

bool poke_type_advantage(PokeType attacker, PokeType defender) {
    // TODO: check if attacker has advantage over defender
    return false;
}

Pokemon poke_create(const char *name, PokeType type) {
    // TODO: create a pokemon with default stats
    Pokemon p = {0};
    return p;
}

void poke_assign_random(Player *players, int count) {
    // TODO: assign random pokemon to each player
}

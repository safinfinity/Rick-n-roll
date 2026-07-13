#include "game.h"
#include "pokemon.h"
#include <stdlib.h>
#include <string.h>

static const char* type_names[] = {
    "None", "Fire", "Water", "Grass", "Electric", "Psychic", "Dragon"
};

static const Color type_colors[] = {
    {0, 0, 0, 0},
    {255, 87, 34, 255},
    {33, 150, 243, 255},
    {76, 175, 80, 255},
    {255, 235, 59, 255},
    {156, 39, 176, 255},
    {121, 85, 72, 255}
};

static int advantage[7][7] = {
    // NONE  FIRE  WATER GRASS ELEC PSYCH DRAGON
    {0,    0,    0,    0,    0,    0,    0},     // NONE
    {0,    0,    0,    1,    0,    0,    0},     // FIRE > GRASS
    {0,    1,    0,    0,    0,    0,    0},     // WATER > FIRE
    {0,    0,    1,    0,    0,    0,    0},     // GRASS > WATER
    {0,    0,    0,    1,    0,    0,    0},     // ELECTRIC > GRASS
    {0,    0,    0,    0,    0,    0,    1},     // PSYCHIC > DRAGON
    {0,    0,    0,    0,    1,    0,    0},     // DRAGON > ELECTRIC
};

const char* poke_type_name(PokeType t) {
    if (t < 0 || t >= 7) return "Unknown";
    return type_names[t];
}

Color poke_type_color(PokeType t) {
    if (t < 0 || t >= 7) return GRAY;
    return type_colors[t];
}

bool poke_type_advantage(PokeType attacker, PokeType defender) {
    if (attacker < 0 || attacker >= 7 || defender < 0 || defender >= 7)
        return false;
    return advantage[attacker][defender] == 1;
}


Pokemon poke_create(const char *name, PokeType type) {
    Pokemon p = {0};
    p.name = name;
    p.type = type;
    p.hp = BASE_HP;
    p.maxHp = BASE_HP;
    p.atk = 10;
    p.def = 10;
    p.color = type_colors[type];
    return p;
}

void poke_assign_random(Player *players, int count) {
    PokeType types[] = {POKE_FIRE, POKE_WATER, POKE_GRASS, POKE_ELECTRIC, POKE_PSYCHIC, POKE_DRAGON};
    int n = sizeof(types) / sizeof(types[0]);

    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        PokeType tmp = types[i];
        types[i] = types[j];
        types[j] = tmp;
    }

    for (int i = 0; i < count; i++) {
        players[i].pokemon = poke_create(type_names[types[i]], types[i]);
    }
}

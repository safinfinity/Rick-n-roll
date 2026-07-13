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
    {0,    0,    0,    2,    0,    0,    0},     // FIRE > GRASS
    {0,    2,    0,    0,    0,    0,    0},     // WATER > FIRE
    {0,    0,    2,    0,    0,    0,    0},     // GRASS > WATER
    {0,    0,    2,    0,    0,    0,    0},     // ELECTRIC > WATER
    {0,    0,    0,    0,    0,    0,    2},     // PSYCHIC > DRAGON
    {0,    2,    0,    0,    2,    0,    0},     // DRAGON > FIRE, DRAGON > ELECTRIC
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
    return advantage[attacker][defender] > 0;
}

typedef struct {
    const char* name;
    PokeType type;
    int hp;
    int atk;
    int def;
} PokeTemplate;

static const PokeTemplate templates[] = {
    {"Charmander", POKE_FIRE, 80, 12, 8},
    {"Vulpix", POKE_FIRE, 70, 15, 6},
    {"Growlithe", POKE_FIRE, 90, 14, 9},
    {"Squirtle", POKE_WATER, 80, 10, 12},
    {"Psyduck", POKE_WATER, 70, 13, 7},
    {"Lapras", POKE_WATER, 110, 11, 14},
    {"Bulbasaur", POKE_GRASS, 80, 11, 10},
    {"Oddish", POKE_GRASS, 65, 14, 6},
    {"Tangela", POKE_GRASS, 85, 12, 12},
    {"Pikachu", POKE_ELECTRIC, 70, 16, 5},
    {"Magnemite", POKE_ELECTRIC, 60, 14, 10},
    {"Electabuzz", POKE_ELECTRIC, 90, 15, 9},
    {"Abra", POKE_PSYCHIC, 55, 18, 4},
    {"Mr. Mime", POKE_PSYCHIC, 75, 14, 10},
    {"Mewtwo", POKE_PSYCHIC, 130, 20, 15},
    {"Dratini", POKE_DRAGON, 85, 13, 11},
    {"Dragonair", POKE_DRAGON, 100, 16, 13},
    {"Dragonite", POKE_DRAGON, 120, 18, 16},
};

#define TEMPLATE_COUNT (sizeof(templates) / sizeof(templates[0]))

static const PokeTemplate starter_pool[] = {
    {"Charmander", POKE_FIRE, 80, 12, 8},
    {"Squirtle", POKE_WATER, 80, 10, 12},
    {"Bulbasaur", POKE_GRASS, 80, 11, 10},
    {"Pikachu", POKE_ELECTRIC, 70, 16, 5},
    {"Abra", POKE_PSYCHIC, 55, 18, 4},
    {"Dratini", POKE_DRAGON, 85, 13, 11},
};
#define STARTER_COUNT (sizeof(starter_pool) / sizeof(starter_pool[0]))

Pokemon poke_create(const char *name, PokeType type) {
    for (size_t i = 0; i < TEMPLATE_COUNT; i++) {
        if (strcmp(templates[i].name, name) == 0 && templates[i].type == type) {
            Pokemon p;
            p.name = templates[i].name;
            p.type = templates[i].type;
            p.hp = templates[i].hp;
            p.maxHp = templates[i].hp;
            p.atk = templates[i].atk;
            p.def = templates[i].def;
            p.color = type_colors[templates[i].type];
            return p;
        }
    }
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
    int used[STARTER_COUNT];
    memset(used, 0, sizeof(used));

    for (int i = 0; i < count; i++) {
        int idx;
        do {
            idx = rand() % STARTER_COUNT;
        } while (used[idx]);
        used[idx] = 1;

        players[i].pokemon = poke_create(starter_pool[idx].name, starter_pool[idx].type);
    }
}

#include "game.h"
#include "pokemon.h"
#include <stdlib.h>
#include <string.h>

static const char* type_names[] = {
    "None", "Fire", "Water", "Grass", "Electric", "Psychic", "Dragon", "Ice", "Fighting"
};

static const Color type_colors[] = {
    {0, 0, 0, 0},
    {255, 87, 34, 255},
    {33, 150, 243, 255},
    {76, 175, 80, 255},
    {255, 235, 59, 255},
    {156, 39, 176, 255},
    {121, 85, 72, 255},
    {90, 170, 255, 255},
    {190, 120, 80, 255}
};

static int advantage[9][9] = {
    /*              NONE FIRE WATER GRASS ELECTRIC PSYCHIC DRAGON ICE FIGHTING */
    /* NONE */    { 0,   0,   0,    0,    0,       0,      0,     0,  0 },

    /* FIRE */    { 0,   0,   0,    1,    0,       0,      0,     1,  0 },

    /* WATER */   { 0,   1,   0,    0,    0,       0,      0,     0,  0 },

    /* GRASS */   { 0,   0,   1,    0,    0,       0,      0,     0,  0 },

    /* ELECTRIC */{ 0,   0,   1,    0,    0,       0,      0,     0,  0 },

    /* PSYCHIC */ { 0,   0,   0,    0,    0,       0,      0,     0,  1 },

    /* DRAGON */  { 0,   0,   0,    0,    0,       0,      1,     0,  0 },

    /* ICE */     { 0,   0,   0,    1,    0,       0,      1,     0,  0 },

    /* FIGHTING */{ 0,   0,   0,    0,    0,       1,      0,     1,  0 }
};

const char* poke_type_name(PokeType t) {
    if (t < 0 || t > POKE_FIGHTING) return "Unknown";
    return type_names[t];
}

Color poke_type_color(PokeType t) {
    if (t < 0 || t > POKE_FIGHTING) return GRAY;
    return type_colors[t];
}

bool poke_type_advantage(PokeType attacker, PokeType defender) {

    if (attacker <= POKE_NONE ||
        attacker > POKE_FIGHTING ||
        defender <= POKE_NONE ||
        defender > POKE_FIGHTING) {
        return false;
    }

    return advantage[attacker][defender] != 0;
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
    {"Glaceon", POKE_ICE, 90, 14, 12},
{"Machamp", POKE_FIGHTING, 105, 17, 13},
};

#define TEMPLATE_COUNT (sizeof(templates) / sizeof(templates[0]))

static const PokeTemplate starter_pool[] = {
    {"Charmander", POKE_FIRE, 80, 12, 8},
    {"Squirtle", POKE_WATER, 80, 10, 12},
    {"Bulbasaur", POKE_GRASS, 80, 11, 10},
    {"Pikachu", POKE_ELECTRIC, 70, 16, 5},
    {"Abra", POKE_PSYCHIC, 55, 18, 4},
    {"Dratini", POKE_DRAGON, 85, 13, 11},
    {"Glaceon", POKE_ICE, 90, 14, 12},
    {"Machamp", POKE_FIGHTING, 105, 17, 13}
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

// Classic Mode: each player gets TOKENS_PER_PLAYER (2) distinct Pokemon sampled
// (without replacement) from the 6-Pokemon pool. Fisher-Yates shuffle keeps it
// fair and independent per player. Each token starts in its base.
static int pokemon_power(const PokeTemplate *p) {
    // Overall combat strength score.
    // Higher = stronger.
    return p->hp + (p->atk * 3) + (p->def * 2);
}


void poke_assign_party(Player *players, int count) {

    /*
     * We have 8 Pokemon and up to 4 players.
     *
     * The 8 Pokemon are ranked by their actual stats and
     * divided into 4 strength tiers:
     *
     * Tier 1 = 2 weakest
     * Tier 2 = next 2
     * Tier 3 = next 2
     * Tier 4 = 2 strongest
     *
     * Every player receives exactly ONE Pokemon from
     * every tier.
     *
     * Therefore nobody can get all the strongest Pokemon.
     */

    if (count <= 0 || count > MAX_PLAYERS)
        return;


    /* --------------------------------------------------
       1. Create an array containing all 8 Pokemon indexes
       -------------------------------------------------- */

    int sorted[STARTER_COUNT];

    for (int i = 0; i < STARTER_COUNT; i++) {
        sorted[i] = i;
    }


    /* --------------------------------------------------
       2. Sort Pokemon from weakest → strongest
       -------------------------------------------------- */

    for (int i = 0; i < STARTER_COUNT - 1; i++) {

        for (int j = i + 1; j < STARTER_COUNT; j++) {

            int powerI = pokemon_power(&starter_pool[sorted[i]]);
            int powerJ = pokemon_power(&starter_pool[sorted[j]]);

            if (powerJ < powerI) {

                int temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }


    /* --------------------------------------------------
       3. Divide the 8 Pokemon into 4 strength tiers
       -------------------------------------------------- */

    int tiers[4][2];

    for (int tier = 0; tier < 4; tier++) {
        tiers[tier][0] = sorted[tier * 2];
        tiers[tier][1] = sorted[tier * 2 + 1];
    }


    /*
     * With your current stats, this will approximately be:
     *
     * Tier 1:
     *   Abra
     *   Pikachu
     *
     * Tier 2:
     *   Squirtle
     *   Charmander
     *
     * Tier 3:
     *   Bulbasaur
     *   Dratini
     *
     * Tier 4:
     *   Glaceon
     *   Machamp
     *
     * The exact order is calculated automatically from stats.
     */


    /* --------------------------------------------------
       4. Give each player one Pokemon from each tier
       -------------------------------------------------- */

    for (int tier = 0; tier < 4; tier++) {

        /*
         * Each Pokemon in this tier must appear exactly
         * enough times to give one to every player.
         *
         * Example with 4 players:
         *
         * Pokemon A × 2
         * Pokemon B × 2
         *
         * Total = 4
         */

        int deck[MAX_PLAYERS];

        int deckIndex = 0;

        for (int copy = 0; copy < count; copy++) {

            /*
             * Randomly choose which of the two Pokemon
             * gets this copy.
             */
            deck[deckIndex++] = tiers[tier][copy % 2];
        }


        /* --------------------------------------------------
           5. Shuffle this tier
           -------------------------------------------------- */

        for (int i = count - 1; i > 0; i--) {

            int j = rand() % (i + 1);

            int temp = deck[i];
            deck[i] = deck[j];
            deck[j] = temp;
        }


        /* --------------------------------------------------
           6. Deal one Pokemon from this tier to each player
           -------------------------------------------------- */

        for (int p = 0; p < count; p++) {

            int pokemonIndex = deck[p];

            players[p].tokens[tier].owner = p;

            players[p].tokens[tier].pokemon =
                poke_create(
                    starter_pool[pokemonIndex].name,
                    starter_pool[pokemonIndex].type
                );

            players[p].tokens[tier].state = TOKEN_BASE;
            players[p].tokens[tier].progress = 0;
        }
    }


    /* --------------------------------------------------
       7. Reset player completion information
       -------------------------------------------------- */

    for (int p = 0; p < count; p++) {

        players[p].finishedCount = 0;
        players[p].finished = false;
        players[p].finishOrder = 0;
    }


    /* --------------------------------------------------
       8. Randomize the four token positions
       -------------------------------------------------- */

    for (int p = 0; p < count; p++) {

        for (int i = TOKENS_PER_PLAYER - 1; i > 0; i--) {

            int j = rand() % (i + 1);

            Token temp = players[p].tokens[i];

            players[p].tokens[i] =
                players[p].tokens[j];

            players[p].tokens[j] =
                temp;
        }

        /*
         * Owner remains the same after swapping.
         */
        for (int k = 0; k < TOKENS_PER_PLAYER; k++) {
            players[p].tokens[k].owner = p;
        }
    }
}
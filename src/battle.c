#include "game.h"
#include "battle.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void init_battle(Game *g, int attacker, int defender) {
    BattleState *b = &g->battle;
    b->attackerIdx = attacker;
    b->defenderIdx = defender;
    b->rollsLeft = DICE_ROLLS_PER_BATTLE;
    b->currentRoll = 0;
    b->attackerHp = g->players[attacker].pokemon.hp;
    b->defenderHp = g->players[defender].pokemon.hp;
    b->attackerMaxHp = g->players[attacker].pokemon.maxHp;
    b->defenderMaxHp = g->players[defender].pokemon.maxHp;
    b->message[0] = '\0';
    b->messageTimer = 0;
    b->finished = false;
    b->attackerWon = false;
}

void battle_update(Game *g) {
    BattleState *b = &g->battle;
    if (b->finished) return;

    if (b->messageTimer > 0) {
        b->messageTimer--;
        return;
    }

    if (b->rollsLeft <= 0 || b->attackerHp <= 0 || b->defenderHp <= 0) {
        b->finished = true;
        if (b->defenderHp <= 0) b->attackerWon = true;
        else if (b->attackerHp <= 0) b->attackerWon = false;
        else {
            b->attackerWon = b->attackerHp > b->defenderHp;
        }

        Player *atk = &g->players[b->attackerIdx];
        Player *def = &g->players[b->defenderIdx];

        if (b->attackerWon) {
            def->position = 0;
            def->pokemon.hp = def->pokemon.maxHp;
            atk->pokemon.hp = atk->pokemon.maxHp;
            snprintf(b->message, sizeof(b->message), "%s wins! %s sent back!", atk->name, def->name);
        } else {
            atk->position = 0;
            atk->pokemon.hp = atk->pokemon.maxHp;
            def->pokemon.hp = def->pokemon.maxHp;
            snprintf(b->message, sizeof(b->message), "%s wins! %s sent back!", def->name, atk->name);
        }
        b->messageTimer = 90;
        return;
    }

    Player *atk = &g->players[b->attackerIdx];
    Player *def = &g->players[b->defenderIdx];

    int atkRoll = (rand() % 6) + 1 + atk->pokemon.atk / 10;
    int defRoll = (rand() % 6) + 1 + def->pokemon.def / 10;

    if (poke_type_advantage(atk->pokemon.type, def->pokemon.type)) {
        atkRoll += TYPE_ADVANTAGE_BONUS;
        snprintf(b->message, sizeof(b->message), "Super effective! %s +%d!", atk->pokemon.name, TYPE_ADVANTAGE_BONUS);
    } else if (poke_type_advantage(def->pokemon.type, atk->pokemon.type)) {
        defRoll += TYPE_ADVANTAGE_BONUS;
        snprintf(b->message, sizeof(b->message), "Super effective! %s +%d!", def->pokemon.name, TYPE_ADVANTAGE_BONUS);
    } else {
        snprintf(b->message, sizeof(b->message), "%s rolls %d vs %s rolls %d", atk->pokemon.name, atkRoll, def->pokemon.name, defRoll);
    }

    int damage = (atkRoll > defRoll) ? atkRoll - defRoll : 0;
    if (damage > 0) {
        def->pokemon.hp -= damage;
        if (def->pokemon.hp < 0) def->pokemon.hp = 0;
        b->defenderHp = def->pokemon.hp;
    } else {
        damage = defRoll - atkRoll;
        atk->pokemon.hp -= damage;
        if (atk->pokemon.hp < 0) atk->pokemon.hp = 0;
        b->attackerHp = atk->pokemon.hp;
    }

    b->rollsLeft--;
    b->messageTimer = 45;
}

void battle_draw(Game *g) {
    BattleState *b = &g->battle;
    Player *atk = &g->players[b->attackerIdx];
    Player *def = &g->players[b->defenderIdx];

    DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){15, 15, 35, 255});

    DrawText("POKEMON BATTLE!", WINDOW_W/2 - MeasureText("POKEMON BATTLE!", 32)/2, 20, 32, GOLD);

    float leftX = WINDOW_W * 0.15f;
    float rightX = WINDOW_W * 0.65f;
    float pokeY = 120;

    DrawCircleV((Vector2){leftX + 80, pokeY + 60}, 60, atk->pokemon.color);
    DrawCircleLinesV((Vector2){leftX + 80, pokeY + 60}, 60, WHITE);
    DrawText(atk->pokemon.name, (int)leftX, (int)(pokeY + 130), 20, WHITE);
    DrawText(TextFormat("Lv.%d", atk->pokemon.atk), (int)leftX, (int)(pokeY + 155), 14, GRAY);

    DrawCircleV((Vector2){rightX + 80, pokeY + 60}, 60, def->pokemon.color);
    DrawCircleLinesV((Vector2){rightX + 80, pokeY + 60}, 60, WHITE);
    DrawText(def->pokemon.name, (int)rightX, (int)(pokeY + 130), 20, WHITE);
    DrawText(TextFormat("Lv.%d", def->pokemon.atk), (int)rightX, (int)(pokeY + 155), 14, GRAY);

    DrawText("VS", WINDOW_W/2 - 25, (int)(pokeY + 40), 40, RED);

    int barW = 200, barH = 18;
    float hpY = pokeY + 190;

    DrawText(atk->name, (int)leftX - 10, (int)hpY, 14, atk->color);
    DrawRectangle((int)leftX - 10, (int)(hpY + 20), barW, barH, DARKGRAY);
    float atkRatio = (float)b->attackerHp / b->attackerMaxHp;
    DrawRectangle((int)leftX - 10, (int)(hpY + 20), (int)(barW * atkRatio), barH, RED);
    DrawText(TextFormat("%d/%d", b->attackerHp, b->attackerMaxHp), (int)leftX + 40, (int)(hpY + 21), 12, WHITE);

    DrawText(def->name, (int)rightX - 10, (int)hpY, 14, def->color);
    DrawRectangle((int)rightX - 10, (int)(hpY + 20), barW, barH, DARKGRAY);
    float defRatio = (float)b->defenderHp / b->defenderMaxHp;
    DrawRectangle((int)rightX - 10, (int)(hpY + 20), (int)(barW * defRatio), barH, RED);
    DrawText(TextFormat("%d/%d", b->defenderHp, b->defenderMaxHp), (int)rightX + 40, (int)(hpY + 21), 12, WHITE);

    DrawText(TextFormat("Rounds left: %d", b->rollsLeft), WINDOW_W/2 - MeasureText(TextFormat("Rounds left: %d", b->rollsLeft), 18)/2, (int)hpY + 60, 18, LIGHTGRAY);

    if (b->message[0]) {
        int mw = MeasureText(b->message, 16);
        int my = (int)hpY + 100;
        DrawRectangle(WINDOW_W/2 - mw/2 - 15, my - 5, mw + 30, 35, (Color){0, 0, 0, 200});
        DrawRectangleLines(WINDOW_W/2 - mw/2 - 15, my - 5, mw + 30, 35, WHITE);
        DrawText(b->message, WINDOW_W/2 - mw/2, my + 5, 16, WHITE);
    }

    if (b->finished) {
        const char *winText = b->attackerWon ? atk->name : def->name;
        char result[64];
        snprintf(result, sizeof(result), "%s wins the battle!", winText);
        int rw = MeasureText(result, 24);
        DrawRectangle(WINDOW_W/2 - rw/2 - 20, WINDOW_H - 100, rw + 40, 50, (Color){0, 0, 0, 200});
        DrawRectangleLines(WINDOW_W/2 - rw/2 - 20, WINDOW_H - 100, rw + 40, 50, GOLD);
        DrawText(result, WINDOW_W/2 - rw/2, WINDOW_H - 85, 24, GOLD);
    }
}

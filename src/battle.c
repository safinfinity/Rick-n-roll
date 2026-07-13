#include "game.h"
#include "pokemon.h"
#include <stdio.h>

void battle_draw(Game *g) {
    DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){10, 10, 25, 255});

    DrawText("BATTLE!", WINDOW_W/2 - MeasureText("BATTLE!", 36)/2, 30, 36, (Color){255, 202, 40, 255});

    int atk = g->battle.attackerIdx;
    int def = g->battle.defenderIdx;

    DrawRectangle(100, 200, 150, 150, g->players[atk].pokemon.color);
    DrawRectangleLines(100, 200, 150, 150, WHITE);

    DrawText(poke_type_name(g->players[atk].pokemon.type), 100, 360, 18, WHITE);
    DrawText(g->players[atk].name, 100, 380, 14, (Color){180, 180, 200, 255});

    DrawRectangle(100, 410, 150, 16, (Color){60, 60, 80, 255});
    float hpPct = (float)g->battle.attackerHp / g->battle.attackerMaxHp;
    Color hpColor = hpPct > 0.5f ? GREEN : hpPct > 0.25f ? YELLOW : RED;
    DrawRectangle(100, 410, (int)(150 * hpPct), 16, hpColor);
    char hpBuf[32];
    sprintf(hpBuf, "HP: %d/%d", g->battle.attackerHp, g->battle.attackerMaxHp);
    DrawText(hpBuf, 100, 430, 12, WHITE);

    DrawRectangle(550, 200, 150, 150, g->players[def].pokemon.color);
    DrawRectangleLines(550, 200, 150, 150, WHITE);

    DrawText(poke_type_name(g->players[def].pokemon.type), 550, 360, 18, WHITE);
    DrawText(g->players[def].name, 550, 380, 14, (Color){180, 180, 200, 255});

    DrawRectangle(550, 410, 150, 16, (Color){60, 60, 80, 255});
    hpPct = (float)g->battle.defenderHp / g->battle.defenderMaxHp;
    hpColor = hpPct > 0.5f ? GREEN : hpPct > 0.25f ? YELLOW : RED;
    DrawRectangle(550, 410, (int)(150 * hpPct), 16, hpColor);
    sprintf(hpBuf, "HP: %d/%d", g->battle.defenderHp, g->battle.defenderMaxHp);
    DrawText(hpBuf, 550, 430, 12, WHITE);

    DrawText("VS", WINDOW_W/2 - 15, 250, 28, (Color){255, 82, 82, 255});

    DrawRectangle(50, WINDOW_H - 180, WINDOW_W - 100, 140, (Color){20, 20, 40, 255});
    DrawRectangleLines(50, WINDOW_H - 180, WINDOW_W - 100, 140, (Color){255, 202, 40, 255});

    DrawText(g->battle.message, 80, WINDOW_H - 150, 20, WHITE);

    char rollBuf[32];
    sprintf(rollBuf, "Rolls left: %d", g->battle.rollsLeft);
    DrawText(rollBuf, 80, WINDOW_H - 120, 14, (Color){180, 180, 200, 255});

    if (!g->battle.finished) {
        DrawText("Press SPACE to roll dice", 80, WINDOW_H - 80, 16, (Color){255, 202, 40, 255});
    } else {
        DrawText("Press SPACE to continue", 80, WINDOW_H - 80, 16, (Color){0, 230, 118, 255});
    }

    if (g->battle.currentRoll > 0) {
        char diceBuf[32];
        sprintf(diceBuf, "Last roll: %d", g->battle.currentRoll);
        DrawText(diceBuf, WINDOW_W/2 - 60, WINDOW_H - 150, 18, (Color){255, 202, 40, 255});
    }
}

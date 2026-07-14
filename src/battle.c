#include "game.h"
#include "pokemon.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define TYPE_DOT_R 10
#define SPRITE_W 128
#define SPRITE_H 128
#define CARD_PAD 12

static void draw_hp_bar(int x, int y, int w, int cur, int max) {
    DrawRectangleLinesEx((Rectangle){(float)x - 1, (float)y - 1, (float)w + 2, 14}, 2, BLACK);
    DrawRectangle(x, y, w, 12, (Color){80, 80, 80, 255});
    float pct = (float)cur / max;
    if (pct < 0) pct = 0;
    Color c = pct > 0.5f ? (Color){0, 200, 80, 255} : pct > 0.25f ? (Color){220, 200, 0, 255} : (Color){240, 40, 40, 255};
    DrawRectangle(x + 1, y + 1, (int)((w - 2) * pct), 10, c);
}

static void draw_hp_text(int x, int y, int cur, int max) {
    char buf[32];
    sprintf(buf, "%3d/%3d", cur, max);
    DrawText(buf, x, y, 12, BLACK);
}

static void draw_type_badge(int x, int y, PokeType t) {
    if (t == POKE_NONE) return;
    Color c = poke_type_color(t);
    DrawCircle(x + TYPE_DOT_R, y + TYPE_DOT_R, (float)TYPE_DOT_R, c);
    DrawCircleLines(x + TYPE_DOT_R, y + TYPE_DOT_R, (float)TYPE_DOT_R, WHITE);
    DrawText(poke_type_name(t), x + TYPE_DOT_R * 2 + 6, y + 4, 14, WHITE);
}

static void draw_sprite(int x, int y, Texture2D *tex) {
    if (tex->width > 0) {
        float scale = fminf((float)SPRITE_W / tex->width, (float)SPRITE_H / tex->height);
        float sw = tex->width * scale;
        float sh = tex->height * scale;
        float sx = x + (SPRITE_W - sw) / 2;
        float sy = y + (SPRITE_H - sh) / 2;
        DrawTextureEx(*tex, (Vector2){sx, sy}, 0, scale, WHITE);
    } else {
        DrawRectangle(x + 4, y + 4, SPRITE_W - 8, SPRITE_H - 8, (Color){60, 60, 80, 255});
    }
}

void battle_draw(Game *g) {
    BattleState *b = &g->battle;
    int atk = b->attackerIdx;
    int def = b->defenderIdx;

    DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){30, 40, 20, 255});

    // ── Header ──
    DrawText("BATTLE!", WINDOW_W / 2 - MeasureText("BATTLE!", 32) / 2, 16, 32, (Color){255, 202, 40, 255});

    // ── Enemy (defender) ── top-right area ──
    {
        Player *p = &g->players[def];
        int ex = WINDOW_W - 450, ey = 70;
        int sx = ex, sy = ey;
        int labX = ex + SPRITE_W + 16, labY = ey + 10;

        DrawRectangleLinesEx((Rectangle){(float)sx - 2, (float)sy - 2, (float)SPRITE_W + 4, (float)SPRITE_H + 4}, 2, (Color){50, 50, 50, 255});
        draw_sprite(sx, sy, &g->pokeSprites[p->pokemon.type]);
        DrawRectangleLinesEx((Rectangle){(float)sx, (float)sy, (float)SPRITE_W, (float)SPRITE_H}, 2, WHITE);

        DrawText(p->name, labX, labY, 16, BLACK);
        char lvBuf[16];
        sprintf(lvBuf, "Lv %d", 10);
        DrawText(lvBuf, labX + MeasureText(p->name, 16) + 10, labY, 14, (Color){60, 60, 60, 255});

        draw_hp_bar(labX, labY + 28, 130, b->defenderHp, b->defenderMaxHp);
        draw_hp_text(labX, labY + 44, b->defenderHp, b->defenderMaxHp);

        draw_type_badge(labX, labY + 62, p->pokemon.type);
    }

    // ── Player (attacker) ── bottom-left area ──
    {
        Player *p = &g->players[atk];
        int px = 50, py = WINDOW_H - 260;
        int sx = px + 250, sy = py;
        int labX = px, labY = py + 8;

        DrawRectangleLinesEx((Rectangle){(float)sx - 2, (float)sy - 2, (float)SPRITE_W + 4, (float)SPRITE_H + 4}, 2, (Color){50, 50, 50, 255});
        draw_sprite(sx, sy, &g->pokeSprites[p->pokemon.type]);
        DrawRectangleLinesEx((Rectangle){(float)sx, (float)sy, (float)SPRITE_W, (float)SPRITE_H}, 2, WHITE);

        DrawText(p->name, labX, labY, 16, BLACK);
        char lvBuf[16];
        sprintf(lvBuf, "Lv %d", 10);
        DrawText(lvBuf, labX + MeasureText(p->name, 16) + 10, labY, 14, (Color){60, 60, 60, 255});

        draw_hp_bar(labX, labY + 28, 130, b->attackerHp, b->attackerMaxHp);
        draw_hp_text(labX, labY + 44, b->attackerHp, b->attackerMaxHp);

        draw_type_badge(labX, labY + 62, p->pokemon.type);
    }

    // ── Center: VS + dice ──
    {
        DrawText("VS", WINDOW_W / 2 - 15, 200, 28, (Color){255, 82, 82, 255});

        if (b->currentRoll > 0) {
            int diceSize = 64;
            int dx = WINDOW_W / 2 - diceSize / 2;
            int dy = 250;
            DrawRectangle(dx, dy, diceSize, diceSize, (Color){40, 40, 60, 255});
            DrawRectangleLines(dx, dy, diceSize, diceSize, (Color){255, 202, 40, 255});

            int cx = dx + diceSize / 2;
            int cy = dy + diceSize / 2;
            int dotR = 5;
            typedef struct { int dx, dy; } Dot;
            static const Dot dots[7][9] = {
                {},
                {{0,0}},
                {{-10,-10},{10,10}},
                {{-10,-10},{0,0},{10,10}},
                {{-10,-10},{10,-10},{-10,10},{10,10}},
                {{-10,-10},{10,-10},{0,0},{-10,10},{10,10}},
                {{-10,-10},{10,-10},{-10,0},{10,0},{-10,10},{10,10}}
            };
            int val = b->currentRoll;
            if (val >= 1 && val <= 6) {
                for (int i = 0; i < val; i++)
                    DrawCircle(cx + dots[val][i].dx, cy + dots[val][i].dy, dotR, (Color){255, 202, 40, 255});
            }
        }
    }

    // ── Flash overlay ──
    if (b->flashTimer > 0) {
        int alpha = (b->flashTimer * 200) / 15;
        if (alpha > 200) alpha = 200;
        DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){255, 255, 255, (unsigned char)alpha});
        DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){255, 255, 200, (unsigned char)(alpha / 3)});
        b->flashTimer--;
    }

    // ── Type advantage check ──
    bool atkAdv = poke_type_advantage(g->players[atk].pokemon.type, g->players[def].pokemon.type);
    bool defAdv = poke_type_advantage(g->players[def].pokemon.type, g->players[atk].pokemon.type);

    // ── Bottom text box ──
    int boxX = 40, boxY = WINDOW_H - 170, boxW = WINDOW_W - 80, boxH = 150;
    DrawRectangle(boxX, boxY, boxW, boxH, (Color){40, 40, 60, 255});
    DrawRectangleLines(boxX, boxY, boxW, boxH, (Color){120, 120, 140, 255});
    DrawRectangleLines(boxX + 2, boxY + 2, boxW - 4, boxH - 4, (Color){60, 60, 80, 255});
    DrawRectangleLines(boxX + 4, boxY + 4, boxW - 8, boxH - 8, (Color){200, 200, 220, 255});

    int textY = boxY + 18;

    if (b->message[0] != '\0') {
        DrawText(b->message, boxX + 24, textY, 18, WHITE);
        textY += 28;
    }

    if (atkAdv) {
        DrawText("IT'S SUPER EFFECTIVE! (ATK)", boxX + 24, textY, 16, (Color){255, 200, 50, 255});
        textY += 24;
    } else if (defAdv) {
        DrawText("IT'S SUPER EFFECTIVE! (DEF)", boxX + 24, textY, 16, (Color){255, 200, 50, 255});
        textY += 24;
    }

    char rollBuf[32];
    sprintf(rollBuf, "Rolls left: %d / %d", b->rollsLeft, DICE_ROLLS_PER_BATTLE);
    DrawText(rollBuf, boxX + 24, textY, 14, (Color){180, 180, 200, 255});
    textY += 22;

    if (!b->finished) {
        DrawText("Press SPACE to roll the dice", boxX + 24, textY, 16, (Color){255, 202, 40, 255});
    } else {
        DrawText("Press SPACE to continue", boxX + 24, textY, 16, (Color){0, 230, 118, 255});
    }

    // ── Sprites flash on hit ──
    if (b->flashTimer > 0 && (b->flashTimer % 4 < 2)) {
        DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){255, 255, 255, 40});
    }
}

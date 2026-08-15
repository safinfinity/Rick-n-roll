#include "game.h"
#include "board.h"
#include "board_render.h"
#include "pokemon.h"
#include <stdio.h>

#define CELL_SIZE 100
#define BOARD_X 60
#define BOARD_Y 140

// Classic Ludo layout (must match board.c)
#define LUDO_GRID 15
#define LUDO_CELL 44.0f
#define LUDO_X 30.0f
#define LUDO_Y 100.0f

static Color square_color(SquareType t) {
    switch (t) {
        case SQ_SAFE:     return (Color){200, 230, 200, 255};
        case SQ_LADDER:   return (Color){180, 230, 180, 255};
        case SQ_SNAKE:    return (Color){230, 180, 180, 255};
        case SQ_EVOLUTION:return (Color){210, 180, 230, 255};
        case SQ_HABITAT:  return (Color){180, 210, 230, 255};
        case SQ_MYSTERY:  return (Color){230, 210, 180, 255};
        case SQ_STONE:    return (Color){200, 200, 220, 255};
        default:          return (Color){240, 235, 220, 255};
    }
}

static const char* square_label(SquareType t) {
    switch (t) {
        case SQ_SAFE:     return "SAFE";
        case SQ_LADDER:   return "LADDER";
        case SQ_SNAKE:    return "SNAKE";
        case SQ_EVOLUTION:return "EVOLVE";
        case SQ_HABITAT:  return "WILD";
        case SQ_MYSTERY:  return "MYSTERY";
        case SQ_STONE:    return "STONE";
        default:          return "";
    }
}

static void draw_token_small(Game *g, Token *t, Player *pl, Vector2 pos) {
    Color typeC = poke_type_color(t->pokemon.type);
    DrawCircleV(pos, 10, typeC);
    DrawCircleLinesV(pos, 10, pl->color);
    DrawCircleLinesV(pos, 10, (Color){10, 10, 15, 255});
    Texture2D spr = g->pokeSprites[t->pokemon.type];
    if (spr.id > 0) {
        float scale = 18.0f / spr.width;
        DrawTextureEx(spr, (Vector2){pos.x - 9, pos.y - 9}, 0, scale, WHITE);
    }
}

static void draw_classic_board(Game *g) {
    float bw = LUDO_GRID * LUDO_CELL;
    DrawRectangle((int)LUDO_X - 16, (int)LUDO_Y - 16, (int)bw + 32, (int)bw + 32, (Color){25, 25, 45, 255});
    DrawRectangleLinesEx((Rectangle){LUDO_X - 16, LUDO_Y - 16, bw + 32, bw + 32}, 2, (Color){90, 90, 120, 255});

    // Corner base yards (6x6 quadrants) in each player's color
    static const Color baseColors[4] = {RED, BLUE, GREEN, YELLOW};
    static const int qr[4] = {0, 0, 9, 9}; // quadrant row offsets (Red=TL, Blue=TR, Green=BR, Yellow=BL)
    static const int qc[4] = {0, 9, 9, 0};
    for (int p = 0; p < MAX_PLAYERS; p++) {
        Color c = baseColors[p];
        DrawRectangle((int)(LUDO_X + qc[p] * LUDO_CELL), (int)(LUDO_Y + qr[p] * LUDO_CELL),
                      (int)(6 * LUDO_CELL), (int)(6 * LUDO_CELL),
                      (Color){c.r, c.g, c.b, 60});
    }

    // Center 3x3 finish square
    DrawRectangle((int)(LUDO_X + 6 * LUDO_CELL), (int)(LUDO_Y + 6 * LUDO_CELL),
                  (int)(3 * LUDO_CELL), (int)(3 * LUDO_CELL), (Color){230, 226, 210, 255});
    DrawRectangleLinesEx((Rectangle){LUDO_X + 6 * LUDO_CELL, LUDO_Y + 6 * LUDO_CELL,
                         3 * LUDO_CELL, 3 * LUDO_CELL}, 2, (Color){80, 70, 50, 255});

    // Shared 52-square track
    for (int i = 0; i < BOARD_SIZE; i++) {
        BoardSquare *sq = &g->board[i];
        Vector2 c = sq->screenPos;
        Rectangle r = {c.x - LUDO_CELL/2, c.y - LUDO_CELL/2, LUDO_CELL, LUDO_CELL};
        DrawRectangleRec(r, square_color(sq->type));
        DrawRectangleLinesEx(r, 1, (Color){100, 90, 70, 255});

        char id[4];
        sprintf(id, "%d", sq->id);
        int fs = (sq->id < 10) ? 10 : 8;
        DrawText(id, (int)(c.x - fs/2), (int)(c.y - 8), fs, (Color){80, 70, 50, 170});
    }

    // Home lanes (private, 6 cells per player, inward toward center)
    for (int p = 0; p < MAX_PLAYERS; p++) {
        Color c = baseColors[p];
        for (int i = 0; i < HOME_STEPS; i++) {
            Vector2 hc = g->homeLanePos[p][i];
            Rectangle r = {hc.x - LUDO_CELL/2, hc.y - LUDO_CELL/2, LUDO_CELL, LUDO_CELL};
            DrawRectangleRec(r, c);
            DrawRectangleLinesEx(r, 1, (Color){10, 10, 15, 255});
        }
    }

    // Base yards: label each corner quadrant with the player's initial
    static const char* baseNames[4] = {"R", "B", "G", "Y"};
    for (int p = 0; p < MAX_PLAYERS; p++) {
        float minx = 1e9f, miny = 1e9f, maxx = -1e9f, maxy = -1e9f;
        for (int i = 0; i < TOKENS_PER_PLAYER; i++) {
            Vector2 c = g->basePos[p][i];
            if (c.x < minx) minx = c.x;
            if (c.x > maxx) maxx = c.x;
            if (c.y < miny) miny = c.y;
            if (c.y > maxy) maxy = c.y;
        }
        DrawRectangle((int)(minx - LUDO_CELL/2 - 4), (int)(miny - LUDO_CELL/2 - 4),
                      (int)(maxx - minx + LUDO_CELL + 8), (int)(maxy - miny + LUDO_CELL + 8),
                      (Color){baseColors[p].r, baseColors[p].g, baseColors[p].b, 70});
        DrawRectangleLines((int)(minx - LUDO_CELL/2 - 4), (int)(miny - LUDO_CELL/2 - 4),
                           (int)(maxx - minx + LUDO_CELL + 8), (int)(maxy - miny + LUDO_CELL + 8), baseColors[p]);
        DrawText(baseNames[p], (int)(minx - 5), (int)(maxy + 4), 14, baseColors[p]);
    }

    // Tokens
    int drawn[BOARD_SIZE];
    for (int i = 0; i < BOARD_SIZE; i++) drawn[i] = 0;
    Vector2 center = {LUDO_X + 7.5f * LUDO_CELL, LUDO_Y + 7.5f * LUDO_CELL};

    for (int p = 0; p < g->playerCount; p++) {
        Player *pl = &g->players[p];
        for (int k = 0; k < TOKENS_PER_PLAYER; k++) {
            Token *t = &pl->tokens[k];
            Vector2 pos;
            if (t->state == TOKEN_BASE) {
                pos = g->basePos[p][k];
            } else if (t->state == TOKEN_FINISHED) {
                pos = (Vector2){center.x + (k % 2) * 16 - 8, center.y + (k / 2) * 16 - 8};
            } else if (t->state == TOKEN_HOME) {
                pos = g->homeLanePos[p][t->progress - SHARED_TRACK_STEPS - 1];
            } else {
                int sq = GetSharedBoardSquare(p, t->progress);
                int idx = sq - 1;
                pos = g->board[idx].screenPos;
                int off = drawn[idx]++;
                pos.x += (off % 2) * 16 - 8;
                pos.y += (off / 2) * 16 - 8;
            }
            draw_token_small(g, t, pl, pos);
        }
    }
}

void board_draw(Game *g) {
    if (g->mode == MODE_CLASSIC) {
        draw_classic_board(g);
        return;
    }

    // Ladder mode: existing 30-square serpentine board
    for (int i = 0; i < BOARD_SQUARES; i++) {
        BoardSquare *sq = &g->board[i];
        Vector2 c = sq->screenPos;
        Rectangle r = {c.x - CELL_SIZE/2, c.y - CELL_SIZE/2, CELL_SIZE, CELL_SIZE};
        Color bg = square_color(sq->type);
        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, 2, (Color){100, 90, 70, 255});

        char id[4];
        sprintf(id, "%d", sq->id);
        int fs = (sq->id < 10) ? 20 : 16;
        DrawText(id, (int)(c.x - fs/2), (int)(c.y - fs/2 - 12), fs, (Color){80, 70, 50, 180});

        const char *label = square_label(sq->type);
        if (label[0]) {
            int ls = 12;
            DrawText(label, (int)(c.x - MeasureText(label, ls)/2), (int)(c.y + 10), ls, (Color){60, 50, 40, 200});
        }
    }

    for (int p = 0; p < g->playerCount; p++) {
        Player *pl = &g->players[p];
        if (pl->finished || pl->position == 0) continue;
        int idx = pl->position - 1;
        if (idx < 0 || idx >= BOARD_SQUARES) continue;
        Vector2 pos = g->board[idx].screenPos;

        int offset_x = (p % 2 == 0) ? -15 : 15;
        int offset_y = (p < 2) ? -15 : 15;
        float tx = pos.x + offset_x;
        float ty = pos.y + offset_y;

        Texture2D spr = g->pokeSprites[pl->pokemon.type];
        if (spr.id > 0) {
            float scale = 60.0f / spr.width;
            DrawTextureEx(spr, (Vector2){tx - 30, ty - 30}, 0, scale, WHITE);
        } else {
            DrawCircleV((Vector2){tx, ty}, 30, pl->color);
            DrawCircleLinesV((Vector2){tx, ty}, 30, BLACK);
        }
        DrawText(pl->name, (int)(tx - 8), (int)(ty + 16), 10, pl->color);
    }
}

void board_draw_hud(Game *g) {
    int panelX = WINDOW_W - 250;
    int panelY = 20;

    if (g->mode == MODE_CLASSIC) {
        int ph = g->playerCount * 92 + 40;
        DrawRectangle(panelX, panelY, 230, ph, (Color){20, 20, 40, 200});
        DrawRectangleLines(panelX, panelY, 230, ph, (Color){80, 80, 100, 255});
        DrawText("PLAYERS", panelX + 10, panelY + 10, 14, (Color){180, 180, 200, 255});

        for (int i = 0; i < g->playerCount; i++) {
            int y = panelY + 35 + i * 92;
            if (i == g->currentPlayer && g->state == STATE_PLAYING) {
                DrawRectangle(panelX + 5, y - 5, 220, 82, (Color){40, 40, 60, 255});
            }
            DrawText(g->players[i].name, panelX + 10, y, 16, g->players[i].color);

            char typeBuf[32];
            sprintf(typeBuf, "%s", poke_type_name(g->players[i].tokens[0].pokemon.type));
            DrawText(typeBuf, panelX + 10, y + 22, 12, g->players[i].tokens[0].pokemon.color);

            char homeBuf[32];
            sprintf(homeBuf, "Home: %d/%d", g->players[i].finishedCount, TOKENS_PER_PLAYER);
            DrawText(homeBuf, panelX + 10, y + 42, 12, WHITE);

            char winBuf[32];
            sprintf(winBuf, "Wins: %d", g->players[i].wins);
            DrawText(winBuf, panelX + 10, y + 62, 12, (Color){180, 180, 200, 255});
        }
        return;
    }

    // Ladder mode: existing HUD with per-player HP bars
    DrawRectangle(panelX, panelY, 230, g->playerCount * 80 + 40, (Color){20, 20, 40, 200});
    DrawRectangleLines(panelX, panelY, 230, g->playerCount * 80 + 40, (Color){80, 80, 100, 255});
    DrawText("PLAYERS", panelX + 10, panelY + 10, 14, (Color){180, 180, 200, 255});

    for (int i = 0; i < g->playerCount; i++) {
        int y = panelY + 35 + i * 80;

        if (i == g->currentPlayer && g->state == STATE_PLAYING) {
            DrawRectangle(panelX + 5, y - 5, 220, 70, (Color){40, 40, 60, 255});
        }

        DrawText(g->players[i].name, panelX + 10, y, 16, g->players[i].color);

        char typeBuf[32];
        sprintf(typeBuf, "%s", poke_type_name(g->players[i].pokemon.type));
        DrawText(typeBuf, panelX + 10, y + 20, 12, g->players[i].pokemon.color);

        DrawRectangle(panelX + 10, y + 38, 150, 10, (Color){40, 40, 60, 255});
        float pct = (float)g->players[i].pokemon.hp / g->players[i].pokemon.maxHp;
        DrawRectangle(panelX + 10, y + 38, (int)(150 * pct), 10,
                      pct > 0.5f ? GREEN : pct > 0.25f ? YELLOW : RED);

        char posBuf[16];
        sprintf(posBuf, "%d/%d", g->players[i].position + 1, BOARD_SQUARES);
        DrawText(posBuf, panelX + 170, y + 34, 12, WHITE);
    }
}

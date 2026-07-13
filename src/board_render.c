#include "game.h"
#include "pokemon.h"
#include <stdio.h>

#define SQ_SIZE 100

static const Color square_colors[] = {
    {40, 40, 60, 255},    // normal
    {30, 60, 30, 255},    // safe
    {60, 30, 30, 255},    // ladder
    {30, 30, 60, 255},    // snake
    {60, 50, 20, 255},    // evolution
    {20, 50, 60, 255},    // habitat
    {50, 30, 60, 255},    // mystery
    {60, 60, 60, 255}     // stone
};

static const char* square_labels[] = {
    "", "SAFE", "", "", "EVO", "HAB", "?", "STR"
};

void board_draw(Game *g) {
    for (int i = 0; i < BOARD_SQUARES; i++) {
        Vector2 pos = g->board[i].screenPos;
        float half = SQ_SIZE / 2.0f - 4;

        DrawRectangle(pos.x - half, pos.y - half, SQ_SIZE - 8, SQ_SIZE - 8,
                      square_colors[g->board[i].type]);
        DrawRectangleLines(pos.x - half, pos.y - half, SQ_SIZE - 8, SQ_SIZE - 8,
                          (Color){80, 80, 100, 255});

        char buf[8];
        sprintf(buf, "%d", i + 1);
        DrawText(buf, pos.x - half + 4, pos.y - half + 4, 12, (Color){150, 150, 170, 255});

        if (g->board[i].type != SQ_NORMAL && square_labels[g->board[i].type][0]) {
            DrawText(square_labels[g->board[i].type], pos.x - half + 4, pos.y + half - 16, 10,
                    (Color){200, 200, 200, 200});
        }
    }

    for (int i = 0; i < BOARD_SQUARES - 1; i++) {
        Vector2 a = g->board[i].screenPos;
        Vector2 b = g->board[i + 1].screenPos;
        DrawLineEx(a, b, 2, (Color){60, 60, 80, 255});
    }

    for (int i = 0; i < g->playerCount; i++) {
        Player *p = &g->players[i];
        if (p->position >= 0 && p->position < BOARD_SQUARES) {
            Vector2 pos = g->board[p->position].screenPos;

            float ox = (i % 2 == 0 ? -12 : 12);
            float oy = (i < 2 ? -12 : 12);

            DrawCircle(pos.x + ox, pos.y + oy, 16, p->pokemon.color);
            DrawCircleLines(pos.x + ox, pos.y + oy, 16, WHITE);

            char buf[16];
            snprintf(buf, sizeof(buf), "P%d", i + 1);
            DrawText(buf, pos.x + ox - 6, pos.y + oy - 6, 12, WHITE);
        }
    }

    Vector2 startPos = g->board[0].screenPos;
    DrawText("START", startPos.x - 20, startPos.y + SQ_SIZE/2 + 5, 14, (Color){0, 230, 118, 255});

    Vector2 homePos = g->board[BOARD_SQUARES - 1].screenPos;
    DrawText("HOME", homePos.x - 18, homePos.y + SQ_SIZE/2 + 5, 14, (Color){255, 202, 40, 255});
}

#include "game.h"
#include "board.h"
#include "board_render.h"
#include <stdio.h>

#define CELL_SIZE 100
#define BOARD_X 60
#define BOARD_Y 140

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

void board_draw(Game *g) {
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
            float scale = 30.0f / spr.width;
            DrawTextureEx(spr, (Vector2){tx - 15, ty - 15}, 0, scale, WHITE);
        } else {
            DrawCircleV((Vector2){tx, ty}, 15, pl->color);
            DrawCircleLinesV((Vector2){tx, ty}, 15, BLACK);
        }
        DrawText(pl->name, (int)(tx - 8), (int)(ty + 16), 10, pl->color);
    }
}

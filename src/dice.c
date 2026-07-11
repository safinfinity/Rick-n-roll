#include "game.h"
#include "dice.h"
#include <stdlib.h>

void dice_roll(Dice *d) {
    d->rolling = true;
    d->rollTimer = 0;
    d->value = 0;
}

void dice_update(Dice *d) {
    if (!d->rolling) return;
    d->rollTimer++;
    d->value = (rand() % 6) + 1;
    if (d->rollTimer >= d->rollDuration) {
        d->rolling = false;
    }
}

void dice_draw(Dice *d, int x, int y, int size) {
    Rectangle bg = {(float)x, (float)y, (float)size, (float)size};
    DrawRectangleRec(bg, WHITE);
    DrawRectangleLinesEx(bg, 3, DARKGRAY);

    if (d->value == 0) {
        DrawText("?", x + size/2 - 10, y + size/2 - 10, 24, DARKGRAY);
        return;
    }

    int dot = size / 12;
    int m = size / 5;
    int cx = x + size / 2;
    int cy = y + size / 2;

    Vector2 dots[7][6] = {
        {},
        {{cx, cy}},
        {{cx-m, cy-m}, {cx+m, cy+m}},
        {{cx-m, cy-m}, {cx, cy}, {cx+m, cy+m}},
        {{cx-m, cy-m}, {cx+m, cy-m}, {cx-m, cy+m}, {cx+m, cy+m}},
        {{cx-m, cy-m}, {cx+m, cy-m}, {cx, cy}, {cx-m, cy+m}, {cx+m, cy+m}},
        {{cx-m, cy-m}, {cx+m, cy-m}, {cx-m, cy}, {cx+m, cy}, {cx-m, cy+m}, {cx+m, cy+m}}
    };

    if (d->value >= 1 && d->value <= 6) {
        for (int i = 0; i < d->value; i++) {
            DrawCircleV(dots[d->value][i], dot, BLACK);
        }
    }
}

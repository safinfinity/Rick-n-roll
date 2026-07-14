#include "game.h"
#include <stdio.h>

void dice_roll(Dice *d) {
    d->value = GetRandomValue(1, 6);
    d->rolling = true;
    d->rollTimer = 0;
    d->rollDuration = 30;
}

void dice_update(Dice *d) {
    if (!d->rolling) return;
    d->rollTimer++;
    if (d->rollTimer >= d->rollDuration) {
        d->rolling = false;
    }
}

void dice_draw(Dice *d, int x, int y) {
    int size = d->rolling ? 100 : 80;
    int cx = x + size / 2;
    int cy = y + size / 2;

    if (d->rolling) {
        int pulse = (d->rollTimer * 8) % 255;
        DrawRectangleLinesEx((Rectangle){(float)x - 6, (float)y - 6, (float)size + 12, (float)size + 12},
                            4, (Color){255, 202, 40, (unsigned char)pulse});
    }

    DrawRectangle(x, y, size, size, (Color){40, 40, 60, 255});
    DrawRectangleLines(x, y, size, size, (Color){255, 202, 40, 255});

    int displayVal = d->rolling ? GetRandomValue(1, 6) : d->value;
    Color dotColor = (Color){255, 202, 40, 255};

    typedef struct { int dx, dy; } Dot;
    int dSize = size / 80;
    static const Dot dots[7][9] = {
        {},
        {{0,0}},
        {{-12,-12},{12,12}},
        {{-12,-12},{0,0},{12,12}},
        {{-12,-12},{12,-12},{-12,12},{12,12}},
        {{-12,-12},{12,-12},{0,0},{-12,12},{12,12}},
        {{-12,-12},{12,-12},{-12,0},{12,0},{-12,12},{12,12}}
    };

    int count = (displayVal >= 1 && displayVal <= 6) ? displayVal : 0;
    for (int i = 0; i < count; i++) {
        DrawCircle(cx + dots[displayVal][i].dx * dSize, cy + dots[displayVal][i].dy * dSize, 6 * dSize, dotColor);
    }

    int tx = x + (d->rolling ? 28 : 35);
    int ty = y + size + 5;
    if (d->rolling) {
        DrawText("Rolling...", tx - 15, ty, 14, (Color){255, 202, 40, (unsigned char)(128 + (d->rollTimer * 4) % 128)});
    } else {
        char buf[8];
        sprintf(buf, "%d", displayVal);
        DrawText(buf, tx - 5, ty, 14, (Color){180, 180, 200, 255});
    }
}

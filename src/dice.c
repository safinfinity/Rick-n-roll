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
    int displayVal = d->rolling ? GetRandomValue(1, 6) : d->value;

    DrawRectangle(x, y, 80, 80, (Color){40, 40, 60, 255});
    DrawRectangleLines(x, y, 80, 80, (Color){255, 202, 40, 255});

    int cx = x + 40;
    int cy = y + 40;
    int dotR = 6;
    Color dotColor = d->rolling ? (Color){200, 200, 200, 255} : (Color){255, 202, 40, 255};

    typedef struct { int dx, dy; } Dot;
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
        DrawCircle(cx + dots[displayVal][i].dx, cy + dots[displayVal][i].dy, dotR, dotColor);
    }

    char buf[8];
    sprintf(buf, "%d", displayVal);
    DrawText(buf, x + 35, y + 85, 14, (Color){180, 180, 200, 255});
}

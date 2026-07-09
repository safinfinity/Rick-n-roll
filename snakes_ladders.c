#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define COLS 10
#define ROWS 10
#define CELL 64
#define BOARD_X 40
#define BOARD_Y 90
#define WIN_W (COLS * CELL + BOARD_X * 2)
#define WIN_H (ROWS * CELL + BOARD_Y + 160)

static const int snake_from[] = {99, 70, 52, 25, 95};
static const int snake_to[]   = {54, 38, 11,  4, 75};
static const int ladder_from[] = { 3,  8, 20, 48, 59, 72, 78};
static const int ladder_to[]   = {22, 30, 42, 67, 86, 91, 98};

static Vector2 SquareCenter(int sq) {
    int row = (sq - 1) / COLS;
    int col = (sq - 1) % COLS;
    if (row % 2 == 1) col = 9 - col;
    float x = BOARD_X + col * CELL + CELL / 2.0f;
    float y = BOARD_Y + (9 - row) * CELL + CELL / 2.0f;
    return (Vector2){x, y};
}

static int CheckSnakeLadder(int pos) {
    for (int i = 0; i < 5; i++)
        if (snake_from[i] == pos) return snake_to[i];
    for (int i = 0; i < 7; i++)
        if (ladder_from[i] == pos) return ladder_to[i];
    return -1;
}

int main(void) {
    SetRandomSeed((unsigned int)time(NULL));
    InitWindow(WIN_W, WIN_H, "Snakes & Ladders");
    SetTargetFPS(60);

    int humanPos = 1, cpuPos = 1;
    int targetPos = 1;
    int currentStep = 0;      // how many steps taken this turn
    int needStep = 0;         // total steps needed this turn
    int diceVal = 0;
    int timer = 0;
    int winner = -1;
    int slFlash = 0;          // snake/ladder flash timer
    int slFrom = 0, slTo = 0; // snake/ladder positions for flash message

    // 0=waiting to roll, 1=human rolling, 2=human moving, 3=cpu thinking, 4=cpu rolling, 5=cpu moving, 6=gameover
    int state = 0;

    Rectangle rollBtn = { WIN_W/2.0f - 60, WIN_H - 130, 120, 45 };

    while (!WindowShouldClose()) {
        timer++;

        // ============== STATE LOGIC ==============
        switch (state) {
        case 0: // waiting for human to roll
            slFlash = 0;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), rollBtn)) {
                diceVal = GetRandomValue(1, 6);
                needStep = diceVal;
                if (humanPos + needStep > 100) needStep = 100 - humanPos;
                currentStep = 0;
                targetPos = humanPos + needStep;
                state = 1;
                timer = 0;
            }
            break;

        case 1: // human rolling animation (show dice briefly)
            if (timer > 10) { state = 2; timer = 0; }
            break;

        case 2: // human moving
            if (timer % 6 == 0 && currentStep < needStep) {
                currentStep++;
                humanPos++;
            }
            if (currentStep >= needStep) {
                humanPos = targetPos;
                int sl = CheckSnakeLadder(humanPos);
                if (sl != -1) {
                    slFrom = humanPos;
                    slTo = sl;
                    humanPos = sl;
                    slFlash = 30;
                }
                if (humanPos == 100) { winner = 0; state = 6; }
                else { state = 3; timer = 0; }
            }
            break;

        case 3: // cpu thinking
            if (timer > 40) {
                diceVal = GetRandomValue(1, 6);
                needStep = diceVal;
                if (cpuPos + needStep > 100) needStep = 100 - cpuPos;
                currentStep = 0;
                targetPos = cpuPos + needStep;
                state = 4;
                timer = 0;
            }
            break;

        case 4: // cpu rolling animation
            if (timer > 10) { state = 5; timer = 0; }
            break;

        case 5: // cpu moving
            if (timer % 6 == 0 && currentStep < needStep) {
                currentStep++;
                cpuPos++;
            }
            if (currentStep >= needStep) {
                cpuPos = targetPos;
                int sl = CheckSnakeLadder(cpuPos);
                if (sl != -1) {
                    slFrom = cpuPos;
                    slTo = sl;
                    cpuPos = sl;
                    slFlash = 30;
                }
                if (cpuPos == 100) { winner = 1; state = 6; }
                else { state = 0; }
            }
            break;

        case 6: // game over
            if (IsKeyPressed(KEY_R)) {
                humanPos = 1; cpuPos = 1; winner = -1;
                diceVal = 0; state = 0; timer = 0; slFlash = 0;
            }
            break;
        }

        // ============== RENDERING ==============
        BeginDrawing();
        ClearBackground((Color){200, 220, 240, 255});

        // Title
        DrawText("Snakes & Ladders", WIN_W/2 - MeasureText("Snakes & Ladders", 28)/2, 15, 28, DARKBLUE);

        // Board cells
        for (int i = 1; i <= 100; i++) {
            Vector2 c = SquareCenter(i);
            Rectangle r = { c.x - CELL/2, c.y - CELL/2, CELL, CELL };
            int row = (i - 1) / COLS;
            int col = (i - 1) % COLS;
            Color bg = ((row + col) % 2 == 0) ? (Color){240, 235, 210, 255} : (Color){225, 210, 185, 255};
            DrawRectangleRec(r, bg);
            DrawRectangleLinesEx(r, 1, (Color){160, 140, 110, 255});
            char buf[8];
            sprintf(buf, "%d", i);
            int fs = (i < 10) ? 14 : (i < 100) ? 12 : 10;
            Vector2 ts = MeasureTextEx(GetFontDefault(), buf, (float)fs, 1);
            DrawText(buf, (int)(c.x - ts.x/2), (int)(c.y - ts.y/2), fs, (Color){90, 70, 50, 200});
        }

        // Ladders (green)
        for (int i = 0; i < 7; i++) {
            Vector2 a = SquareCenter(ladder_from[i]);
            Vector2 b = SquareCenter(ladder_to[i]);
            DrawLineEx(a, b, 5, (Color){30, 170, 30, 220});
            for (int j = 1; j < 6; j++) {
                Vector2 p = { a.x + (b.x - a.x) * j / 6.0f, a.y + (b.y - a.y) * j / 6.0f };
                DrawLineEx((Vector2){p.x - 5, p.y - 5}, (Vector2){p.x + 5, p.y + 5}, 3, (Color){40, 190, 40, 220});
                DrawLineEx((Vector2){p.x + 5, p.y - 5}, (Vector2){p.x - 5, p.y + 5}, 3, (Color){40, 190, 40, 220});
            }
        }

        // Snakes (red) - wavy zigzag - drawn on top
        for (int i = 0; i < 5; i++) {
            Vector2 a = SquareCenter(snake_from[i]);
            Vector2 b = SquareCenter(snake_to[i]);
            Vector2 dir = { b.x - a.x, b.y - a.y };
            float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
            float nx = -dir.y / len;
            float ny = dir.x / len;
            int segments = 24;
            Vector2 prev = { a.x + dir.x * 0 / segments,
                             a.y + dir.y * 0 / segments };
            for (int j = 1; j <= segments; j++) {
                float t = (float)j / segments;
                float wave = sinf(t * 6.28f * 2.5f) * 12.0f;
                Vector2 p = { a.x + dir.x * t + nx * wave,
                              a.y + dir.y * t + ny * wave };
                DrawLineEx(prev, p, 10, (Color){200, 25, 25, 255});
                // dark outline for depth
                DrawLineEx(prev, p, 12, (Color){140, 10, 10, 100});
                prev = p;
            }
            // zigzag scale pattern on body
            for (int j = 0; j < 12; j++) {
                float t = (j + 0.5f) / 12;
                float wave = sinf(t * 6.28f * 2.5f) * 12.0f;
                float side = (j % 2 == 0) ? 8.0f : -8.0f;
                Vector2 p = { a.x + dir.x * t + nx * (wave + side),
                              a.y + dir.y * t + ny * (wave + side) };
                DrawCircleV(p, 4, (Color){230, 80, 80, 255});
            }
            // snake head
            DrawCircleV(a, 11, (Color){220, 30, 30, 255});
            DrawCircleLinesV(a, 11, (Color){160, 10, 10, 255});
            // two eyes
            float eh = nx * 5, ev = ny * 5;
            DrawCircleV((Vector2){a.x - dir.x/len*4 + eh, a.y - dir.y/len*4 + ev}, 3, WHITE);
            DrawCircleV((Vector2){a.x - dir.x/len*4 - eh, a.y - dir.y/len*4 - ev}, 3, WHITE);
            DrawCircleV((Vector2){a.x - dir.x/len*4 + eh, a.y - dir.y/len*4 + ev}, 1.5f, BLACK);
            DrawCircleV((Vector2){a.x - dir.x/len*4 - eh, a.y - dir.y/len*4 - ev}, 1.5f, BLACK);
            // tail
            DrawCircleV(b, 6, (Color){180, 20, 20, 255});
        }

        // Animate snake/ladder flash
        if (slFlash > 0) {
            Vector2 from = SquareCenter(slFrom);
            Vector2 to = SquareCenter(slTo);
            float t = (float)slFlash / 30.0f;
            Color flashColor = (slTo < slFrom) ? RED : GREEN;
            DrawLineEx(from, to, 6 + (int)(6 * t), Fade(flashColor, t));
            char msg[32];
            sprintf(msg, slTo < slFrom ? "SNAKE! %d->%d" : "LADDER! %d->%d", slFrom, slTo);
            DrawText(msg, WIN_W/2 - MeasureText(msg, 22)/2, WIN_H - 160, 22, flashColor);
            slFlash--;
        }

        // Player tokens
        Vector2 hp = SquareCenter(humanPos);
        Vector2 cp = SquareCenter(cpuPos);

        // Highlight current player
        if (state == 0 || state == 1 || state == 2) {
            DrawRectangleLinesEx((Rectangle){hp.x - 20, hp.y - 20, 40, 40}, 2, YELLOW);
        }
        if (state == 3 || state == 4 || state == 5) {
            DrawRectangleLinesEx((Rectangle){cp.x - 20, cp.y - 20, 40, 40}, 2, YELLOW);
        }

        DrawCircleV(hp, 14, RED);
        DrawCircleLinesV(hp, 14, MAROON);
        DrawCircleV(cp, 14, BLUE);
        DrawCircleLinesV(cp, 14, DARKBLUE);
        DrawText("You", (int)hp.x + 18, (int)hp.y - 7, 13, RED);
        DrawText("CPU", (int)cp.x + 18, (int)cp.y - 7, 13, BLUE);

        // Bottom panel
        DrawRectangleRec(rollBtn, (state == 0) ? (Color){220, 220, 220, 255} : (Color){180, 180, 180, 255});
        DrawRectangleLinesEx(rollBtn, 2, DARKGRAY);
        const char *btnText = (state == 0) ? "ROLL" : "...";
        DrawText(btnText, (int)(rollBtn.x + rollBtn.width/2 - MeasureText(btnText, 22)/2),
                 (int)(rollBtn.y + rollBtn.height/2 - 11), 22, DARKGRAY);

        // Dice value
        if (diceVal > 0) {
            char buf[32];
            sprintf(buf, "Rolled: %d", diceVal);
            DrawText(buf, WIN_W/2 - MeasureText(buf, 18)/2, WIN_H - 75, 18, DARKBLUE);
        }

        // Status
        if (state != 6) {
            const char *s = "";
            if (state == 0) s = "Your turn - Click ROLL";
            else if (state == 1) s = "You rolled!";
            else if (state == 2) s = "Moving...";
            else if (state == 3) s = "Computer thinking...";
            else if (state == 4) s = "Computer rolled!";
            else if (state == 5) s = "Computer moving...";
            DrawText(s, WIN_W/2 - MeasureText(s, 16)/2, WIN_H - 35, 16, (state < 3) ? DARKGREEN : DARKBLUE);
        } else {
            char buf[64];
            sprintf(buf, "%s wins! Press R to restart", winner == 0 ? "You" : "Computer");
            DrawText(buf, WIN_W/2 - MeasureText(buf, 22)/2, WIN_H - 90, 22, DARKGREEN);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

#include "game.h"
#include "menu.h"
#include "pokemon.h"
#include <string.h>
#include <stdio.h>
static Vector2 GetVirtualMousePosition(void)
{
    // Convert the real mouse position to the logical 1200x800 game space.
    // This matches the Camera2D transform used by main.c.
    Vector2 mouse = GetMousePosition();

    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();

    float scaleX = screenW / (float)WINDOW_W;
    float scaleY = screenH / (float)WINDOW_H;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    float offsetX = (screenW - WINDOW_W * scale) / 2.0f;
    float offsetY = (screenH - WINDOW_H * scale) / 2.0f;

    return (Vector2){
        (mouse.x - offsetX) / scale,
        (mouse.y - offsetY) / scale
    };
}

static Rectangle classicBtn = {WINDOW_W/2 - 150, 280, 300, 60};
static Rectangle ladderBtn  = {WINDOW_W/2 - 150, 370, 300, 60};
static Rectangle p2Btn      = {WINDOW_W/2 - 180, 260, 160, 50};
static Rectangle p3Btn      = {WINDOW_W/2 + 20, 260, 160, 50};
static Rectangle p4Btn      = {WINDOW_W/2 - 80,  340, 160, 50};

void menu_init(void) {
}

int menu_update(Game *g) {
    Vector2 mouse = GetVirtualMousePosition();

    if (g->state == STATE_MENU) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, classicBtn)) {
                g->mode = MODE_CLASSIC;
                return MENU_MODE_PICKED;
            }
            if (CheckCollisionPointRec(mouse, ladderBtn)) {
                g->mode = MODE_LADDER;
                return MENU_MODE_PICKED;
            }
        }
    }

    if (g->state == STATE_PLAYER_COUNT) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, p2Btn)) return 2;
            if (CheckCollisionPointRec(mouse, p3Btn)) return 3;
            if (CheckCollisionPointRec(mouse, p4Btn)) return 4;
        }
    }

    if (g->state == STATE_GAME_OVER) {
        if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER)) {
            game_reset(g);
            return STATE_MENU;
        }
    }

    return MENU_NO_ACTION;
}

void menu_draw(Game *g) {
    if (g->state == STATE_MENU) {
        DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){15, 15, 35, 255});

        DrawText("Rick N Roll", WINDOW_W/2 - MeasureText("Rick N Roll", 60)/2, 100, 60, GOLD);
        DrawText("Pokemon + Ludo Board Game", WINDOW_W/2 - MeasureText("Pokemon + Ludo Board Game", 22)/2, 175, 22, LIGHTGRAY);

        bool hoverC = CheckCollisionPointRec(GetVirtualMousePosition(), classicBtn);
        DrawRectangleRec(classicBtn, hoverC ? (Color){80, 50, 50, 255} : (Color){60, 30, 30, 255});
        DrawRectangleLinesEx(classicBtn, 2, GOLD);
        DrawText("CLASSIC MODE", (int)(classicBtn.x + 40), (int)(classicBtn.y + 15), 24, WHITE);

        bool hoverL = CheckCollisionPointRec(GetVirtualMousePosition(), ladderBtn);
        DrawRectangleRec(ladderBtn, hoverL ? (Color){50, 80, 50, 255} : (Color){30, 60, 30, 255});
        DrawRectangleLinesEx(ladderBtn, 2, GOLD);
        DrawText("LADDER MODE", (int)(ladderBtn.x + 40), (int)(ladderBtn.y + 15), 24, WHITE);

        DrawText("Classic: Battle opponents & reach home", WINDOW_W/2 - 200, 470, 16, GRAY);
        DrawText("Ladder: Capture Pokemon & evolve (WIP)", WINDOW_W/2 - 200, 495, 16, GRAY);
    }

    if (g->state == STATE_PLAYER_COUNT) {
        DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){15, 15, 35, 255});
        DrawText("SELECT PLAYERS", WINDOW_W/2 - MeasureText("SELECT PLAYERS", 36)/2, 120, 36, GOLD);
        DrawText("How many players?", WINDOW_W/2 - MeasureText("How many players?", 20)/2, 180, 20, LIGHTGRAY);

        Rectangle btns[] = {p2Btn, p3Btn, p4Btn};
        const char* labels[] = {"2 Players", "3 Players", "4 Players"};
        for (int i = 0; i < 3; i++) {
            bool hover = CheckCollisionPointRec(GetVirtualMousePosition(), btns[i]);
            DrawRectangleRec(btns[i], hover ? (Color){60, 60, 100, 255} : (Color){40, 40, 70, 255});
            DrawRectangleLinesEx(btns[i], 2, GOLD);
            DrawText(labels[i], (int)(btns[i].x + 20), (int)(btns[i].y + 13), 18, WHITE);
        }
    }

    if (g->state == STATE_GAME_OVER) {
        DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){15, 15, 35, 220});

        int winner = -1;
        for (int i = 0; i < g->playerCount; i++) {
            if (g->players[i].finished && g->players[i].finishOrder == 1) {
                winner = i;
                break;
            }
        }
        if (winner == -1) {
            for (int i = 0; i < g->playerCount; i++) {
                if (g->players[i].position == BOARD_SQUARES) {
                    winner = i;
                    break;
                }
            }
        }

        if (winner >= 0) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%s WINS!", g->players[winner].name);
            DrawText(buf, WINDOW_W/2 - MeasureText(buf, 48)/2, 250, 48, GOLD);
            DrawText(TextFormat("Pokemon: %s", g->players[winner].pokemon.name),
                     WINDOW_W/2 - MeasureText(TextFormat("Pokemon: %s", g->players[winner].pokemon.name), 22)/2, 330, 22, WHITE);
        } else {
            DrawText("GAME OVER", WINDOW_W/2 - MeasureText("GAME OVER", 48)/2, 250, 48, GOLD);
        }

        DrawText("Press R or ENTER to restart", WINDOW_W/2 - MeasureText("Press R or ENTER to restart", 20)/2, 450, 20, LIGHTGRAY);
    }
}

#include "game.h"
#include "menu.h"
#include "pokemon.h"
#include <string.h>// use korinai actually
#include <stdio.h>
static Vector2 GetVirtualMousePosition(void)
{
    // Convert the real mouse position to the logical 1200x800 game space.
    // This matches the Camera2D transform used by main.c.
    Vector2 mouse = GetMousePosition();

    float screenW = (float)GetScreenWidth();//this function returns acreen width
    float screenH = (float)GetScreenHeight();//returns height

    float scaleX = screenW / (float)WINDOW_W;
    float scaleY = screenH / (float)WINDOW_H;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    float offsetX = (screenW - WINDOW_W * scale) / 2.0f;//horizontal empty space
    float offsetY = (screenH - WINDOW_H * scale) / 2.0f;// vertical empty spcae

    return (Vector2){ //returns a vector2 data type value instead of float and this return(vector2) creates one vector2
        (mouse.x - offsetX) / scale,
        (mouse.y - offsetY) / scale
    };
}
// why -150? we want to show classic and ladder at teh center of width 1200, and its own width is 300 so 300/2
static Rectangle classicBtn = {WINDOW_W/2 - 150, 280, 300, 60};   //{x,y,width,height}
static Rectangle ladderBtn  = {WINDOW_W/2 - 150, 370, 300, 60};
static Rectangle p2Btn      = {WINDOW_W/2 - 180, 260, 160, 50};
static Rectangle p3Btn      = {WINDOW_W/2 + 20, 260, 160, 50};
static Rectangle p4Btn      = {WINDOW_W/2 - 80,  340, 160, 50};

void menu_init(void) {// calling this from header. its a placeholder we kept for adding some features but later decided to not, eg, things popping up in a dynamic way one by one
}// why is it not static? cus its not only private property of menu.c, main.c maight also have it

int menu_update(Game *g) {
    Vector2 mouse = GetVirtualMousePosition();

    if (g->state == STATE_MENU) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON/*raylib const*/)) {// used to check mouse input
            if (CheckCollisionPointRec(mouse, classicBtn)) { // check kortesi classic mode click korse naki ladder
                g->mode = MODE_CLASSIC;
                return MENU_MODE_PICKED;//returns num 100. why? game.h
            }
            if (CheckCollisionPointRec(mouse, ladderBtn)) {
                g->mode = MODE_LADDER;
                return MENU_MODE_PICKED;
            }
        }
    }

    if (g->state == STATE_PLAYER_COUNT) {//player count screen theke info niye main.c te pathabe
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, p2Btn)) return 2;//eei shob info menu update er thru te main.c te jabe
            if (CheckCollisionPointRec(mouse, p3Btn)) return 3;
            if (CheckCollisionPointRec(mouse, p4Btn)) return 4;
        }
    }

    if (g->state == STATE_GAME_OVER) {
        if (IsKeyPressed(KEY_R/*raylib's built in constant*/) || IsKeyPressed(KEY_ENTER)) {
            game_reset(g);
            return STATE_MENU;
        }
    }

    return MENU_NO_ACTION; // incase kichhui pick kori nai, oitao main.c k janano lagbe
}

void menu_draw(Game *g) {
    if (g->state == STATE_MENU) {
        //DrawRectangle(x, y, width, height, color);
        DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){/*15*/255, /*15*/127, /*35*/127, 255});

        //DrawText(text, x, y, fontSize, color);
        DrawText("Rick N Roll", WINDOW_W/2 - MeasureText("Rick N Roll", 60)/2, 100, 60, GOLD);
        DrawText("Pokemon + Ludo Board Game", WINDOW_W/2 - MeasureText("Pokemon + Ludo Board Game", 22)/2, 175, 22, WHITE);

        //checks if hovering over classic button
        bool hoverC = CheckCollisionPointRec(GetVirtualMousePosition(), classicBtn);//taking mouse ner pos and clscbtn er
        DrawRectangleRec(classicBtn, hoverC ? (Color){101, 195, 219, 255} : (Color){39, 138, 165, 255});//drawRectangleRec(classicBtn,colour) this takes a whole rectangle variable
        DrawRectangleLinesEx(classicBtn, 2, PURPLE);//draws a golden border
        DrawText("CLASSIC MODE", (int)(classicBtn.x + 40), (int)(classicBtn.y + 15), 24, WHITE);

        bool hoverL = CheckCollisionPointRec(GetVirtualMousePosition(), ladderBtn);
        DrawRectangleRec(ladderBtn, hoverL ? (Color){215, 169, 227, 255} : (Color){167, 139, 199, 255});
        DrawRectangleLinesEx(ladderBtn, 2, GOLD);// 2 pixel thickness
        DrawText("LADDER MODE", (int)(ladderBtn.x + 40), (int)(ladderBtn.y + 15), 24, WHITE);

        DrawText("Classic: Battle opponents & reach home", WINDOW_W/2 - 200, 470, 16, BLACK);
        DrawText("Ladder: Capture Pokemon & evolve (WIP)", WINDOW_W/2 - 200, 495, 16, BLACK);
    }

    if (g->state == STATE_PLAYER_COUNT) {
        DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){255, 127, 127, 255});
        DrawText("SELECT PLAYERS", WINDOW_W/2 - MeasureText("SELECT PLAYERS", 36)/2, 120, 36, WHITE);
        DrawText("How many players?", WINDOW_W/2 - MeasureText("How many players?", 20)/2, 180, 20, BLACK);

        Rectangle btns[] = {p2Btn, p3Btn, p4Btn};
        const char* labels[] = {"2 Players", "3 Players", "4 Players"};
        for (int i = 0; i < 3; i++) {
            bool hover = CheckCollisionPointRec(GetVirtualMousePosition(), btns[i]);
            DrawRectangleRec(btns[i], hover ? (Color){215, 169, 227, 255} : (Color){167, 139, 199, 255});
            DrawRectangleLinesEx(btns[i], 2, GOLD);
            DrawText(labels[i], (int)(btns[i].x + 20), (int)(btns[i].y + 13), 18, WHITE);
        }
    }

    
}

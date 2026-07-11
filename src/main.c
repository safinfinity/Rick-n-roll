#include "raylib.h"
#include "game.h"
#include "pokemon.h"
#include "board.h"
#include "board_render.h"
#include "dice.h"
#include "battle.h"
#include "menu.h"

int main(void) {
    InitWindow(WINDOW_W, WINDOW_H, "Poku-Doku");
    SetTargetFPS(60);

    Game game = {0};
    game_init(&game);

    while (!WindowShouldClose()) {
        // TODO: update game state

        BeginDrawing();
        ClearBackground((Color){15, 15, 30, 255});
        // TODO: draw game
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

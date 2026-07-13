#include "raylib.h"
#include "game.h"
#include "board.h"
#include "board_render.h"
#include "dice.h"

int main(void) {
    InitWindow(WINDOW_W, WINDOW_H, "Rick-n-roll");
    SetTargetFPS(60);

    Game game = {0};
    game_init(&game);

    Dice dice = {0};

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE) && !dice.rolling) {
            dice_roll(&dice);
        }

        dice_update(&dice);

        BeginDrawing();
        ClearBackground((Color){15, 15, 30, 255});
        board_draw(&game);
        dice_draw(&dice, WINDOW_W - 120, 20);

        DrawText("Press SPACE to roll dice", WINDOW_W - 250, WINDOW_H - 40, 16, (Color){180, 180, 200, 255});
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

#include "game.h"
#include "pokemon.h"
#include "board.h"
#include "board_render.h"

int main(void) {
    InitWindow(WINDOW_W, WINDOW_H, "Rick-n-roll");
    SetTargetFPS(60);

    Game game = {0};
    game_init(&game);
    game.playerCount = 2;
    poke_assign_random(game.players, game.playerCount);
    board_init(&game);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){15, 15, 30, 255});
        board_draw(&game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

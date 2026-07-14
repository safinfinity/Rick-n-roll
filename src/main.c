#include "raylib.h"
#include "game.h"
#include "board.h"
#include "board_render.h"
#include "battle.h"
#include "dice.h"
#include <stdio.h>

static void load_poke_sprites(Game *g) {
    g->pokeSprites[POKE_FIRE]      = LoadTexture("assets/images/fire.png");
    g->pokeSprites[POKE_WATER]     = LoadTexture("assets/images/water.png");
    g->pokeSprites[POKE_GRASS]     = LoadTexture("assets/images/grass.png");
    g->pokeSprites[POKE_ELECTRIC]  = LoadTexture("assets/images/electric.png");
    g->pokeSprites[POKE_PSYCHIC]   = LoadTexture("assets/images/psychic.png");
    g->pokeSprites[POKE_DRAGON]    = LoadTexture("assets/images/dragon.png");
}

static void unload_poke_sprites(Game *g) {
    for (int i = 1; i < 7; i++) {
        UnloadTexture(g->pokeSprites[i]);
    }
}

int main(void) {
    InitWindow(WINDOW_W, WINDOW_H, "Rick-n-roll");
    SetTargetFPS(60);

    Game game = {0};
    game_init(&game);
    load_poke_sprites(&game);

    Dice dice = {0};
    bool wasRolling = false;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE) && !dice.rolling && game.state == STATE_PLAYING) {
            dice_roll(&dice);
        }

        bool diceJustFinished = wasRolling && !dice.rolling;
        wasRolling = dice.rolling;
        dice_update(&dice);

        if (diceJustFinished && game.state == STATE_PLAYING) {
            Player *cur = &game.players[game.currentPlayer];
            if (!cur->finished) {
                int newPos = cur->position + dice.value;
                if (newPos > BOARD_SQUARES) newPos = BOARD_SQUARES;
                cur->position = newPos;

                // Check collision with another player on the same square
                if (cur->position > 0) {
                    for (int i = 0; i < game.playerCount; i++) {
                        if (i != game.currentPlayer &&
                            game.players[i].position == cur->position &&
                            !game.players[i].finished) {
                            // Battle: current player attacks the one already there
                            game.battle.attackerIdx = game.currentPlayer;
                            game.battle.defenderIdx = i;
                            game.battle.rollsLeft = DICE_ROLLS_PER_BATTLE;
                            game.battle.attackerHp = cur->pokemon.hp;
                            game.battle.defenderHp = game.players[i].pokemon.hp;
                            game.battle.attackerMaxHp = cur->pokemon.maxHp;
                            game.battle.defenderMaxHp = game.players[i].pokemon.maxHp;
                            game.battle.finished = false;
                            game.battle.currentRoll = 0;
                            sprintf(game.battle.message, "BATTLE! %s vs %s!",
                                    cur->name, game.players[i].name);
                            game.battle.messageTimer = 60;
                            game.state = STATE_BATTLE;
                            break;
                        }
                    }
                }

                // Check if reached the end
                if (cur->position >= BOARD_SQUARES) {
                    cur->finished = true;
                    cur->finishOrder = 1;
                    for (int i = 0; i < game.playerCount; i++) {
                        if (game.players[i].finished && i != game.currentPlayer) {
                            cur->finishOrder = game.players[i].finishOrder + 1;
                        }
                    }
                }

                // Check if all players finished
                int finishedCount = 0;
                for (int i = 0; i < game.playerCount; i++) {
                    if (game.players[i].finished) finishedCount++;
                }
                if (finishedCount >= game.playerCount) {
                    game.state = STATE_GAME_OVER;
                }
            }

            // Advance to next unfinished player
            if (game.state == STATE_PLAYING) {
                int next = game.currentPlayer;
                do {
                    next = (next + 1) % game.playerCount;
                } while (game.players[next].finished && next != game.currentPlayer);
                game.currentPlayer = next;
            }
        }

        // Handle battle input
        if (game.state == STATE_BATTLE) {
            if (game.battle.messageTimer > 0) game.battle.messageTimer--;
            if (game.battle.finished) {
                if (IsKeyPressed(KEY_SPACE)) {
                    int atk = game.battle.attackerIdx;
                    int def = game.battle.defenderIdx;
                    game.players[atk].pokemon.hp = game.battle.attackerHp;
                    game.players[def].pokemon.hp = game.battle.defenderHp;
                    if (game.battle.attackerWon) {
                        game.players[def].position = 0;
                        game.players[atk].wins++;
                    } else {
                        game.players[atk].position = 0;
                        game.players[def].wins++;
                    }
                    game.state = STATE_PLAYING;
                    int next = game.currentPlayer;
                    do {
                        next = (next + 1) % game.playerCount;
                    } while (game.players[next].finished && next != game.currentPlayer);
                    game.currentPlayer = next;
                }
            } else {
                if (IsKeyPressed(KEY_SPACE)) {
                    battle_roll(&game);
                }
            }
        }

        // Handle game over
        if (game.state == STATE_GAME_OVER) {
            if (IsKeyPressed(KEY_SPACE)) {
                game_init(&game);
                load_poke_sprites(&game);
                dice = (Dice){0};
            }
        }

        BeginDrawing();
        ClearBackground((Color){15, 15, 30, 255});

        if (game.state == STATE_BATTLE) {
            battle_draw(&game);
        } else if (game.state == STATE_GAME_OVER) {
            DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){10, 10, 30, 240});
            const char *title = "GAME OVER";
            DrawText(title, WINDOW_W/2 - MeasureText(title, 48)/2, 200, 48, (Color){255, 202, 40, 255});

            // Find the winner (finishOrder == 1)
            for (int i = 0; i < game.playerCount; i++) {
                if (game.players[i].finishOrder == 1) {
                    char winBuf[128];
                    sprintf(winBuf, "%s wins with %s!", game.players[i].name,
                            game.players[i].pokemon.name);
                    DrawText(winBuf, WINDOW_W/2 - MeasureText(winBuf, 28)/2, 300, 28, game.players[i].color);
                    break;
                }
            }

            // Show all player results
            for (int i = 0; i < game.playerCount; i++) {
                char resBuf[64];
                sprintf(resBuf, "#%d %s (%s) - Wins: %d", game.players[i].finishOrder,
                        game.players[i].name, game.players[i].pokemon.name, game.players[i].wins);
                DrawText(resBuf, WINDOW_W/2 - MeasureText(resBuf, 18)/2, 380 + i * 30, 18,
                         (Color){180, 180, 200, 255});
            }

            DrawText("Press SPACE to play again", WINDOW_W/2 - 160, 550, 20, (Color){255, 202, 40, 255});
        } else {
            board_draw(&game);
            dice_draw(&dice, WINDOW_W - 120, 20);

            // Draw current player info
            Player *cur = &game.players[game.currentPlayer];
            char turnBuf[64];
            sprintf(turnBuf, "%s's turn", cur->name);
            DrawText(turnBuf, 20, 20, 20, cur->color);
            char pokeBuf[64];
            sprintf(pokeBuf, "%s (%s)", cur->pokemon.name, poke_type_name(cur->pokemon.type));
            DrawText(pokeBuf, 20, 48, 16, (Color){180, 180, 200, 255});

            if (!dice.rolling && game.state == STATE_PLAYING) {
                DrawText("Press SPACE to roll", WINDOW_W - 230, WINDOW_H - 30, 16, (Color){255, 202, 40, 255});
            }
        }

        EndDrawing();
    }

    unload_poke_sprites(&game);
    CloseWindow();
    return 0;
}

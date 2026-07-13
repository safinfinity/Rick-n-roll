#include "raylib.h"
#include "game.h"
#include "pokemon.h"
#include "board.h"
#include "board_render.h"
#include "dice.h"
#include "battle.h"
#include "menu.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static char statusText[256] = "";

static void check_square_effect(Game *g, int playerIdx) {
    Player *p = &g->players[playerIdx];
    int pos = p->position;
    if (pos < 1 || pos > BOARD_SQUARES) return;

    int type = board_find_square_type(g, pos);
    switch (type) {
        case SQ_LADDER: {
            int dest = board_get_ladder_dest(pos);
            if (dest > 0 && dest <= BOARD_SQUARES) {
                snprintf(statusText, sizeof(statusText), "LADDER! %s climbs from %d to %d!", p->name, pos, dest);
                p->position = dest;
            }
            break;
        }
        case SQ_SNAKE: {
            int dest = board_get_snake_dest(pos);
            if (dest > 0 && dest <= BOARD_SQUARES) {
                snprintf(statusText, sizeof(statusText), "SNAKE! %s falls from %d to %d!", p->name, pos, dest);
                p->position = dest;
            }
            break;
        }
        case SQ_SAFE:
            snprintf(statusText, sizeof(statusText), "%s rests at a safe spot!", p->name);
            break;
        case SQ_EVOLUTION:
            p->pokemon.atk += 3;
            p->pokemon.def += 2;
            p->pokemon.hp += 20;
            p->pokemon.maxHp += 20;
            snprintf(statusText, sizeof(statusText), "%s's %s evolved! Stats up!", p->name, p->pokemon.name);
            break;
        case SQ_HABITAT:
            p->pokemon.hp = p->pokemon.maxHp;
            snprintf(statusText, sizeof(statusText), "%s fully healed at Habitat!", p->name);
            break;
        case SQ_MYSTERY: {
            int r = rand() % 4;
            switch (r) {
                case 0:
                    p->position += 2;
                    if (p->position > BOARD_SQUARES) p->position = BOARD_SQUARES;
                    snprintf(statusText, sizeof(statusText), "MYSTERY: %s moves +2!", p->name);
                    break;
                case 1:
                    p->position -= 2;
                    if (p->position < 1) p->position = 1;
                    snprintf(statusText, sizeof(statusText), "MYSTERY: %s moves -2!", p->name);
                    break;
                case 2:
                    p->pokemon.atk += 5;
                    snprintf(statusText, sizeof(statusText), "MYSTERY: %s gets +ATK!", p->name);
                    break;
                case 3:
                    p->pokemon.def += 5;
                    snprintf(statusText, sizeof(statusText), "MYSTERY: %s gets +DEF!", p->name);
                    break;
            }
            break;
        }
        case SQ_STONE:
            p->pokemon.atk += 5;
            p->pokemon.def += 5;
            p->pokemon.hp += 30;
            p->pokemon.maxHp += 30;
            snprintf(statusText, sizeof(statusText), "STONE: %s's Pokemon power surged!", p->name);
            break;
        default:
            break;
    }
}

static int find_player_at(Game *g, int position, int exclude) {
    for (int i = 0; i < g->playerCount; i++) {
        if (i == exclude) continue;
        if (g->players[i].position == position && !g->players[i].finished) return i;
    }
    return -1;
}

static void next_turn(Game *g) {
    int attempts = 0;
    do {
        g->currentPlayer = (g->currentPlayer + 1) % g->playerCount;
        attempts++;
        if (attempts > g->playerCount * 2) break;
    } while (g->players[g->currentPlayer].finished);
}

int main(void) {
    InitWindow(WINDOW_W, WINDOW_H, "Rick-n-roll");
    SetTargetFPS(60);

    Game game = {0};
    game_init(&game);
    game.state = STATE_MENU;

    while (!WindowShouldClose()) {
        int result = menu_update(&game);

        if (result >= 2 && result <= 4) {
            game.playerCount = result;
            game.state = STATE_DRAFT;
            for (int i = 0; i < game.playerCount; i++) {
                game.players[i].position = 0;
                game.players[i].finished = false;
                game.players[i].finishOrder = 0;
            }
            poke_assign_random(game.players, game.playerCount);
            game.state = STATE_PLAYING;
            game.currentPlayer = 0;
            game.dice.value = 0;
            game.dice.rolling = false;
            snprintf(statusText, sizeof(statusText), "%s's turn - Click ROLL", game.players[0].name);
        }

        switch (game.state) {
            case STATE_PLAYING: {
                Rectangle rollBtn = {WINDOW_W - 200, WINDOW_H - 100, 160, 50};

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(GetMousePosition(), rollBtn)) {
                        dice_roll(&game.dice);
                        game.state = STATE_ROLLING;
                    }
                }
                break;
            }

            case STATE_ROLLING: {
                dice_update(&game.dice);
                if (!game.dice.rolling) {
                    game.state = STATE_MOVING;
                }
                break;
            }

            case STATE_MOVING: {
                Player *p = &game.players[game.currentPlayer];
                int steps = game.dice.value;
                int prevPos = p->position;
                p->position += steps;
                if (p->position > BOARD_SQUARES) p->position = BOARD_SQUARES;

                snprintf(statusText, sizeof(statusText), "%s rolled %d and moved from %d to %d", p->name, steps, prevPos, p->position);

                int hit = find_player_at(&game, p->position, game.currentPlayer);
                if (hit >= 0) {
                    init_battle(&game, game.currentPlayer, hit);
                    game.state = STATE_BATTLE;
                } else if (p->position == BOARD_SQUARES) {
                    p->finished = true;
                    static int finishCounter = 0;
                    finishCounter++;
                    p->finishOrder = finishCounter;
                    if (finishCounter == 1) {
                        snprintf(statusText, sizeof(statusText), "%s reaches the finish! %s WINS!", p->name, p->name);
                        game.state = STATE_GAME_OVER;
                    } else {
                        snprintf(statusText, sizeof(statusText), "%s finished #%d!", p->name, p->finishOrder);
                        next_turn(&game);
                        game.state = STATE_PLAYING;
                    }
                } else {
                    check_square_effect(&game, game.currentPlayer);
                    next_turn(&game);
                    game.state = STATE_PLAYING;
                }
                break;
            }

            case STATE_BATTLE: {
                battle_update(&game);
                BattleState *b = &game.battle;
                if (b->finished && b->messageTimer <= 0) {
                    game.state = STATE_PLAYING;
                    next_turn(&game);
                }
                break;
            }

            default:
                break;
        }

        BeginDrawing();
        ClearBackground((Color){15, 15, 30, 255});

        switch (game.state) {
            case STATE_MENU:
            case STATE_PLAYER_COUNT:
            case STATE_DRAFT:
                menu_draw(&game);
                break;

            case STATE_PLAYING:
            case STATE_ROLLING:
            case STATE_MOVING: {
                board_draw(&game);

                int pnlX = WINDOW_W - 280;
                DrawRectangle(pnlX - 20, 0, 300, WINDOW_H, (Color){20, 20, 45, 255});
                DrawRectangleLines(pnlX - 20, 0, 300, WINDOW_H, (Color){40, 40, 70, 255});

                Player *cur = &game.players[game.currentPlayer];
                DrawText("TURN", pnlX + 60, 20, 24, GOLD);
                DrawText(cur->name, pnlX + 80, 55, 28, cur->color);

                DrawText("Players:", pnlX + 20, 110, 16, GRAY);
                for (int i = 0; i < game.playerCount; i++) {
                    int y = 140 + i * 50;
                    DrawCircleV((Vector2){pnlX + 35, (float)y + 10}, 10, game.players[i].color);
                    DrawText(game.players[i].name, pnlX + 55, y, 16, game.players[i].color);
                    DrawText(TextFormat("Pos: %d/30", game.players[i].position), pnlX + 55, y + 20, 12, LIGHTGRAY);
                    if (game.players[i].finished) {
                        DrawText("FINISHED", pnlX + 145, y + 20, 12, GREEN);
                    }
                }

                Rectangle rollBtn = {pnlX + 30, 360, 200, 50};
                bool hover = CheckCollisionPointRec(GetMousePosition(), rollBtn);
                DrawRectangleRec(rollBtn, hover ? (Color){80, 80, 120, 255} : (Color){50, 50, 80, 255});
                DrawRectangleLinesEx(rollBtn, 2, GOLD);
                DrawText("ROLL DICE", pnlX + 45, 373, 20, WHITE);

                dice_draw(&game.dice, pnlX + 50, 430, 100);

                if (statusText[0]) {
                    int tw = MeasureText(statusText, 16);
                    DrawRectangle(WINDOW_W/2 - tw/2 - 10, WINDOW_H - 45, tw + 20, 30, (Color){0, 0, 0, 200});
                    DrawRectangleLines(WINDOW_W/2 - tw/2 - 10, WINDOW_H - 45, tw + 20, 30, (Color){60, 60, 100, 255});
                    DrawText(statusText, WINDOW_W/2 - tw/2, WINDOW_H - 40, 16, WHITE);
                }

                DrawFPS(10, 10);
                break;
            }

            case STATE_BATTLE:
                battle_draw(&game);
                break;

            case STATE_GAME_OVER:
                board_draw(&game);
                menu_draw(&game);
                break;

            default:
                menu_draw(&game);
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

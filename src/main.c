#include "raylib.h"
#include "game.h"
#include "board.h"
#include "board_render.h"
#include "battle.h"
#include "dice.h"
#include "pokemon.h"  
#include "menu.h"
#include <stdio.h>
#include <string.h>

static void load_poke_sprites(Game *g) {
    g->pokeSprites[POKE_FIRE]      = LoadTexture("assets/images/fire.png");
    g->pokeSprites[POKE_WATER]     = LoadTexture("assets/images/water.png");
    g->pokeSprites[POKE_GRASS]     = LoadTexture("assets/images/grass.png");
    g->pokeSprites[POKE_ELECTRIC]  = LoadTexture("assets/images/electric.png");
    g->pokeSprites[POKE_PSYCHIC]   = LoadTexture("assets/images/psychic.png");
    g->pokeSprites[POKE_DRAGON]    = LoadTexture("assets/images/dragon.png");
    g->pokeSprites[POKE_ICE]       = LoadTexture("assets/images/glaceon.png");
g->pokeSprites[POKE_FIGHTING]  = LoadTexture("assets/images/machamp.png");

    // Smooth sprite scaling when the window is resized/fullscreened.
    for (int i = 1; i < 9; i++) {
        SetTextureFilter(g->pokeSprites[i], TEXTURE_FILTER_BILINEAR);
    }
}

static void unload_poke_sprites(Game *g) {
    for (int i = 1; i < 9; i++) {
        UnloadTexture(g->pokeSprites[i]);
    }
}

// ── Classic Mode helpers ──

// Advance the turn to the next player who has not finished.
static void advance_turn(Game *g) {
    // Desired turn order:
    // Red -> Blue -> Yellow -> Green -> Red
    static const int turnOrder[MAX_PLAYERS] = {0, 1, 2, 3};

    int currentIndex = 0;

    // Find current player's position in the turn order
    for (int i = 0; i < g->playerCount; i++) {
        if (turnOrder[i] == g->currentPlayer) {
            currentIndex = i;
            break;
        }
    }

    int nextIndex = currentIndex;

    do {
        nextIndex = (nextIndex + 1) % g->playerCount;
    } while (
        g->players[turnOrder[nextIndex]].finished &&
        nextIndex != currentIndex
    );

    g->currentPlayer = turnOrder[nextIndex];
}

// Find an opponent's ACTIVE token standing on the same square as mine
static int find_opponent_on(Game *g, int square, int myPlayer, int *oppToken) {  //Which player is the opponent? and Which token of that player is there? int *opptoken is a pointer as it returns these q/a

    for (int p = 0; p < g->playerCount; p++) {
        if (p == myPlayer) continue; // if two tokens from palyer 1 lands on same square, dont initiate a battle
        for (int k = 0; k < TOKENS_PER_PLAYER; k++) { // 2 for loops bcs, each 4 player has 2 poke tokens
            Token *t = &g->players[p].tokens[k]; // g->players[2].tokens[1] means player 2's 2nd poke, & for the address where its located
            if (t->state == TOKEN_ACTIVE && GetSharedBoardSquare(p, t->progress) == square) {
                *oppToken = k;// when if cond fulfilled, that token k of that player p is my opponent
                return p;
            }
        }
    }
    return -1; // if nobody is in that square except me or my another token, we dont do anything 
}

// Start a battle between two tokens (Classic Mode).
static void start_battle_tokens(Game *g, int atkPlayer, int atkToken, int defPlayer, int defToken) {  // this func is for preparing the battle ground
    Token *atk = &g->players[atkPlayer].tokens[atkToken];
    Token *def = &g->players[defPlayer].tokens[defToken];
    g->state = STATE_BATTLE;
    g->battle.attackerIdx = atkPlayer;
    g->battle.defenderIdx = defPlayer;
    g->battle.attackerToken = atkToken;
    g->battle.defenderToken = defToken;
    g->battle.rollsLeft = 0;
    g->battle.attackerHp = atk->pokemon.hp;
    g->battle.defenderHp = def->pokemon.hp;
    g->battle.attackerMaxHp = atk->pokemon.maxHp;
    g->battle.defenderMaxHp = def->pokemon.maxHp;
    g->battle.finished = false;
    g->battle.currentRoll = 0;
    sprintf(g->battle.message, "BATTLE ! %s vs %s!", atk->pokemon.name, def->pokemon.name);
    g->battle.messageTimer = 60;
}

// Re-count the current player's finished tokens; all of them home = victory.
static void check_finish(Game *g) {
    Player *p = &g->players[g->currentPlayer  /* player array index 0-3  er moddhe current player  0 1 2 nki 3*/]; // checks if it player 1 or 2 or 3 or 4
    int fin = 0; // number of finished pokemon=0
    for (int i = 0; i < TOKENS_PER_PLAYER; i++) {
        if (p->tokens[i].state == TOKEN_FINISHED) fin++; // ekta poke ekta full round dile fin++ hobe
    }
    p->finishedCount = fin;
    if (fin == TOKENS_PER_PLAYER /* if everybody is home */ ) {
        int order = 1;
        for (int i = 0; i < g->playerCount; i++) {
            if (g->players[i].finished && i != g->currentPlayer /*checks if we already marked this one, if not, we mark a position, 1st 2nd etc...*/) order++;// marking who finished first
        }
        p->finished = true;
        p->finishOrder = order;// marks which player achieved which position
        g->state = STATE_GAME_OVER; 
    }
}

// Apply the battle outcome: loser's token returns to base, winner keeps the square.
static void resolve_battle(Game *g) {
    int atk = g->battle.attackerIdx;
    int def = g->battle.defenderIdx;
    if (g->mode == MODE_CLASSIC) {
        Token *atkT = &g->players[atk].tokens[g->battle.attackerToken];  //specifying player 0 er token 0 is attacker
        Token *defT = &g->players[def].tokens[g->battle.defenderToken]; //specifying player 2 er token 1 is defender
        atkT->pokemon.hp = g->battle.attackerHp; // During the battle, HP is updated in g->battle, not immediately in the tokens. These lines copy the final HP values back to the Pokémon stored in the real tokens
        defT->pokemon.hp = g->battle.defenderHp; 
        if (g->battle.attackerWon) {// attackerWOn is a bool, if true defender-->base, atkr++
            SendTokenToBase(defT);
            g->players[atk].wins++;
        } else {
            SendTokenToBase(atkT);// sendTokenBase(token *t) is a function from board.c
            g->players[def].wins++;
        }
    } 
    else // if the attack occurs in ladder mode
    {
        g->players[atk].pokemon.hp = g->battle.attackerHp;// no token tension, pokemon is stored directly under player
        g->players[def].pokemon.hp = g->battle.defenderHp;
        if (g->battle.attackerWon) {
            g->players[def].position = 0;// if someone loses they are sent back to starting square 0
            g->players[atk].wins++; // winner stays in the square
        } else {
            g->players[atk].position = 0;// player is an array of structs btw so each player er under e achhe win ,lose ,pos etc
            g->players[def].wins++;
        }
    }
}

//shows game over page ki show korbe 
static void draw_game_over(Game *g) { //static means this helper function can only be used inside main.c.
    DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){255, 127, 127, 255});

    DrawText("GAME OVER",/*placing text at horizontal center*/ WINDOW_W/2 - MeasureText("GAME OVER", 48/*font size 48*/)/2, 120, 48, (Color){255, 255, 255, 255});
    //DrawRectangle(0, 0, WINDOW_W, WINDOW_H, (Color){209, 232, 252, 255});
    if (g->mode == MODE_CLASSIC) {
        for (int i = 0; i < g->playerCount; i++) {
            if (g->players[i].finished && g->players[i].finishOrder == 1) {//checks if a player has finished and if he was the first to finish
                char winBuf[128];// winner message
                sprintf(winBuf, "%s WINS!", g->players[i].name);
                DrawText(winBuf, WINDOW_W/2 - MeasureText(winBuf, 40)/2, 210, 40, g->players[i].color);

                char team[256] = "Team: ";
                for (int k = 0; k < TOKENS_PER_PLAYER; k++) {
                    strncat(team, g->players[i].tokens[k].pokemon.name, sizeof(team) - strlen(team) - 1);
                    if (k < TOKENS_PER_PLAYER - 1) strncat(team, ", ", sizeof(team) - strlen(team) - 1);
                }
                DrawText(team, WINDOW_W/2 - MeasureText(team, 20)/2, 270, 20, WHITE);
                break;
            }
        }
        for (int i = 0; i < g->playerCount; i++) {
            char resBuf[128];
            sprintf(resBuf, "#%d %s - Home %d/%d - Wins: %d", g->players[i].finishOrder,
                    g->players[i].name, g->players[i].finishedCount, TOKENS_PER_PLAYER,
                    g->players[i].wins);
            DrawText(resBuf, WINDOW_W/2 - MeasureText(resBuf, 18)/2, 350 + i * 32, 18, (Color){180, 180, 200, 255});
        }
    } else {//g->mode != MODE_CLASSIC
        for (int i = 0; i < g->playerCount; i++) {
            if (g->players[i].finishOrder == 1) {
                char winBuf[128];
                sprintf(winBuf, "%s wins with %s!", g->players[i].name, g->players[i].pokemon.name);
                DrawText(winBuf, WINDOW_W/2 - MeasureText(winBuf, 28)/2, 240, 28, g->players[i].color);
                break;
            }
        }
        for (int i = 0; i < g->playerCount; i++) {
            char resBuf[64];
            sprintf(resBuf, "#%d %s (%s) - Wins: %d", g->players[i].finishOrder,
                    g->players[i].name, g->players[i].pokemon.name, g->players[i].wins);
            DrawText(resBuf, WINDOW_W/2 - MeasureText(resBuf, 18)/2, 350 + i * 30, 18, (Color){0, 0, 0, 255});
        }
    }

    DrawText("Press SPACE to play again", WINDOW_W/2 - 160, 560, 20, (Color){255, 255, 255, 255});
}
int main(void) {
    // 1200x800 is the logical design resolution. The actual window is
    // resizable; Camera2D scales the game directly to the framebuffer.
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_W, WINDOW_H, "Rick-n-roll");
    SetWindowMinSize(800, 533);
    SetTargetFPS(60);

    Camera2D gameCamera = {0};
    gameCamera.target = (Vector2){WINDOW_W / 2.0f, WINDOW_H / 2.0f};
    gameCamera.offset = (Vector2){WINDOW_W / 2.0f, WINDOW_H / 2.0f};
    gameCamera.rotation = 0.0f;
    gameCamera.zoom = 1.0f;

    Game game = {0};
    game_init(&game);
    load_poke_sprites(&game);

    Dice dice = {0};
    bool wasRolling = false;
    bool awaitingTokenChoice = false; // classic: waiting for a token key press
    int turnRolls[3] = {0, 0, 0};    // all dice values earned in the current Classic turn
    int turnRollCount = 0;            // number of stored rolls (1-3)
    int turnRollIndex = 0;            // roll currently being applied
    int sixCount = 0;                  // consecutive sixes in this turn

    while (!WindowShouldClose()) {
                // F11 toggles fullscreen
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        // Menu handling: only runs while on a menu screen
        if (game.state == STATE_MENU || game.state == STATE_PLAYER_COUNT) {
            int r = menu_update(&game);
            if (r == MENU_MODE_PICKED) {
                game.state = STATE_PLAYER_COUNT;
            } else if (r >= 2 && r <= 4) {
                game.playerCount = r;
                if (game.mode == MODE_CLASSIC)
                    poke_assign_party(game.players, game.playerCount);
                else
                    poke_assign_random(game.players, game.playerCount);
                board_init(&game);
                for (int i = 0; i < game.playerCount; i++) {
                    game.players[i].position = 0;
                    game.players[i].finished = false;
                    game.players[i].finishOrder = 0;
                    game.players[i].wins = 0;
                    game.players[i].finishedCount = 0;
                }
                game.currentPlayer = 0; // Red always starts
                awaitingTokenChoice = false;
                turnRollCount = 0;
                turnRollIndex = 0;
                sixCount = 0;
                memset(turnRolls, 0, sizeof(turnRolls));
                game.state = STATE_PLAYING;
            }
        }

        if (IsKeyPressed(KEY_SPACE) && !dice.rolling && game.state == STATE_PLAYING && !awaitingTokenChoice) {
            dice_roll(&dice);
        }

        bool diceJustFinished = wasRolling && !dice.rolling;
        wasRolling = dice.rolling;
        dice_update(&dice);

        if (diceJustFinished && game.state == STATE_PLAYING) {
            if (game.mode == MODE_CLASSIC) {
                int rolled = dice.value;
                // Store every roll. A 6 grants another roll, but NEVER starts
                // that roll automatically. The player must press SPACE again.
                if (turnRollCount < 3) turnRolls[turnRollCount++] = rolled;

                if (rolled == 6) {
                    sixCount++;
                    // Three sixes in one turn cancel the entire turn.
                    if (sixCount >= 3) {
                        turnRollCount = 0;
                        turnRollIndex = 0;
                        sixCount = 0;
                        memset(turnRolls, 0, sizeof(turnRolls));
                        awaitingTokenChoice = false;
                        advance_turn(&game);
                    }
                    // Otherwise wait for SPACE. No movement happens yet.
                } else {
                    // First non-6 ends the rolling phase; all stored values
                    // can now be spent independently.
                    sixCount = 0;
                    turnRollIndex = 0;
                    while (turnRollIndex < turnRollCount) {
                        int value = turnRolls[turnRollIndex];
                        int pl = game.currentPlayer;
                        bool any = false;
                        for (int i = 0; i < TOKENS_PER_PLAYER; i++) {
                            Token *t = &game.players[pl].tokens[i];
                            if (CanDeployToken(t, value) || CanMoveToken(t, value)) {
                                any = true;
                                break;
                            }
                        }
                        if (any) {
                            awaitingTokenChoice = true;
                            break;
                        }
                        turnRollIndex++;
                    }
                    if (turnRollIndex >= turnRollCount) {
                        turnRollCount = 0;
                        turnRollIndex = 0;
                        memset(turnRolls, 0, sizeof(turnRolls));
                        awaitingTokenChoice = false;
                        advance_turn(&game);
                    }
                }
            } else {
                // ── Ladder mode: single-token movement ──
                Player *cur = &game.players[game.currentPlayer];
                if (!cur->finished) {
                    int newPos = cur->position + dice.value;
                    if (newPos > BOARD_SQUARES) newPos = BOARD_SQUARES;
                    cur->position = newPos;
                    if (cur->position > 0) {
                        for (int i = 0; i < game.playerCount; i++) {
                            if (i != game.currentPlayer &&
                                game.players[i].position == cur->position &&
                                !game.players[i].finished) {
                                game.battle.attackerIdx = game.currentPlayer;
                                game.battle.defenderIdx = i;
                                game.battle.attackerToken = 0;
                                game.battle.defenderToken = 0;
                                game.battle.rollsLeft = 0;
                                game.battle.attackerHp = cur->pokemon.hp;
                                game.battle.defenderHp = game.players[i].pokemon.hp;
                                game.battle.attackerMaxHp = cur->pokemon.maxHp;
                                game.battle.defenderMaxHp = game.players[i].pokemon.maxHp;
                                game.battle.finished = false;
                                game.battle.currentRoll = 0;
                                sprintf(game.battle.message, "BATTLE! %s vs %s!", cur->name, game.players[i].name);
                                game.battle.messageTimer = 60;
                                game.state = STATE_BATTLE;
                                break;
                            }
                        }
                    }
                    if (cur->position >= BOARD_SQUARES) {
                        cur->finished = true;
                        cur->finishOrder = 1;
                        for (int i = 0; i < game.playerCount; i++) {
                            if (game.players[i].finished && i != game.currentPlayer)
                                cur->finishOrder = game.players[i].finishOrder + 1;
                        }
                    }
                    int finishedCount = 0;
                    for (int i = 0; i < game.playerCount; i++)
                        if (game.players[i].finished) finishedCount++;
                    if (finishedCount >= game.playerCount) game.state = STATE_GAME_OVER;
                }
                if (game.state == STATE_PLAYING) advance_turn(&game);
            }
        }

        // Classic mode: spend stored dice values one at a time.
        // Each value is independent: 6,2 may be deploy+move, move+move, etc.
        if (awaitingTokenChoice && game.state == STATE_PLAYING) {
            int pick = -1;
            if (IsKeyPressed(KEY_ONE)) pick = 0;
            else if (IsKeyPressed(KEY_TWO)) pick = 1;
            else if (IsKeyPressed(KEY_THREE)) pick = 2;
            else if (IsKeyPressed(KEY_FOUR)) pick = 3;

            if (pick >= 0 && turnRollIndex < turnRollCount) {
                int pl = game.currentPlayer;
                int roll = turnRolls[turnRollIndex];
                Token *t = &game.players[pl].tokens[pick];
                bool used = false;
                bool battled = false;

                if (CanDeployToken(t, roll)) {
                    t->state = TOKEN_ACTIVE;
                    t->progress = 1;
                    used = true;
                } else if (CanMoveToken(t, roll)) {
                    MoveToken(t, roll);
                    used = true;
                    if (t->state == TOKEN_ACTIVE) {
                        int sq = GetSharedBoardSquare(pl, t->progress);
                        int oppToken = -1;
                        int opp = find_opponent_on(&game, sq, pl, &oppToken);
                        if (opp >= 0 && !IsSafeSquare(sq)) {
                            start_battle_tokens(&game, pl, pick, opp, oppToken);
                            battled = true;
                        }
                    }
                }

                if (used) {
                    awaitingTokenChoice = false;
                    turnRollIndex++;
                    if (!battled && game.state == STATE_PLAYING) {
                        check_finish(&game);
                        if (game.state == STATE_PLAYING) {
                            while (turnRollIndex < turnRollCount) {
                                int nextRoll = turnRolls[turnRollIndex];
                                bool any = false;
                                for (int i = 0; i < TOKENS_PER_PLAYER; i++) {
                                    Token *nt = &game.players[pl].tokens[i];
                                    if (CanDeployToken(nt, nextRoll) || CanMoveToken(nt, nextRoll)) {
                                        any = true;
                                        break;
                                    }
                                }
                                if (any) {
                                    awaitingTokenChoice = true;
                                    break;
                                }
                                turnRollIndex++;
                            }
                            if (turnRollIndex >= turnRollCount) {
                                turnRollCount = 0;
                                turnRollIndex = 0;
                                sixCount = 0;
                                memset(turnRolls, 0, sizeof(turnRolls));
                                advance_turn(&game);
                            }
                        }
                    }
                }
            }
        }

        // Handle battle input
        if (game.state == STATE_BATTLE) {
            if (game.battle.messageTimer > 0) game.battle.messageTimer--;
            if (game.battle.finished) {
                if (IsKeyPressed(KEY_SPACE)) {
                    resolve_battle(&game);
                    game.state = STATE_PLAYING;
                    if (game.mode == MODE_CLASSIC) {
                        // The triggering roll was already spent. Continue with
                        // the next stored roll, if one remains.
                        int pl = game.currentPlayer;
                        while (turnRollIndex < turnRollCount) {
                            int nextRoll = turnRolls[turnRollIndex];
                            bool any = false;
                            for (int i = 0; i < TOKENS_PER_PLAYER; i++) {
                                Token *nt = &game.players[pl].tokens[i];
                                if (CanDeployToken(nt, nextRoll) || CanMoveToken(nt, nextRoll)) {
                                    any = true;
                                    break;
                                }
                            }
                            if (any) {
                                awaitingTokenChoice = true;
                                break;
                            }
                            turnRollIndex++;
                        }
                        if (turnRollIndex >= turnRollCount) {
                            turnRollCount = 0;
                            turnRollIndex = 0;
                            sixCount = 0;
                            memset(turnRolls, 0, sizeof(turnRolls));
                            advance_turn(&game);
                        }
                    } else {
                        advance_turn(&game);
                    }
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
                awaitingTokenChoice = false;
                turnRollCount = 0;
                turnRollIndex = 0;
                sixCount = 0;
                memset(turnRolls, 0, sizeof(turnRolls));
            }
        }

        // ---------------------------------------------------------
        // DRAW AT THE LOGICAL 1200x800 GAME COORDINATES.
        // Camera2D scales those coordinates to the current window size.
        // ---------------------------------------------------------
        BeginDrawing();
        ClearBackground(BLACK);

        float screenW = (float)GetScreenWidth();
        float screenH = (float)GetScreenHeight();

        // Preserve the 1200x800 aspect ratio so the board is never stretched.
        float scaleX = screenW / (float)WINDOW_W;
        float scaleY = screenH / (float)WINDOW_H;
        float scale = (scaleX < scaleY) ? scaleX : scaleY;

        gameCamera.target = (Vector2){WINDOW_W / 2.0f, WINDOW_H / 2.0f};
        gameCamera.offset = (Vector2){screenW / 2.0f, screenH / 2.0f};
        gameCamera.zoom = scale;

        BeginMode2D(gameCamera);
        ClearBackground((Color){15, 15, 30, 255});

        if (game.state == STATE_MENU || game.state == STATE_PLAYER_COUNT) {
            menu_draw(&game);
        } else if (game.state == STATE_BATTLE) {
            battle_draw(&game);
        } else if (game.state == STATE_GAME_OVER) {
            draw_game_over(&game);
        } else {
            board_draw(&game);
            board_draw_hud(&game);
            dice_draw(&dice, WINDOW_W - 150, 430);

            Player *cur = &game.players[game.currentPlayer];

            // Prominent turn announcement: shown before the first roll of a
            // turn and hidden as soon as that player starts rolling.
            if (game.state == STATE_PLAYING && !dice.rolling &&
                turnRollCount == 0 && !awaitingTokenChoice) {
                char turnBuf[64];
                sprintf(turnBuf, "%s's turn", cur->name);
                int turnFont = 42;
                int turnX = WINDOW_W / 2 - MeasureText(turnBuf, turnFont) / 2;
                DrawText(turnBuf, turnX + 3, 18 + 3, turnFont, BLACK);
                DrawText(turnBuf, turnX, 18, turnFont, cur->color);
                DrawRectangleLinesEx(
                    (Rectangle){(float)turnX - 16, 10,
                                (float)MeasureText(turnBuf, turnFont) + 32,
                                56},
                    2, cur->color);
            }

            if (game.mode == MODE_CLASSIC) {
                char line[256] = "";
                for (int i = 0; i < TOKENS_PER_PLAYER; i++) {
                    Token *t = &cur->tokens[i];
                    char part[64];
                    if (t->state == TOKEN_BASE)
                        sprintf(part, "%d:%s[base]", i + 1, t->pokemon.name);
                    else if (t->state == TOKEN_FINISHED)
                        sprintf(part, "%d:%s[goal]", i + 1, t->pokemon.name);
                    else if (t->state == TOKEN_HOME)
                        sprintf(part, "%d:%s[home%d]", i + 1, t->pokemon.name, t->progress - SHARED_TRACK_STEPS);
                    else
                        sprintf(part, "%d:%s[%d]", i + 1, t->pokemon.name,
                                GetSharedBoardSquare(game.currentPlayer, t->progress));
                    strncat(line, part, sizeof(line) - strlen(line) - 1);
                    strncat(line, "  ", sizeof(line) - strlen(line) - 1);
                }
                DrawText(line, 20, 48, 14, (Color){180, 180, 200, 255});

                if (turnRollCount > 0) {
                    char rollsBuf[160] = "Rolls: ";
                    for (int r = 0; r < turnRollCount; r++) {
                        char part[24];
                        if (r == turnRollIndex && awaitingTokenChoice)
                            sprintf(part, "[%d] ", turnRolls[r]);
                        else
                            sprintf(part, "%d ", turnRolls[r]);
                        strncat(rollsBuf, part, sizeof(rollsBuf) - strlen(rollsBuf) - 1);
                    }
                    DrawText(rollsBuf, 20, 78, 18, WHITE);
                }
                if (awaitingTokenChoice) {
                    char prompt[96];
                    sprintf(prompt, "Using roll %d: press 1-%d to choose a Pokemon",
                            turnRolls[turnRollIndex], TOKENS_PER_PLAYER);
                    DrawText(prompt, 20, 105, 18, (Color){255, 202, 40, 255});
                } else if (!dice.rolling && game.state == STATE_PLAYING && turnRollCount > 0 && sixCount > 0) {
                    char prompt[96];
                    sprintf(prompt, "%d! Press SPACE to roll again", sixCount);
                    DrawText(prompt, 20, 105, 18, (Color){255, 202, 40, 255});
                }
            } else {
                char pokeBuf[64];
                sprintf(pokeBuf, "%s (%s)", cur->pokemon.name, poke_type_name(cur->pokemon.type));
                DrawText(pokeBuf, 20, 48, 16, (Color){180, 180, 200, 255});
            }

            if (!dice.rolling && game.state == STATE_PLAYING && !awaitingTokenChoice) {
                if (game.mode != MODE_CLASSIC || turnRollCount == 0) {
                    DrawText("Press SPACE to roll", WINDOW_W - 230, WINDOW_H - 30, 16, (Color){255, 202, 40, 255});
                } else if (sixCount > 0) {
                    DrawText("SPACE = bonus roll", WINDOW_W - 230, WINDOW_H - 30, 16, (Color){255, 202, 40, 255});
                }
            }
        }

        EndMode2D();
        EndDrawing();
    }

    unload_poke_sprites(&game);
    CloseWindow();

    return 0;
}

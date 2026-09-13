#include "game.h"         //gives this file access to things defined there, such as game,player,token,sq type etc.
#include "board.h"        //allows board_render.c to work with the board functionality
#include "board_render.h"
#include "pokemon.h"      //Allows this file to use Pokémon-related functions
#include <stdio.h>

#define CELL_SIZE 100     //This represents the size of a board cell in pixels in whichever drawing code uses it
#define BOARD_X 60        
#define BOARD_Y 140       //These define the position of the board

// Classic Ludo layout (must match board.c)
#define LUDO_GRID 15    //the Classic Ludo board is treated as a 15 × 15 grid
#define LUDO_CELL 44.0f//Each grid cell is 44 pixels wide/high;Because Raylib's drawing/position calculations often use floating-point values
#define LUDO_X 30.0f   
#define LUDO_Y 100.0f  //These specify where the 15×15 Ludo grid starts on the screen

static Color square_color(SquareType t) {                    //Given a type of board square, decide what color that square should be
    switch (t) {//Here static means the function is private to this .c file,Other .c files cannot directly call this function
        case SQ_SAFE:     return (Color){200, 230, 200, 255};//If the square is a safe square, return this Raylib color
        case SQ_LADDER:   return (Color){180, 230, 180, 255};//Ladder → greenish color
        case SQ_SNAKE:    return (Color){230, 180, 180, 255};//Snake → reddish color
        case SQ_EVOLUTION:return (Color){210, 180, 230, 255};//Evolution → purple-ish color
        case SQ_HABITAT:  return (Color){180, 210, 230, 255};//Habitat → blue-ish color
        case SQ_MYSTERY:  return (Color){230, 210, 180, 255};//Mystery → beige-ish color
        case SQ_STONE:    return (Color){200, 200, 220, 255};//Stone → gray-ish color
        default:          return (Color){240, 235, 220, 255};//If t doesn't match any of the listed cases, use the default color
    }
}

static const char* square_label(SquareType t) {  //takes a SquareType,returns a string
    switch (t) {
        case SQ_SAFE:     return "SAFE";
        case SQ_LADDER:   return "LADDER";
        case SQ_SNAKE:    return "SNAKE";
        case SQ_EVOLUTION:return "EVOLVE";
        case SQ_HABITAT:  return "WILD";
        case SQ_MYSTERY:  return "MYSTERY";
        case SQ_STONE:    return "STONE";
        default:          return "";           //For a normal square, return an empty string
    }
}

static void draw_owner_marker(Vector2 pos, Color ownerColor) {//draws a small marker showing which player owns a Pokémon/token
    // Small owner-color marker placed near the top of each Pokemon sprite.
    // This makes identical Pokemon distinguishable when multiple players own
    // the same species/type.
    const float r = 3.0f;//Creates a constant floating-point variable r,So the marker's radius is 3 pixels
    Vector2 marker = {pos.x, pos.y - 14.0f};//Creates a new Vector2 called marker,x=same as pokemon,y = Pokémon's y - 14
    DrawCircleV(marker, r + 1.0f, BLACK);//Raylib function that draws a filled circle
    DrawCircleV(marker, r, ownerColor);//Draws the smaller colored circle on top
    DrawCircleLinesV(marker, r, WHITE);//Draws a white outline around the circle
}

static void draw_token_small(Game *g, Token *t, Player *pl, Vector2 pos) {//draws a small Pokémon on the board.
    // Validate the Pokemon type before using it as an array index.
    if (t->pokemon.type <= POKE_NONE ||
    t->pokemon.type > POKE_FIGHTING) {    //whether the Pokémon type is valid

        DrawCircleV(pos, 18, pl->color);  //If the Pokémon type is invalid, instead of trying to access a sprite, draw a simple circle using the player's color
        DrawCircleLinesV(pos, 18, BLACK); //Draws a black outline around that circle
        return;
    }

    Texture2D spr = g->pokeSprites[t->pokemon.type];//Texture2D is a Raylib type used for an image,This gets the Pokémon's image from the sprite array

    // Texture wasn't loaded correctly.
    if (spr.id == 0 || spr.width <= 0 || spr.height <= 0) {//This checks if the texture is invalid
        DrawCircleV(pos, 18, poke_type_color(t->pokemon.type));//If the sprite failed, draw a circle using the Pokémon type's color instead
        DrawCircleLinesV(pos, 18, pl->color);
        DrawCircleLinesV(pos, 18, BLACK);
        return;
    }

    // Keep the sprite inside the 44x44 board cell.
    float spriteSize = 40.0f;//The Pokémon sprite should be approximately 40 × 40 pixels
    float scale = spriteSize / (float)spr.width;//calculates how much the original image needs to be scaled

    DrawTextureEx(//Raylib function for drawing a texture with position,rotation,scale,tint
        spr,     //The Pokémon image to draw
        (Vector2){
            pos.x - spriteSize / 2.0f,//calculates the top-left corner of the sprite
            pos.y - spriteSize / 2.0f//subtraction because pos is being treated as the center of the Pokémon
        },
        0.0f,//Rotation angle,0 meas dont rotate the angle
        scale,//The scale calculated earlier
        WHITE
    );

    // Owner marker is deliberately drawn last so it stays visible over the
    // Pokemon image.
    draw_owner_marker(pos, pl->color);//After drawing the Pokémon, draw the small player-color marker
}

static void draw_classic_board(Game *g) {
    float bw = LUDO_GRID * LUDO_CELL;//Calculates the board's total width
    DrawRectangle((int)LUDO_X - 16, (int)LUDO_Y - 16, (int)bw + 32, (int)bw + 32, (Color){25, 25, 45, 255});//draws a large dark rectangle slightly bigger than the board
    DrawRectangleLinesEx((Rectangle){LUDO_X - 16, LUDO_Y - 16, bw + 32, bw + 32}, 2, (Color){90, 90, 120, 255});//This draws an outline around that rectangle,2 means the outline is 2 pixels thick

    // Corner base yards (6x6 quadrants) in each player's color
static const Color baseColors[4] = {RED, BLUE, YELLOW, GREEN};
    static const int qr[4] = {0, 0, 9, 9}; // quadrant row offsets (Red=TL, Blue=TR, Green=BR, Yellow=BL)
    static const int qc[4] = {0, 9, 9, 0};
    for (int p = 0; p < MAX_PLAYERS; p++) {
        Color c = baseColors[p];//Gets the current player's color
        DrawRectangle((int)(LUDO_X + qc[p] * LUDO_CELL), (int)(LUDO_Y + qr[p] * LUDO_CELL),
                      (int)(6 * LUDO_CELL), (int)(6 * LUDO_CELL),
                      (Color){c.r, c.g, c.b, 60});//draws each player's 6×6 base area
    }

    // Center 3x3 finish square
    DrawRectangle((int)(LUDO_X + 6 * LUDO_CELL), (int)(LUDO_Y + 6 * LUDO_CELL),
                  (int)(3 * LUDO_CELL), (int)(3 * LUDO_CELL), (Color){230, 226, 210, 255});//makes the player's base semi-transparent
    DrawRectangleLinesEx((Rectangle){LUDO_X + 6 * LUDO_CELL, LUDO_Y + 6 * LUDO_CELL,
                         3 * LUDO_CELL, 3 * LUDO_CELL}, 2, (Color){80, 70, 50, 255});

    // Shared 52-square track
    for (int i = 0; i < BOARD_SIZE; i++) {
        BoardSquare *sq = &g->board[i];
        Vector2 c = sq->screenPos;
        Rectangle r = {c.x - LUDO_CELL/2, c.y - LUDO_CELL/2, LUDO_CELL, LUDO_CELL};
        DrawRectangleRec(r, square_color(sq->type));
        DrawRectangleLinesEx(r, 1, (Color){100, 90, 70, 255});

        char id[4];
        sprintf(id, "%d", sq->id);
        int fs = (sq->id < 10) ? 10 : 8;
        DrawText(id, (int)(c.x - fs/2), (int)(c.y - 8), fs, (Color){80, 70, 50, 170});
    }

    // Home lanes (private, 6 cells per player, inward toward center)
    for (int p = 0; p < MAX_PLAYERS; p++) {
        Color c = baseColors[p];
        for (int i = 0; i < HOME_STEPS; i++) {
            Vector2 hc = g->homeLanePos[p][i];
            Rectangle r = {hc.x - LUDO_CELL/2, hc.y - LUDO_CELL/2, LUDO_CELL, LUDO_CELL};
            DrawRectangleRec(r, c);
            DrawRectangleLinesEx(r, 1, (Color){10, 10, 15, 255});
            // Poké Ball in the center destination
if (g->pokeballTexture.id != 0) {

    float ballSize = 100.0f;

    Rectangle source = {
        0,
        0,
        (float)g->pokeballTexture.width,
        (float)g->pokeballTexture.height
    };

    Rectangle destination = {
        LUDO_X + 7.5f * LUDO_CELL - ballSize / 2.0f,
        LUDO_Y + 7.5f * LUDO_CELL - ballSize / 2.0f,
        ballSize,
        ballSize
    };

    DrawTexturePro(
        g->pokeballTexture,
        source,
        destination,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
}
        }
    }

    // Base yards: label each corner quadrant with the player's initial
static const char* baseNames[4] = {"R", "B", "Y", "G"};
    for (int p = 0; p < MAX_PLAYERS; p++) {
        float minx = 1e9f, miny = 1e9f, maxx = -1e9f, maxy = -1e9f;
        for (int i = 0; i < TOKENS_PER_PLAYER; i++) {
            Vector2 c = g->basePos[p][i];
            if (c.x < minx) minx = c.x;
            if (c.x > maxx) maxx = c.x;
            if (c.y < miny) miny = c.y;
            if (c.y > maxy) maxy = c.y;
        }
        DrawRectangle((int)(minx - LUDO_CELL/2 - 4), (int)(miny - LUDO_CELL/2 - 4),
                      (int)(maxx - minx + LUDO_CELL + 8), (int)(maxy - miny + LUDO_CELL + 8),
                      (Color){baseColors[p].r, baseColors[p].g, baseColors[p].b, 70});
        DrawRectangleLines((int)(minx - LUDO_CELL/2 - 4), (int)(miny - LUDO_CELL/2 - 4),
                           (int)(maxx - minx + LUDO_CELL + 8), (int)(maxy - miny + LUDO_CELL + 8), baseColors[p]);
        DrawText(baseNames[p], (int)(minx - 5), (int)(maxy + 4), 14, baseColors[p]);
    }

    // Tokens
    int drawn[BOARD_SIZE];
    for (int i = 0; i < BOARD_SIZE; i++) drawn[i] = 0;
    Vector2 center = {LUDO_X + 7.5f * LUDO_CELL, LUDO_Y + 7.5f * LUDO_CELL};

    for (int p = 0; p < g->playerCount; p++) {
        Player *pl = &g->players[p];
        for (int k = 0; k < TOKENS_PER_PLAYER; k++) {
            Token *t = &pl->tokens[k];
            Vector2 pos;
            if (t->state == TOKEN_BASE) {
                pos = g->basePos[p][k];
            } else if (t->state == TOKEN_FINISHED) {
                pos = (Vector2){center.x + (k % 2) * 16 - 8, center.y + (k / 2) * 16 - 8};
            } else if (t->state == TOKEN_HOME) {
                pos = g->homeLanePos[p][t->progress - SHARED_TRACK_STEPS - 1];
            } else {
                int sq = GetSharedBoardSquare(p, t->progress);
                int idx = sq - 1;
                pos = g->board[idx].screenPos;
                int off = drawn[idx]++;
                pos.x += (off % 2) * 16 - 8;
                pos.y += (off / 2) * 16 - 8;
            }
            draw_token_small(g, t, pl, pos);
        }
    }
}

void board_draw(Game *g) {
    if (g->mode == MODE_CLASSIC) {
        draw_classic_board(g);
        return;
    }

    // Ladder mode: existing 30-square serpentine board
    for (int i = 0; i < BOARD_SQUARES; i++) {
        BoardSquare *sq = &g->board[i];
        Vector2 c = sq->screenPos;
        Rectangle r = {c.x - CELL_SIZE/2, c.y - CELL_SIZE/2, CELL_SIZE, CELL_SIZE};
        Color bg = square_color(sq->type);
        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, 2, (Color){100, 90, 70, 255});

        char id[4];
        sprintf(id, "%d", sq->id);
        int fs = (sq->id < 10) ? 20 : 16;
        DrawText(id, (int)(c.x - fs/2), (int)(c.y - fs/2 - 12), fs, (Color){80, 70, 50, 180});

        const char *label = square_label(sq->type);
        if (label[0]) {
            int ls = 12;
            DrawText(label, (int)(c.x - MeasureText(label, ls)/2), (int)(c.y + 10), ls, (Color){60, 50, 40, 200});
        }
    }

    for (int p = 0; p < g->playerCount; p++) {
        Player *pl = &g->players[p];
        if (pl->finished || pl->position == 0) continue;
        int idx = pl->position - 1;
        if (idx < 0 || idx >= BOARD_SQUARES) continue;
        Vector2 pos = g->board[idx].screenPos;

        int offset_x = (p % 2 == 0) ? -15 : 15;
        int offset_y = (p < 2) ? -15 : 15;
        float tx = pos.x + offset_x;
        float ty = pos.y + offset_y;

        Texture2D spr = g->pokeSprites[pl->pokemon.type];
        if (spr.id > 0) {
            float scale = 60.0f / spr.width;
            DrawTextureEx(spr, (Vector2){tx - 30, ty - 30}, 0, scale, WHITE);
        } else {
            DrawCircleV((Vector2){tx, ty}, 30, pl->color);
            DrawCircleLinesV((Vector2){tx, ty}, 30, BLACK);
        }
        DrawText(pl->name, (int)(tx - 8), (int)(ty + 16), 10, pl->color);
    }
}

void board_draw_hud(Game *g) {
    int panelX = WINDOW_W - 250;
    int panelY = 20;

    if (g->mode == MODE_CLASSIC) {
        int ph = g->playerCount * 92 + 40;
        DrawRectangle(panelX, panelY, 230, ph, (Color){20, 20, 40, 200});
        DrawRectangleLines(panelX, panelY, 230, ph, (Color){80, 80, 100, 255});
        DrawText("PLAYERS", panelX + 10, panelY + 10, 14, (Color){180, 180, 200, 255});

static const int hudOrder[MAX_PLAYERS] = {0, 1, 2, 3};

for (int i = 0; i < g->playerCount; i++) {
    int p = hudOrder[i];
    int y = panelY + 35 + i * 92;
            if (i == g->currentPlayer && g->state == STATE_PLAYING) {
                DrawRectangle(panelX + 5, y - 5, 220, 82, (Color){40, 40, 60, 255});
            }
            DrawText(g->players[p].name, panelX + 10, y, 16, g->players[p].color);

            char typeBuf[32];
            sprintf(typeBuf, "%s", poke_type_name(g->players[p].tokens[0].pokemon.type));
            DrawText(typeBuf, panelX + 10, y + 22, 12, g->players[p].tokens[0].pokemon.color);

            char homeBuf[32];
            sprintf(homeBuf, "Home: %d/%d", g->players[p].finishedCount, TOKENS_PER_PLAYER);
            DrawText(homeBuf, panelX + 10, y + 42, 12, WHITE);

            char winBuf[32];
            sprintf(winBuf, "Wins: %d", g->players[p].wins);
            DrawText(winBuf, panelX + 10, y + 62, 12, (Color){180, 180, 200, 255});
        }
        return;
    }

    // Ladder mode: existing HUD with per-player HP bars
    DrawRectangle(panelX, panelY, 230, g->playerCount * 80 + 40, (Color){20, 20, 40, 200});
    DrawRectangleLines(panelX, panelY, 230, g->playerCount * 80 + 40, (Color){80, 80, 100, 255});
    DrawText("PLAYERS", panelX + 10, panelY + 10, 14, (Color){180, 180, 200, 255});

    for (int i = 0; i < g->playerCount; i++) {
        int y = panelY + 35 + i * 80;

        if (i == g->currentPlayer && g->state == STATE_PLAYING) {
            DrawRectangle(panelX + 5, y - 5, 220, 70, (Color){40, 40, 60, 255});
        }

        DrawText(g->players[i].name, panelX + 10, y, 16, g->players[i].color);

        char typeBuf[32];
        sprintf(typeBuf, "%s", poke_type_name(g->players[i].pokemon.type));
        DrawText(typeBuf, panelX + 10, y + 20, 12, g->players[i].pokemon.color);

        DrawRectangle(panelX + 10, y + 38, 150, 10, (Color){40, 40, 60, 255});
        float pct = (float)g->players[i].pokemon.hp / g->players[i].pokemon.maxHp;
        DrawRectangle(panelX + 10, y + 38, (int)(150 * pct), 10,
                      pct > 0.5f ? GREEN : pct > 0.25f ? YELLOW : RED);

        char posBuf[16];
        sprintf(posBuf, "%d/%d", g->players[i].position + 1, BOARD_SQUARES);
        DrawText(posBuf, panelX + 170, y + 34, 12, WHITE);
    }
}

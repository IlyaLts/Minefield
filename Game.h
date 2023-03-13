/*
===============================================================================
    Copyright (C) 2023 Ilya Lyakhovets

    This program is free software : you can redistribute it and /or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see < http://www.gnu.org/licenses/>.
===============================================================================
*/

#ifndef __GAME_H__
#define __GAME_H__

#include "Main.h"
#include "Tile.h"

#define TILE_SIZE                   25
#define BOOM_RATIUS                 30
#define BOOM_DURATION               1920.0f
#define MAX_TIME                    99999

// Difficulty constants
#define PREFERRED_GAME_DURATION     300
#define DEFAULT_MINE_RATIO          0.15f
#define DEFAULT_FIELD_WIDTH         25
#define DEFAULT_FIELD_HEIGHT        22
#define ATTEMPTS_BEFORE_CHANGE      5
#define MINE_RATIO_CHANGE           0.005f
#define MINIMAL_MINE_RATIO          0.05f
#define MAXIMAL_MINE_RATIO          0.25f
#define MINIMAL_FIELD_WIDTH         22
#define MINIMAL_FIELD_HEIGHT        18
#define MAXIMAL_FIELD_WIDTH         50
#define MAXIMAL_FIELD_HEIGHT        30

/*
===========================================================

    Game

===========================================================
*/
class Game
{
public:

    enum gameState_t
    {
        PLAYING,
        WON,
        LOST
    } gameState;

                    Game() : buttonRestart(TILE_SIZE * 2, TILE_SIZE * 2) {}

    bool            Init();
    void            Draw();
    void            Update();

    void            Restart();
    void            OpenEmptyNeighborTiles();
    void            FlagClosedMinesTiles();
    void            ShowAllMines();
    void            ClampFieldDimensions();

    bool            hasUnopenEmptyTiles() const;

    Tile            field[MAXIMAL_FIELD_WIDTH][MAXIMAL_FIELD_HEIGHT];

    libVec2i        fieldSize;
    libVec2i        newFieldSize;
    float           mineRatio;
    int             shownMinesLeft;
    int             minesLeft;
    int             gameTime = 0;
    int             attempts = 0;
    bool            tileClicked = false;
    libTimer        timer;

    libFont *       font = nullptr;
    libTexture *    t_curSmile = nullptr;
    libTexture *    t_smile = nullptr;
    libTexture *    t_smileClick = nullptr;
    libTexture *    t_smileWin = nullptr;
    libTexture *    t_smileLost = nullptr;
    libTexture *    t_tile = nullptr;
    libTexture *    t_tileOpen = nullptr;
    libTexture *    t_mine = nullptr;
    libTexture *    t_flag = nullptr;
    libTexture *    t_question = nullptr;
    libSprite *     t_boom = nullptr;
    libSound *      s_boom = nullptr;

    libVec2         boom;
    libButton       buttonRestart;
};

#endif // !__GAME_H__

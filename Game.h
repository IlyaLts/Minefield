/*
===============================================================================
    Copyright (C) 2023-2025 Ilya Lyakhovets

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

#pragma once

#include "Main.h"
#include "Tile.h"
#include "Settings.h"

#define MARGIN_X                    5
#define MARGIN_Y                    5
#define TILE_SIZE                   25

#define BOOM_RATIUS                 30
#define BOOM_DURATION               1920.0f
#define SCOREBOARD_MIN_VALUE        -99
#define SCOREBOARD_MAX_VALUE        999
#define MINIMAL_FIELD_WIDTH         10
#define MINIMAL_FIELD_HEIGHT        10
#define MAXIMAL_FIELD_WIDTH         70
#define MAXIMAL_FIELD_HEIGHT        35
#define MINIMAL_MINES               10
#define MAXIMAL_MINES               MAXIMAL_FIELD_WIDTH * MAXIMAL_FIELD_HEIGHT

// Auto difficulty constants
#define PREFERRED_GAME_DURATION     300
#define DEFAULT_MINE_RATIO          0.15f
#define DEFAULT_AUTO_FIELD_WIDTH    25
#define DEFAULT_AUTO_FIELD_HEIGHT   22
#define ATTEMPTS_BEFORE_CHANGE      3
#define MINE_RATIO_CHANGE           0.005f
#define MINIMAL_MINE_RATIO          0.05f
#define MAXIMAL_MINE_RATIO          0.25f

class Settings;

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

    bool            Init();
    void            Draw();
    void            Update();

    void            Restart();
    void            SaveSettings();

    void            ShowHelp() const;
    void            ToggleSettings();
    void            ToggleAudio();

    libCfg          cfg;

private:

    void            DrawPanel(const libVec2 corner, const libVec2 &corner2, float thickness);

    void            GenerateMines();
    void            UpdateHoveredTile();
    void            UpdateTiles();
    void            UpdateTileFlags();

    bool            LeftPressed() const;
    bool            LeftReleased() const;
    bool            LeftPressing() const;
    bool            RightPressed() const;
    bool            RightReleased() const;
    bool            RightPressing() const;
    bool            MiddlePressed() const;
    bool            MiddleReleased() const;
    bool            MiddlePressing() const;
    
    void            OpenTile(int x, int y);
    void            Chord(int x, int y);
    void            SetNeighborPressState(int x, int y, bool pressed);
    void            OpenEmptyNeighborTiles();
    void            FlagClosedMineTiles();
    void            ShowAllMines();
    void            ClampFieldDimensions();
    void            AdjustWindowSize();

    bool            hasUnopenEmptyTiles() const;
    bool            IsTileToBeUnpressed(int x, int y) const;
    bool            IsAdjacentTileHovered(int x, int y) const;
    bool            IsTileHovered(int x, int y) const;

    Settings        settings;

    Tile            field[MAXIMAL_FIELD_WIDTH][MAXIMAL_FIELD_HEIGHT];
    bool            tileClicked = false;
    bool            hoveredTile = false;
    libVec2i        hoveredTileCoord;
    bool            firstClick = false;
    libVec2i        firstClickCoord;

    libVec2i        fieldSize;
    libVec2i        autoFieldSize;
    float           mineRatio = DEFAULT_MINE_RATIO;
    int             shownMinesLeft = 0;
    int             minesLeft = 0;
    int             gameTime = 0;
    int             attempts = 0;
    libTimer        timer;
    bool            settingsShown = false;
    float           audioVolume = DEFAULT_AUDIO_VOLUME;

    libFont *       font = nullptr;
    libFont *       digital = nullptr;
    libTexture *    t_panel = nullptr;
    libTexture *    t_scoreboard = nullptr;
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

    libVec2         boomCoord;
    libButton       buttonRestart;
    libButton       buttonSettings;
};

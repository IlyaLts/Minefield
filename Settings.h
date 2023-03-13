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

#define DEFAULT_DIFFICULTY          Settings::AUTO
#define DEFAULT_CUSTOM_WIDTH        30
#define DEFAULT_CUSTOM_HEIGHT       20
#define DEFAULT_CUSTOM_MINES        145
#define DEFAULT_MARKS_ENABLED       true
#define DEFAULT_AUDIO_VOLUME        0.4f

class Game;

/*
===========================================================

    Settings

===========================================================
*/
class Settings
{
public:

    enum difficulty_t
    {
        BEGINNER,
        INTERMEDIATE,
        EXPERT,
        AUTO,
        CUSTOM
    };

    bool            Init(Game *game);
    void            Update();
    void            Draw();

    difficulty_t    Difficulty() const { return difficulty; }
    int             CustomWidth() const { return customWidth; }
    int             CustomHeight() const { return customHeight; }
    int             CustomMines() const { return customMines; }
    bool            MarksEnabled() const { return marksEnabled; }

private:

    Game *          game = nullptr;

    int             customWidth = DEFAULT_CUSTOM_WIDTH;
    int             customHeight = DEFAULT_CUSTOM_HEIGHT;
    int             customMines = DEFAULT_CUSTOM_MINES;
    difficulty_t    difficulty = DEFAULT_DIFFICULTY;
    difficulty_t    chosenDifficulty = DEFAULT_DIFFICULTY;
    bool            marksEnabled = true;

    libButton       buttonMines;
    libButton       buttonWidth;
    libButton       buttonHeight;
    libButton       buttonMarks;
    libButton       buttonSound;

    libButton *     selectedButton = nullptr;
    libButton       difficultyButtons[5];
    libButton       buttonSave;
    libButton       buttonHelp;

    libFont *       font = nullptr;
    libTexture *    t_tile = nullptr;
    libTexture *    t_tileOpen = nullptr;
    libTexture *    t_inputField = nullptr;
    libTexture *    t_question = nullptr;
    libTexture *    t_soundOn = nullptr;
    libTexture *    t_soundOff = nullptr;
    libTexture *    t_mine = nullptr;
};

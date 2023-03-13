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

#ifndef __MAIN_H__
#define __MAIN_H__

#include "../libEngine/Include/Engine.h"

#ifdef LIB_RELEASE
    #define ENGINE_PATH "./"
    #define DATA_PACK "Data.lpk//"
#else
    #define ENGINE_PATH "../../libEngine/Binaries/"
    #define DATA_PACK "Data/"
#endif

#define MINEFIELD_VERSION "1.0"
#define TILE_SIZE 25
#define BOOM_RATIUS 30
#define BOOM_DURATION 1920.0f
#define DEFAULT_BOOM_VOLUME 0.4f
#define MAX_TIME 99999

// Difficulty constants
#define DESIRED_GAME_DURATION 300
#define DEFAULT_MINE_RATIO 0.15f
#define DEFAULT_FIELD_WIDTH 25
#define DEFAULT_FIELD_HEIGHT 22
#define ATTEMPTS_BEFORE_CHANGE 5
#define MINE_RATIO_CHANGE 0.005f
#define MINIMAL_MINE_RATIO 0.05f
#define MAXIMAL_MINE_RATIO 0.25f
#define MINIMAL_FIELD_WIDTH 22
#define MINIMAL_FIELD_HEIGHT 18
#define MAXIMAL_FIELD_WIDTH 50
#define MAXIMAL_FIELD_HEIGHT 30

#endif // !__MAIN_H__

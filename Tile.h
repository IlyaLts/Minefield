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

/*
===========================================================

    Tile

===========================================================
*/
class Tile
{
public:

    enum type_t : libUint8
    {
        EMPTY,
        MINED
    };

    enum state_t : libUint8
    {
        CLOSED,
        OPEN,
        FLAGGED,
        QUESTIONED
    };

                    Tile() { Reset(); }

    void            Reset();

    bool            CanOpen() const;
    bool            IsIncorrectlyFlaggedOrMined() const;
    bool            HasNoNearestMines() const;

    libButton       button;
    int             nearestMines;
    type_t          type;
    state_t         state;
};

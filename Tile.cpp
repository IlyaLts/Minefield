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

#include "Main.h"
#include "Tile.h"

/*
===================
Tile::Reset
===================
*/
void Tile::Reset()
{
    type = EMPTY;
    state = CLOSED;
    nearestMines = 0;

    button.SetEnabled(true);
    button.textureColor.base = libColor();
}

/*
===================
Tile::CanOpen
===================
*/
bool Tile::CanOpen() const
{
    return state == CLOSED || state == QUESTIONED;
}

/*
===================
Tile::IsIncorrectlyFlaggedOrMined
===================
*/
bool Tile::IsIncorrectlyFlaggedOrMined() const
{
    return (type == MINED && state != FLAGGED) || (type != MINED && state == FLAGGED);
}

/*
===================
Tile::HasNoNearestMines
===================
*/
bool Tile::HasNoNearestMines() const
{
    return type == EMPTY && !nearestMines;
}

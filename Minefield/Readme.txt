===================
Introduction
===================
Minefield is a classic Minesweeper clone that has an automatic difficulty adjustment feature that adapts to your skill level. 

The rules are simple: click on a tile to reveal what's underneath. If it's a mine, you lose. If it's a number,
it tells you how many mines are adjacent to that tile. If it's blank, it means there are no mines nearby and
the surrounding tiles are automatically revealed. You can also right-click on a tile to mark it as a mine or as uncertain.
Additionally, you can use the chording feature, by middle-clicking a numbered tile to reveal its surrounding tiles,
but only if you've correctly flagged the exact number of adjacent mines. Be careful, as incorrectly flagged mines will cause
you to lose. The goal is to clear all the tiles that don't have mines.    

===================						  
Controls
===================

Mouse
-----
LMB - Open a tile.
RMB - Flag a tile.
LMB = RMB - Chord.
MMB - Chord.

Keyboard
--------
Space - Flag a tile\n"
Ctrl - Open a tile\n"
Shift - Chord\n"
Ctrl + Space - chord.
F1 - Help.
F3 - Settings.
F3 - Controls.
F4 - Turn on/off sound.
F12 - Take a screenshot

===================						  
System Requirements
===================
Any video card that supports OpenGL 4.6.

===================						  
Troubleshooting
===================

Windows
-------
If the game doesn't have sound, you need to install OpenAL (oalinst.exe).
You can find the installer in the Redist folder and run it by double-clicking on it.

If the game doesn't save screenshots (F12), you need to grant access to the game folder.
To do this, right-click on GrantAccess.bat, which is located in the game folder, and select ‘Run as administrator’.

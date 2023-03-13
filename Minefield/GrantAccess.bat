echo off
set gamePath=%~dpnx0
icacls "%gamePath:~0,-16%" /grant Everyone:M
pause
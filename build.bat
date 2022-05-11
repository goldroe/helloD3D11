@echo off

PUSHD bin

CL -nologo -WX -W4 -wd4100 -wd4189 -FC -Zi ..\src\win32_sokoban.cpp /Fesokoban
POPD

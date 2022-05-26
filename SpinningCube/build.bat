@echo off

IF NOT EXIST bin MKDIR bin
PUSHD bin

SET WARNING_FLAGS=-WX -W4 -wd4100 -wd4189 -wd4101
SET COMPILER_FLAGS=-nologo -FC -Zi -D_DEBUG

CL %WARNING_FLAGS% %COMPILER_FLAGS% ..\src\main.cpp /FeHelloCube.exe
POPD

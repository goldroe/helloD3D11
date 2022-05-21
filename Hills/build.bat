@echo off

IF NOT EXIST bin MKDIR bin
PUSHD bin

SET WARNING_FLAGS=-WX -W4 -wd4100 -wd4189 -wd4101
SET COMPILER_FLAGS=-nologo -FC -EHsc -Zi -DD3D11_DEBUG

CL %WARNING_FLAGS% %COMPILER_FLAGS% ..\src\main.cpp ..\src\grid.cpp /FeHills.exe
POPD

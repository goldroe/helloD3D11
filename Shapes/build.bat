@echo off

IF NOT EXIST bin MKDIR bin
PUSHD bin

SET WARNING_FLAGS=-WX -W4 -wd4100 -wd4189 -wd4101
SET COMPILER_FLAGS=-nologo -FC -Zi -DD3D11_DEBUG /EHsc

CL %WARNING_FLAGS% %COMPILER_FLAGS% ..\src\main.cpp ..\src\shapes.cpp /FeHelloShapes.exe
POPD

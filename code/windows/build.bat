@echo off
IF NOT EXIST ..\..\build mkdir ..\..\build
IF NOT EXIST ..\..\build\windows mkdir ..\..\build\windows

fxc -nologo /T vs_5_1 /Vn test_vertex_bytes /Fh shaders/test_vertex.h shaders/test_vertex.hlsl 
fxc -nologo /T ps_5_1 /Vn test_pixel_bytes /Fh shaders/test_pixel.h shaders/test_pixel.hlsl 

pushd ..\..\build\windows

set compilerFlags=-MT -nologo -GR- -EHa- -Od -Oi -FC -Z7 /D _DEBUG /D _GNG_WINDOWS

cl %compilerFlags%  ../../code/game/gng_game.c -LD /Fegng.dll -link /PDB:gngdll.pdb /OPT:REF /EXPORT:updateGNGGame /EXPORT:getSoundSamplesGNGGame 
cl %compilerFlags% ../../code/windows/win_platform.cpp /Fegng.exe -link /OPT:REF /SUBSYSTEM:WINDOWS user32.lib D3d12.lib dxgi.lib

popd

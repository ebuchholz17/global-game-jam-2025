@echo off
IF NOT EXIST ..\..\build mkdir ..\..\build
IF NOT EXIST ..\..\build\windows mkdir ..\..\build\windows
pushd ..\..\build\windows

"..\..\tools\qmpack\build\qmpack.exe" "..\..\src\data\assets" "assets.qpk"

popd

@echo off
IF NOT EXIST ..\..\build mkdir ..\..\build
IF NOT EXIST ..\..\build\web mkdir ..\..\build\web
IF NOT EXIST ..\..\build\web\assets mkdir ..\..\build\web\assets

call tsc

pushd ..\..\build\web

call clang --target=wasm32 -g -nostdlib -fvisibility=hidden -Wl,--no-entry  -Wl,--export-dynamic -Wl,--import-memory -D _GNG_WEB -Wl,--allow-undefined-file=..\..\code\web\wasm.syms ..\..\code\web\web_platform.c -o gng.wasm 
xcopy /y /s ..\..\code\web\index.html index.html*
xcopy /y /s /e ..\..\assets assets

popd

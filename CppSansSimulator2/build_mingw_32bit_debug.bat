@echo off
mkdir ..\out
mkdir ..\out\Debug
mkdir ..\out\Debug\x64-mingw
g++ -std=c++17 -o ../out/Debug/x64-mingw/CppSansSimulator2.exe src/*.cpp^
 -I../external/SDL2/include/ -I../external/SDL2_image/include/^
 -I../external/SDL2_ttf/include/ -I../external/SDL2_mixer/include/^
 -L../external/SDL2/i686-w64-mingw32/lib/ -L../external/SDL2_image/i686-w64-mingw32/lib/^
 -L../external/SDL2_ttf/i686-w64-mingw32/lib/ -L../external/SDL2_mixer/i686-w64-mingw32/lib/^
 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
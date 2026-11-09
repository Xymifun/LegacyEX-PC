# MinecraftLegacyEX-PC-source

Source code of the PC version for LegacyEX. Keep in mind this build is barebones and mostly for developers to look at closer
To keep this legal, no game files are included and you need your Wii U Edition files to access this open-source version of the game

## What you need on Windows

Building on Windows isn't tested, and since I made this on Arch Linux it's currently only guaranteed for Arch Linux.
For Windows: Get the prebuilt zip from the Releases page instead, which has the game, prepare.py and a README.txt with the basic steps

## What you need on Linux

clang, lld, llvm, ninja, cmake, python 3, the mingw-w64 GCC runtime and Wine. On Arch Linux:

```
sudo pacman -S clang lld llvm ninja cmake python mingw-w64-gcc wine
```

## 1: Game files

Copies the game files from your dumps into assets/, assuming you have folders like "/Minecraft Wii U Edition [Game] [00050000101d9d00]/" and "/Minecraft Wii U Edition [Update] [0005000e101d9d00]/"

```
python3 tools/prepare.py --base "<base game dump>" --update "<update dump>"
```

## 2: Libraries (you only need to do this one time)

Downloads zlib, libpng, curl, boost, stb_image, wut, lua and opus into build/deps.

```
python3 tools/build/deps.py build/deps
```

## 3: Build the game

```
cmake -S tools/pc -B build/pc64 -G Ninja -DCMAKE_TOOLCHAIN_FILE=$PWD/tools/pc/mingw-x86_64.cmake
cmake --build build/pc64
```

## 4: Play (64-bit)

```
cd build/pc64
wine MinecraftPC.exe
```

## Note

Keep in mind this is barebones for developers who want a proper look at the source code.
Various things like the TU0 files, extra tutorial worlds, title logos and other optional cosmetic content aren't included on this page due to copyright
However, if you have a hold of the "MinecraftLegacyEX-PC-assets.zip" file, you can use "python3 tools/prepare.py --base <base game dump> --update <update dump> [--assets-zip <assets zip>]"

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

Keep in mind this is barebones for developers who want a proper look at the source code (to check up on what's under the hood and how reliable the code is).

Various things like the TU0 files, extra tutorial worlds, title logos and other optional cosmetic content aren't included on this page due to copyright
However, if you have a hold of the "MinecraftLegacyEX-PC-assets.zip" file, you can use "python3 tools/prepare.py --base <base game dump> --update <update dump> [--assets-zip <assets zip>]"

## Decompilation

This was made from an unfinished decompilation released by Jacobwasbeast at https://github.com/Jacobwasbeast/mcwiiu-reconstructed. The edited version is also known as "LCEReborn"
The decompilation was made with AI, which this PC Port is built from, so consider this build touched by Generative AI by default unlike other purely human-coded versions like neoLegacy.

For a detailed list for parts I got additional help with:
###### Initially tearing out all the load-bearing Wii U-dependent stuff so it can boot on PC
###### Improving my mouse function so it behaves like an actual mouse instead of moving around like a joystick
###### Sorting the glyphs for the Xbox / PS3 (the graphics were messy in the Wii files)
###### Fixing my layout for the controller keyboard
###### Fixing lighting issues when implementing world sizes (had to find the specific places where lighting was locked in by world size)
###### Correcting issues with the OG decompilation (essentially finalizing it) because the minigame function matches were unfinished
###### Getting through the web of many minigame crashes
###### Figuring out why my friends kept spawning underground for Cavern
###### Adding a ton of proper logging features
###### Making sure fixes for permanent bugs like the Glide Ghost issue were actually correct
###### Fixing the friends system since this kept messing up badly
###### Fixing crashes for those trying to run it on Android (memory heaps)
###### Help with viewport scaling
###### Fixing my layout for the controller keyboard (specifically the special characters)
###### Figuring out the Golden Age water change
###### Handling Skin Packs that weren't from the Wii U edition
###### Upgrading the game to 64-bit (I took a lot of the code from LCEReborn)
###### Fixing audio issues with 64-bit
###### Details of soft anticheat, especially for making sure there's no false positives
###### Making sure the different variants of controllers can be detected properly
###### Skin size fallbacks
###### Small help with skin animation fallbacks (for Emerald Launcher Skins)
###### Basic Python script that downloads the open-source libraries both builds compile against (deps.py) is from LCEReborn
###### Sample native mod

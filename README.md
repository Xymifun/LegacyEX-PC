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

# Features of LegacyEX

## For a list of changes I made from the raw decompilation:

Works on PC

Fullscreen support

F3 menu

Compatible with most keyboard/mouse capabilities

Compatible with various controllers (for the Wii U, PS3, PS4, Xbox 360, Xbox One, etc.)

Added a scuffed in-game keyboard for controller users

FOV can be changed from 30 to 100

Framerate can be changed from 30 to Unlimited

Render Distance can be changed from 2 to 32

Simulation Distance can be changed from 5 to 16

Render Scale can also be lowered (this barely does anything)

World Sizes can be changed from Classic up to Large

Split-screen works (although used as placeholder guest players; there's no coherent alternative I can think of)

Added LCEOnline support (later added Online Multiplayer features, but it should still work here)

Changed every single mention of the intended device to "Legacy Edition" and "PC"

Java-style chat (my best attempt)

Extra options for the Solo Glide (for speedrunners; speedrunning this on PC was essentially my leading motivation for this project)

Replaced the defunct "Minecraft Store" with "Online Multiplayer"

LAN play via methods like Port Forwarding can easily be done. Requires a main host

Rendering fixes to banners, Horses, Fish, Drowned, Phantoms, UI masks, distant terain (caused when I was adding world sizes), loading bar progress

Fixed visual bug with End Rod in Creative menu

Performance adjustments

Saving should be 20x faster

Fixed bug where Battle Minigame chests were populated with just apples (???)

Made it so maps in Minigames load way faster

Fixed bug where my friends spawned under the floor in the Cavern Battle map

Fixed bug where the Glide Ghost could get messed up and start ~10 seconds ahead, permanently (until you deleted that save file)

Upgraded game to 64-bit

New "Massive" World Size (10,240 x 10,240) mostly for big SMP servers

Normal Worlds can now support over 8 players (24 total)

Higher mob caps for breeding, spawn eggs, villagers, wolves, etc.

In the creative menu, you can get rid of items easily

Adaptive aspect ratio for the display

Added the many skin packs the Wii U Edition never had (from 4J’s Xbox 360 and PS4 files)

Added a few extra worlds from the PS4 Mash-Ups

Emerald Launcher compatibility with Custom Skins and SOME Workshop skins

Created a 24/7 Survival SMP Server that you can play once connected to mclegacy.space

Soft Anticheat

"Public World Title" option that can also display your world name in the Join menu

Fixed water transparency

Fixed red tint when hitting mob

Lever hitbox fixed

Fixed offhand slot being on the wrong side

Fixed interaction with armor stands

Patched a retail duplication bug

Patched a retail world crashing bug

Fixed some visual glitches in 6 texture packs

Brought back the native Voice Chat

Game Chat has been changed to Voice Chat, and you are able to configure your input and output settings

There is a new Proximity Chat setting

Hosts can force or disable Proximity Chat in-game

You can finally change keybinds properly in-game

Splitscreen players (Player2, Player3, Player4) now have persistent saves

You can change the names of splitscreen players in their identity files (AppData --> Roaming)

Basic mod support

## Customizable Golden Age options (personal favorite):

Old block textures, item visuals, models (pig, boat, bed, chest, Enderman) and sounds (hurt, bow, explosion, doors, TNT)

Old Water

Old Lava

Title Logo selectors with Default, Wii U, Xbox 360, Xbox One, PS3, PS4, PS Vita

Historical Tutorial Worlds with Latest, TU46, TU31, TU19, TU14, TU12, TU9, TU7, TU5, TU3, TU1, Pre-Release

Historical Tutorial Worlds come with their own native rulesets and boundaries (except for Pre-Release, there's no source)

Historical Panorama Variants with Latest, TU46, TU31, TU19, TU14, TU9, TU7, TU5, TU1, Pre-Release

## Crashes:

Fixed crash in Battle Minigame

Fixed crash in Minigame where a Guest leaving crashes the Host

Fixed crash in Tumble where the raw decompilation made some mistakes with the function names

Fixed crash with spawn eggs

Fixed crash where Minigames could be stuck waiting for the data to load

Fixed crash caused by Minigame map loading taking too long

Fixed many more crashes that were tied to these issues

## Requires Discord:

Added a very important mclegacy.space IP for Online Multiplayer that allows for a hostless lobby session that can easily support 400-1000 people around the clock

Connected to a Discord Bot that gives a small code for authentication

Added full functionality for adding Friends in-game via Discord

All worlds in mclegacy.space are public by default unless if you toggle the "Friends Only" option

Checking Public Minigames shows an "(X Available)" label next to them so you don't blindly guess

# Note

Keep in mind this is barebones for developers who want a proper look at the source code (to check up on what's under the hood and how reliable the code is).

Various things like the TU0 files, extra tutorial worlds, title logos, panoramas and other optional cosmetic content aren't included on this page due to copyright
If you have a "MinecraftLegacyEX-PC-assets.zip" file, you can use "python3 tools/prepare.py --base <base game dump> --update <update dump> [--assets-zip <assets zip>]"

## Decompilation

This was made from an unfinished decompilation released by Jacobwasbeast at https://github.com/Jacobwasbeast/mcwiiu-reconstructed. The edited version is also known as "LCEReborn"
The decompilation was made with AI, which this PC Port is built from, so consider this build touched by Generative AI by default unlike other purely human-coded versions like neoLegacy.

For a detailed list for parts I got additional help with (keep in mind this list was made after I finished, from memory):
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

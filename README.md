# GooseBot
Implementation of Zerg AI starcraft bot using the BasicSc2Bot Template SC2 Bot for CMPUT 350 at UAlberta.

This bot works with our fork of [Sc2LadderServer](https://github.com/solinas/Sc2LadderServer) which will be used to run the tournament at the end of the term. It should help you
set up the build process with the correct version of SC2 API so you can focus on creating your bot.

# Developer Install / Compile Instructions
## Requirements
* [CMake](https://cmake.org/download/)
* Starcraft 2 ([Windows](https://starcraft2.com/en-us/)) ([Linux](https://github.com/Blizzard/s2client-proto#linux-packages)) 
* [Starcraft 2 Map Packs](https://github.com/Blizzard/s2client-proto#map-packs)

## Windows

Download and install [Visual Studio 2019](https://www.visualstudio.com/downloads/) if you need it. Building with Visual Studio 2019 not yet supported.

```bat
:: Clone the project
$ git clone --recursive https://github.com/CMPUT-350-Fall-2022/STARCRAFT-GooseBot.git
$ cd STARCRAFT-GooseBot

:: Create build directory.
$ mkdir build
$ cd build

:: Generate VS solution.
$ cmake ../ -G "Visual Studio 16 2019"

:: Build the project using Visual Studio.
$ start GooseBot.sln
```

## Mac

Note: Try opening the SC2 game client before installing. If the game crashes before opening, you may need to change your Share name:
* Open `System Preferences`
* Click on `Sharing`
* In the `Computer Name` textfield, change the default 'Macbook Pro' to a single word name (the exact name shouldn't matter, as long as its not the default name)

```bat
:: Clone the project
$ git clone --recursive https://github.com/laingkob/STARCRAFT-GooseBot.git
$ cd STARCRAFT-GooseBot

:: Create build directory.
$ mkdir build
$ cd build

:: Generate a Makefile
:: Use 'cmake -DCMAKE_BUILD_TYPE=Debug ../' if debug info is needed
$ cmake ../

:: Build
$ make
```

## Linux
The Linux version is headless, meaning that you will not be able to see your bot 
First, download the [Linux package](https://github.com/Blizzard/s2client-proto#linux-packages).
Unzip it to your home directory. 
The directory should read as `/home/<USER>/StarCraftII/`.

Rename the `Maps` directory to lowercase, and place any downloaded maps inside this directory:
```bash
$ mv /home/<USER>/StarCraftII/Maps /home/<USER>/StarCraftII/maps
```

Finally, create a directory (note the added space) which contains a file `ExecuteInfo.txt`, which lists the executable directory:
```bash
$ mkdir "/home/<USER>/StarCraft II"
$ echo "executable = /home/<USER>/StarCraftII/Versions/Base75689/SC2_x64" > "/home/<USER>/StarCraft II/ExecuteInfo.txt"
```
The `Base75689` will need to match the correct version which matches the version you downloaded. To check, navigate to `/home/<USER>/StarCraftII/Versions/`.

Remember to replace `<USER>` with the name of your user profile.

# Playing against the built-in AI

In addition to competing against other bots using the [Sc2LadderServer](https://github.com/solinas/Sc2LadderServer), this bot can play against the built-in
AI by specifying command line argurments.

For example,

```
./GooseBot.exe -c -a zerg -d Hard -m CactusValleyLE.SC2Map
```

will result in the bot playing against the zerg built-in AI on hard difficulty on the map CactusValleyLE.

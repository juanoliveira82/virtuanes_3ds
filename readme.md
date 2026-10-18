<p align="center">
  <img src="assets/splash/splash_logo.png" alt="VirtuaNES Logo" >
</p>

**A fast, high-compatibility NES and Famicom emulator for the Nintendo 3DS family.**

Full 60 FPS on almost every game, on Old 3DS, New 3DS and 2DS.

## Highlights

- **A huge game library.** 197 of the 256 iNES mapper numbers are supported, among them MMC1, 3, 5 and 6 and VRC1, 2, 3, 4, 6 and 7, plus the mappers behind many multicarts, ROM hacks and homebrew. [docs/mappers.md](docs/mappers.md) lists the few that are still missing.
- **Famicom Disk System.** Plays FDS games, with disk switching.
- **Demanding hacks.** Fixed MMC5 bugs let Rockman 4 Minus Infinity and Zelda: Legend of Link run.
- **Cheats and save states.** Game Genie cheats in simple .CHX files, plus save states and auto-save.
- **Made for the 3DS.** Fast forward up to 180 FPS, button mapping (including insert coin for VS games), dark and light themes and an animated splash screen.

Built for speed: VirtuaNES trades some cycle-level accuracy for full-speed play on 3DS hardware, so a desktop emulator such as FCEUX or Nestopia is the better choice when exactness matters more than portability.

This project continues from [GamingFanboy365's virtuanes3ds](https://github.com/GamingFanboy365/virtuanes3ds), which is a fork of bubble2k16's [emus3ds](https://github.com/bubble2k16/emus3ds), narrowed down to the VirtuaNES core.

## Screenshots

![alt tag](https://github.com/bubble2k16/emus3ds/blob/master/screenshots/VirtuaNES%20-%20Bionic%20Commando.bmp)

![alt tag](https://github.com/bubble2k16/emus3ds/blob/master/screenshots/VirtuaNES%20-%20Gradius%20II.bmp)

![alt tag](https://github.com/bubble2k16/emus3ds/blob/master/screenshots/VirtuaNES%20-%20Kirby's%20Adventure.bmp)

![alt tag](https://github.com/bubble2k16/emus3ds/blob/master/screenshots/VirtuaNES%20-%20Legend%20of%20Link.bmp)

![alt tag](https://github.com/bubble2k16/emus3ds/blob/master/screenshots/VirtuaNES%20-%20Rockman%204%20Minus%20Infinity.bmp)

![alt tag](https://github.com/bubble2k16/emus3ds/blob/master/screenshots/VirtuaNES%20-%20Super%20Mario%20Bros%203.bmp)

## Installation

You need a 3DS, 2DS or New 3DS that can run homebrew (usually with custom firmware). There are two ways to install: the `.3dsx` runs from the Homebrew Launcher without installing anything, and the `.cia` installs VirtuaNES as an app on the HOME menu.

VirtuaNES for 3DS is distributed as source code: you build it yourself (see [Building](#building)), which produces `virtuanes_3ds.3dsx`, `virtuanes_3ds.smdh` and `virtuanes_3ds.cia`.

### Homebrew Launcher (.3dsx)

1. Create the folder `/3ds/virtuanes_3ds` on your SD card and copy `virtuanes_3ds.3dsx` and `virtuanes_3ds.smdh` into it. The `.smdh` gives the emulator its icon and name in the launcher.
2. Put your ROMs (`.nes` or `.fds`) in any folder on the SD card.
3. Open the Homebrew Launcher and start VirtuaNES.

### CIA

1. Install `virtuanes_3ds.cia` with a CIA installer such as FBI. Installing a newer version over an older one updates it.
2. Put your ROMs (`.nes` or `.fds`) in any folder on the SD card.
3. Launch VirtuaNES from the HOME menu.

### FDS BIOS

This is only needed for Famicom Disk System games (`.fds` files). Regular `.nes` games work without it.

The Famicom Disk System was a disk-drive add-on for the Famicom, sold in Japan. To run its games, an emulator needs a copy of the system's BIOS, a small file usually called `disksys.rom`. The BIOS is copyrighted, so it isn't included: you have to provide your own copy.

1. Rename the BIOS to `disksys.rom`.
2. Place it in `/3ds/virtuanes_3ds/bios/disksys.rom` on your SD card.

The emulator can switch disks for games that use more than one.

## Using the emulator

1. Tap the bottom screen for the menu.
2. Use Up/Down to choose option, and A to confirm. (Hold down X while pressing Up/Down to Page Up/Page Down)
3. Use Left/Right to change between ROM selection and emulator options.
4. You can quit the emulator to your homebrew launcher / your CFW's home screen.

### Themes

The menu has a Dark theme and a Light theme. Choose it with Theme in the Appearance section of the Emulator tab at start-up, or in the Options tab while a game is running. The choice is saved with the other global settings. The Appearance section also has the font.

## .CHX cheat files

The .CHX is a cheat file format that you can create with any text editor. Each line in the file corresponds to one cheat, and is of the following format:

```
[Y/N],[CheatCode],[Name]
```

1. [Y/N] represents whether the cheat is enabled. Whenever you enable/disable it in the emulator, the .CHX cheat file will be modified to save your changes.
2. [CheatCode] must be an NES Game Genie 6- or 8-character cheat code. An NES Game-Genie code looks like this: **GXOUSUSE**.
3. [Name] is a short name that represents this cheat. Since this will appear in the emulator, keep it short (< 30 characters).

The .CHX must have the same name as your ROM. This is an example of a .CHX file:

Filename: **8x8 Eyes.CHX**

```
Y,GXOUSUSE,Most attacks won't damage Orin
Y,GXNGNOSE,Most attacks won't damage Cutrus
```

NOTE: If you enable a cheat, and then disable it, you may need to reset the game for the disabling to take effect.

## Development

### Building

The code builds with a current devkitARM (libctru 2.x). If you have devkitPro installed, run `make`, which produces `virtuanes_3ds.3dsx` and `virtuanes_3ds.cia`.

Without a local devkitPro, `tools/build.sh` runs the same `make` inside the official `devkitpro/devkitarm` Docker image and passes its arguments through, so `tools/build.sh clean` works too.

`make DEBUGOUT=1` turns on the `DEBUGOUT()` traces that are sprinkled through the VirtuaNES core and mappers (ROM header details at load time, unhandled mapper writes and so on). They go to `svcOutputDebugString`, which shows up in the emulator's log. The flag isn't tracked by the build, so run `make clean` when switching it on or off.

`make TRACE=1` is for debugging on real hardware, where there's no emulator log. The app appends a step-by-step log of start-up, ROM loading, sound start, the first frames, HOME menu events and shutdown to `virtuanes_trace.txt` at the root of the SD card, saving after every line so the log survives a freeze. A watchdog thread also logs the app's state every 2 seconds while it's in the background or closing. Like `DEBUGOUT`, run `make clean` when switching it.

### Running and debugging in Azahar

The [Azahar](https://azahar-emu.org/) 3DS emulator (the successor of Citra and Lime3DS) runs the `.3dsx` directly, which is the quickest way to try a change. If you have it installed, open `virtuanes_3ds.3dsx` from it, and put your ROMs on its emulated SD card, the `sdmc` folder in Azahar's user directory. Under an emulator VirtuaNES runs without sound, because Azahar doesn't implement the CSND sound service it uses.

`tools/azahar/run.sh` runs Azahar from the `linuxserver/azahar` Docker image instead, and can script it. By default it runs headless on a virtual display, presses 3DS buttons and takes screenshots of both screens at native size, which is handy for checking that a game boots and looks right after a mapper change:

```sh
tools/build.sh
tools/azahar/run.sh -r game.nes -- wait:8 key:a wait:5 shot:title key:start wait:3 shot:ingame
```

The ROMs given with `-r` are the only ones on the emulated SD card, so pressing A at the ROM menu loads the first one. Screenshots and the Azahar log (including `DEBUGOUT()` output) end up in `.azahar/out`. `tools/azahar/run.sh --help` lists all the steps and options, and `--gui` shows the emulator on your X11 display instead.

For a debugger, start the emulator with `-g`, which makes Azahar's GDB stub wait for a connection, and then attach devkitARM's GDB with `tools/azahar/gdb.sh`:

```sh
tools/azahar/run.sh -r game.nes -g -t 3600 &
tools/azahar/gdb.sh
(gdb) break Mapper004::Write
(gdb) continue
```

`tools/azahar/smoketest.py` checks the whole setup end to end. It builds a tiny test ROM that shows a blue screen and turns it red while A is held, runs it in Azahar and checks the screenshots.

`tools/azahar/mappertest.py` does the same for mappers: it generates a test ROM per mapper whose code sets the mapper's registers and checks what the CPU and PPU then see (every PRG and CHR bank starts with its own number) and how the nametables are mirrored. The screen turns green when every check passes, and red otherwise. Pass mapper numbers to run only those, e.g. `tools/azahar/mappertest.py 210`.

### Adding a mapper

Mappers live in `src/cores/virtuanes/NES/Mapper`, one class per mapper (`MapperNNN.h` and `MapperNNN.cpp`) deriving from `Mapper`. They are compiled as part of `src/cores/virtuanes/NES/MapperFactory.cpp`, which `#include`s every mapper header and source file, and whose `CreateMapper()` maps iNES mapper numbers (and UNIF board names) to the classes. So a new mapper needs its two files plus the two `#include`s and a `case` in `CreateMapper()`. The existing mappers such as `Mapper003` (CNROM) are the best reference for the bank switching helpers (`SetPROM_8K_Bank()`, `SetVROM_1K_Bank()` and friends).

To test a new mapper, add a test function and a `TESTS` entry to `tools/azahar/mappertest.py`. [docs/mappers.md](docs/mappers.md) lists the iNES mappers that VirtuaNES doesn't support yet, and which ones are the easiest to add.

### Splash screen artwork

The animated splash screen on the top screen is made from two PNG files in the `data` folder. The Makefile embeds them in the emulator, so they end up in both the `.3dsx` and the `.cia`:

- `assets/splash/splash_logo.png` is the logo, drawn at the centre. Use a PNG with transparency. Anything larger than the 400x240 screen is scaled down to fit.
- `assets/splash/splash_background.png` is the background, which scrolls upwards and repeats. 400 pixels wide (or more) and at least 240 tall works best. Make its top and bottom edges match, so the loop has no seam. It is darkened a little so that the logo stands out.

Replace the files and run `make`. Don't put any other kind of file in `assets/splash/`, because the Makefile embeds everything in it. The scrolling speed and the darkening of the background are constants at the top of `src/3ds/3dssplash.cpp`.

The images that come with the project are simple original artwork. Replace them with your own.

## Changelog
For a detailed list of changes and version history, see the [CHANGELOG.md](CHANGELOG.md).

## Use of AI

Parts of this project were written with the help of AI coding assistants, and it is better to say so openly.

- This fork was developed with the help of Anthropic's Claude. Claude helped write the color themes, the bottom bar with its button badges, the animated splash screen, some fixes to the build files and the documentation. The design decisions and the testing are the maintainer's.
- [GamingFanboy365](https://github.com/GamingFanboy365/virtuanes3ds), whose fork this project is based on, also used Claude in some of his commits, which are marked with a `Co-Authored-By` line.
- The VirtuaNES emulation core and the original 3DS port were written long before these tools existed, by the Norix team and by bubble2k16.

The code that was written with AI help is commented in detail, so that it can be read and reviewed like any other. If you find a problem, please open an issue.

## Credits

1. Norix team for the fantastic well-optimized VirtuaNES emulator
2. The FCEUX team and the other authors named in the mapper files (such as CaH4e3 and Xodnizel), for the mappers ported from FCEUX.
3. Mitsutaka Okazaki (emu2413), Mamiya (NEZplug) and the other authors credited by Norix, whose code is part of the VirtuaNES core.
4. bubble2k16, who ported VirtuaNES to the 3DS ([emus3ds](https://github.com/bubble2k16/emus3ds)), the base of this project.
5. Authors of the Citra 3DS Emulator team. Without them, this project would have been extremely difficult.
6. SG6000 for helping with debugging the nasty crashing bug.
7. Fellow forummers on GBATemp for the bug reports and suggestions for improvements.
8. [GamingFanboy365](https://github.com/GamingFanboy365/virtuanes3ds), whose fork this project is based on, for adding the new mappers 30, 37, 152, 153, 154, 155, 157, 158, 159, 207 and 210, for the build setup and the Azahar test tools, and for the fixes that make the emulator run on real hardware (start-up, sound, the HOME menu and exit).
9. matbo87, whose Snes9x 3DS fork inspired the themes, the bottom bar and the splash screen.

## License

VirtuaNES for 3DS is free software, distributed under the terms of the GNU General Public License, version 2. See [LICENSE](LICENSE). It comes without any warranty.

This is the license of the VirtuaNES source code: its author, Norix, released it under the GNU General Public License, version 2, as the readme of his source code package says. (The readme of the Windows executable calls the program freeware, which describes that program and not its source code.)

The project also contains work with its own copyright notices, which are kept in the source files:

- the mappers ported from FCEUX (for example 028, 036, 120 and 206), under the GNU GPL, version 2 or later;
- lodepng, by Lode Vandevenne (`src/3ds/3dslodepng.*`), under the zlib license;
- the code that Norix credits in the VirtuaNES core (see the credits above).

The source code is in this repository. The project doesn't include ROMs or BIOS files, and it doesn't distribute ready-made builds.

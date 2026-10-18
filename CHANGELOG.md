## Changelog

### v1.04

- Fixed building the CIA from the MSYS2 shell on Windows.
- Removed the box art and screenshot previews from the ROM list, and the tools/make_previews.py script.
- New user interface themes: Dark (based on the dark theme of matbo87's Snes9x 3DS) and Light (updated from the original look). Change the theme in the Appearance section of the Emulator tab, or in the Options. The choice is saved with the global settings.
- Dialogs, such as the theme picker and the confirmation dialogs, follow the colors of the selected theme.
- The Emulator tab of the start-up menu has a new Appearance section, with the theme and the font, next to Quit Emulator.
- The bottom bar of the menu shows colored A and B buttons next to Select and Back. The battery indicator is orange and turns green while the console is charging.
- Animated splash screen on the top screen while you browse the ROM list: a scrolling background behind the logo. Both images are PNG files built into the emulator (data/splash_logo.png and data/splash_background.png), so they are replaced by editing those files and running make.
- Fixed games hanging at start-up on real 3DS hardware, and the sound being silent there. A function that ended without returning a value turned into an endless loop with current compilers, and a missing return is now a build error.
- The sound stops while the HOME menu is open or the lid is closed, and comes back when you return to the game.
- Fixed the hang when closing the app from the HOME menu, and the "An error has occurred" message when exiting.
- With auto-savestate on, exiting now saves the state (it used to load it, losing the progress since the last game switch).
- Added `make TRACE=1`, a build that logs the start-up, the sound and the shutdown to the SD card, to help find freezes on real hardware.

### v1.03

- Builds with current devkitARM (libctru 2.x) on Linux, macOS and Windows.
- The top screen always runs in 2D mode (the 3D slider no longer changes it).
- Runs in the Azahar and Citra emulators (without sound).
- Box art and screenshot previews on the top screen in the ROM list, with room for NES, Famicom and Famicom Disk System covers (removed in v1.04).
- New mappers: 30 (UNROM 512, including games that save by reprogramming their flash ROM; those saves go in a .flash file next to the ROM), 37 (Super Mario Bros. + Tetris + Nintendo World Cup), 152, 153 (Famicom Jump II), 154 (Devil Man), 155 (MMC1A), 157 (Datach, without the barcode reader), 158 (Alien Syndrome), 159 (Bandai 24C01 EEPROM games), 207 (Fudou Myouou Den) and 210 (Namco 175/340: Family Circuit '91, Splatterhouse Wanpaku Graffiti, Famista '92-'94 and others).
- The CIA now has a title version (1.0.3), so it installs as an upgrade of older versions.

### v1.02

- Fixed bug in MMC5 mapper that was causing Castlevania 3's graphics to corrupt.
- Optimized rendering to 16-bit buffer to reduce cache misses, and minor optimizations for MMC5 rendering.
- Fixed occassional crashing bug when loading a ROM.

### v1.01

- Fixed cheats (previously cheats couldn't be activated whatever you did)
- Fixed mapper problem in Kabuki Quantum Fighter

### v1.00

- Added key mappings to fast forward (capped at 180 FPS), and open the emulator menu.
- Updated configuration menu
- Added feature to auto-save state
- Migrated and fixed some mappers from VirtuaNESEx (thanks to the authors of VirtualNESEx)
- The following mappers are fixed/added: 4, 7, 16, 23, 34, 47, 49, 52, 65, 69, 71, 73, 74, 86, 91, 92, 121, 132, 141, 148, 150, 162, 163, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 189, 190, 194, 198, 199, 212, 235, 237, 240, 241, 252
- Fixed VS mapper for Super Mario Bros
- Added ability to map any 3DS key to insert coin for VS games
- Fixed Lagrange Point VRC7 emulation for the Old 3DS by playing at 20KHz. The old 3DS was too slow to render sounds at 32KHz or even at 22KHz.
- Minor optimizations for sound generation.
- A number of games are fixed due to the mapper updates. Examples are: Crisis Force (Japan), Feng Yun (Ch), San Guo Zhi (Ch), and many others.

### v0.92

- Migrated some mappers from VirtuaNES Up (thank you dragon2snow, and authors of VirtualNES Plus)
- The following mappers are fixed: 0, 15, 16, 25, 27, 58, 62, 74, 83, 111, 163, 187, 193, 194, 199, 241
- Added Mapper 28, 35, 36, 120, 156, 162, 170, 175, 206, 209, 211, 216, 220, 253
- Fixed rendering problem with Mother (J) and Marble Madness
- Detect and auto-patch correct header numbers for certain ROMs (database of corrected mapping for ROMs taken from FCEUX)
- Games that are now working on this version:
  > Final Fantasy VII Demake (both English / Chinese versions)
  > DBZ RPG 2/3 (English)
  > Donkey Kong Country 4
  > Super Contra 3
  > Super Samurai Spirits 2
  > Action 53
  > Probably a few others.
- Added settings to allow players to switch between global/local config for audio, button mapping and rapid fire.

### v0.91

- Enabled support of FDS ROMS without the standard FDS headers.
- Added feature to allow players to switch disks for FDS games.
- Fixed faint vertical lines appearing at the left/right edges of the screen.
- Enabled APU frame counter IRQ (fixes Qix, and probably many others)
- Used 64-bit integer for total cycle counter. This should fix sound problems after running a game for too long.
- Improved sound synchronization - minimizes occurrence of songs going out of tune or just behaving oddly.
- Improved digital sample playback synchronisation. Fixes Skate of Die 2’s intro song.
- Allow mapping of 3DS buttons to the NES Start/Select buttons.
- Added cropped 4:3 Fit and cropped fullscreen modes.
- Fixed some issues with MMC5 mappers when powering up.
- Fixed issues handling of CHR-RAM.
- Updated MMC5 mapper to handle register $5130 correctly.
- Updated MMC5 mapper to write to XRAM so that it can be executed from CPU.
- Updated MMC5 mapper to save /load state of the updated internal registers.
- Fixes Zelda - Legend of Link and Rockman 4 Minus Infinity MMC5 build.
- Minor optimizations by removing checks for clock tests.
- Fixed Smash TV D-pad problem
- Fixed bug with the saving of the auto-fire rate.
- Fixed bug where the game-specific settings never seemed to get saved.
- Fixed bug where the emulator freezes when you select a ROM after selecting one that fails to load.
- Fixed problem of a full black screen for right-eye when using a New 3DS.
- Fixed bug that causes crashing on certain 3DSes.

### v0.90

- First release.
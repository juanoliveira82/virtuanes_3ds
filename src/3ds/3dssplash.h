#ifndef _3DSSPLASH_H_
#define _3DSSPLASH_H_

//---------------------------------------------------------
// Animated splash screen on the top screen while the ROM
// menu is shown.
//
// The artwork is built into the emulator from two PNG files
// in the data folder of the project:
//
//   data/splash_logo.png
//       The logo, drawn centred to the screen. Use a PNG with
//       transparency. Anything bigger than the 400x240 screen
//       is scaled down to fit.
//
//   data/splash_background.png
//       Background that scrolls upwards, tiled both ways.
//       400 pixels wide (or more) is best, and any height
//       from 240 up. Make its top and bottom edges match,
//       because it repeats. It is darkened a little so
//       that the logo stands out.
//
// Edit the PNGs and run make; the Makefile embeds them in
// the .3dsx and the .cia (rule %.png.o). Don't put any other
// kind of file in data/ without adding a rule for it.
//---------------------------------------------------------

// Call before showing the ROM menu.
void splash3dsBegin();

// Draws the next frame of the animation. Call once per
// frame while the ROM menu is running.
void splash3dsTick();

// Call after leaving the ROM menu.
void splash3dsEnd();

#endif

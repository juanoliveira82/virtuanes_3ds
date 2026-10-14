#include <cstring>
#include <string.h>
#include <stdio.h>
#include <3ds.h>

#include "3dstypes.h"
#include "3dsexit.h"
#include "3dsmenu.h"
#include "3dsgpu.h"
#include "3dsui.h"
#include "3dsmain.h"

#include "3dsinterface.h"
#include "3dstheme.h"

#define CONSOLE_WIDTH           40
#define MENU_HEIGHT             (14)
#define DIALOG_HEIGHT           (5)

#define ANIMATE_TAB_STEPS 3


typedef struct
{
    SMenuItem   *MenuItems;
    char        SubTitle[256];
    char        *Title;
    char        *DialogText;
    int         ItemCount;
    int         FirstItemIndex;
    int         SelectedItemIndex;
} SMenuTab;


SMenuTab            menuTab[5];
SMenuTab            dialogTab;

int                 menuTabCount;
int                 currentMenuTab = 0;
bool                isDialog = false;

bool                transferGameScreen = false;
int                 transferGameScreenCount = 0;

bool                swapBuffer = true;

// Called once per frame, whenever the menu swaps the buffers.
static void         (*frameCallback)() = NULL;


//-------------------------------------------------------
// Common options
//-------------------------------------------------------
SMenuItem optionsForNoYes[] = {
    MENU_MAKE_ACTION(0, "No"),
    MENU_MAKE_ACTION(1, "Yes"),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForOk[] = {
    MENU_MAKE_ACTION(0, "OK"),
    MENU_MAKE_LASTITEM  ()
};

SMenuItem optionsForTheme[3] = {
    MENU_MAKE_DIALOG_ACTION(THEME_LIGHT, "Light mode", ""),
    MENU_MAKE_DIALOG_ACTION(THEME_DARK, "Dark mode", ""),
    MENU_MAKE_LASTITEM  ()
};


//-------------------------------------------------------
// Sets a flag to tell the menu selector
// to transfer the emulator's rendered frame buffer
// to the actual screen's frame buffer.
//
// Usually you will set this to true during emulation,
// and set this to false when this program first runs.
//-------------------------------------------------------
void menu3dsSetTransferGameScreen(bool transfer)
{
    transferGameScreen = transfer;
    if (transfer)
        transferGameScreenCount = 2;
    else
        transferGameScreenCount = 0;

}



//-------------------------------------------------------
// Draw a black screen with a specified opacity.
//-------------------------------------------------------
void menu3dsDrawBlackScreen(float opacity)
{
    ui3dsDrawRect(0, 0, 320, 240, 0x000000, opacity);    
}


//-------------------------------------------------------
// Swap buffers and wait for vertical blank.
//-------------------------------------------------------
void menu3dsSwapBuffersAndWaitForVBlank()
{
    // Without the GPU (the system took it to close the app), the transfer
    // and the waits would never complete.
    if (!gspHasGpuRight())
        return;

    // The splash animation writes to the top screen
    if (frameCallback)
        frameCallback();

    if (transferGameScreenCount)
    {
        gpu3dsTransferToScreenBuffer();
		if (emulator.isReal3DS)
			gspWaitForPPF();
        transferGameScreenCount --;
    }
    if (swapBuffer)
    {
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    else
    {
        gspWaitForVBlank();
    }
    
    swapBuffer = false;
}


//-------------------------------------------------------
// Draws menu items to the screen.
//-------------------------------------------------------
void menu3dsDrawItems(
    SMenuTab *currentTab, int horizontalPadding, int menuStartY, int maxItems,
    int selectedItemBackColor,
    int selectedItemTextColor, 
    int selectedItemDescriptionTextColor, 
    int checkedItemTextColor, 
    int normalItemTextColor,
    int normalItemDescriptionTextColor,
    int disabledItemTextColor, 
    int headerItemTextColor, 
    int subtitleTextColor)
{
    int fontHeight = 13;
    char gauge[52];
    char menuTextBuffer[512];
    char selectedTextBuffer[512];
    
    // Display the subtitle
    if (currentTab->SubTitle[0])
    {
        maxItems--;
        snprintf (menuTextBuffer, 511, "%s", currentTab->SubTitle);
        ui3dsDrawStringWithNoWrapping(20, menuStartY, 300, menuStartY + fontHeight, 
            subtitleTextColor, HALIGN_LEFT, menuTextBuffer);
        menuStartY += fontHeight;
    }

    int line = 0;
    int color = 0xffffff;

    // Draw all the individual items
    //
    for (int i = currentTab->FirstItemIndex;
        i < currentTab->ItemCount && i < currentTab->FirstItemIndex + maxItems; i++)
    {
        int y = line * fontHeight + menuStartY;

        if (currentTab->MenuItems[i].Text == NULL)
            snprintf (menuTextBuffer, 511, "");
        else
            snprintf (menuTextBuffer, 511, "%s", currentTab->MenuItems[i].Text);

        // Draw the selected background 
        //
        if (currentTab->SelectedItemIndex == i)
        {
            ui3dsDrawRect(0, y, 320, y + 14, selectedItemBackColor);
        }
        
        if (currentTab->MenuItems[i].Type == MENUITEM_HEADER1)
        {
            color = headerItemTextColor;
            ui3dsDrawStringWithNoWrapping(horizontalPadding, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_LEFT, menuTextBuffer);
            ui3dsDrawRect(horizontalPadding, y + fontHeight - 1, 320 - horizontalPadding, y + fontHeight, color);
        }
        else if (currentTab->MenuItems[i].Type == MENUITEM_HEADER2)
        {
            color = headerItemTextColor;
            ui3dsDrawStringWithNoWrapping(horizontalPadding, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_LEFT, menuTextBuffer);
        }
        else if (currentTab->MenuItems[i].Type == MENUITEM_DISABLED)
        {
            color = disabledItemTextColor;
            ui3dsDrawStringWithNoWrapping(horizontalPadding, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_LEFT, menuTextBuffer);
        }
        else if (currentTab->MenuItems[i].Type == MENUITEM_ACTION)
        {
            color = normalItemTextColor;
            if (currentTab->SelectedItemIndex == i)
                color = selectedItemTextColor;
            ui3dsDrawStringWithNoWrapping(horizontalPadding, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_LEFT, menuTextBuffer);

            color = normalItemDescriptionTextColor;
            if (currentTab->SelectedItemIndex == i)
                color = selectedItemDescriptionTextColor;
            if (currentTab->MenuItems[i].Description != NULL)
            {
                snprintf(menuTextBuffer, 511, "%s", currentTab->MenuItems[i].Description);
                ui3dsDrawStringWithNoWrapping(horizontalPadding, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_RIGHT, menuTextBuffer);
            }
        }
        else if (currentTab->MenuItems[i].Type == MENUITEM_CHECKBOX)
        {
            if (currentTab->MenuItems[i].Value == 0)
            {
                color = disabledItemTextColor;
                if (currentTab->SelectedItemIndex == i)
                    color = selectedItemTextColor;
                ui3dsDrawStringWithNoWrapping(horizontalPadding, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_LEFT, menuTextBuffer);

                snprintf(menuTextBuffer, 511, "\xfe");
                ui3dsDrawStringWithNoWrapping(280, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_RIGHT, menuTextBuffer);
            }
            else
            {
                color = normalItemTextColor;
                if (currentTab->SelectedItemIndex == i)
                    color = selectedItemTextColor;
                ui3dsDrawStringWithNoWrapping(horizontalPadding, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_LEFT, menuTextBuffer);

                snprintf(menuTextBuffer, 511, "\xfd");
                ui3dsDrawStringWithNoWrapping(280, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_RIGHT, menuTextBuffer);
            }
        }
        else if (currentTab->MenuItems[i].Type == MENUITEM_GAUGE)
        {
            color = normalItemTextColor;
            if (currentTab->SelectedItemIndex == i)
                color = selectedItemTextColor;

            ui3dsDrawStringWithNoWrapping(horizontalPadding, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_LEFT, menuTextBuffer);

            int max = 40;
            int diff = currentTab->MenuItems[i].GaugeMaxValue - currentTab->MenuItems[i].GaugeMinValue;
            int pos = (currentTab->MenuItems[i].Value - currentTab->MenuItems[i].GaugeMinValue) * (max - 1) / diff;

            for (int j = 0; j < max; j++)
                gauge[j] = (j == pos) ? '\xfa' : '\xfb';
            gauge[max] = 0;
            ui3dsDrawStringWithNoWrapping(245, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_RIGHT, gauge);
        }
        else if (currentTab->MenuItems[i].Type == MENUITEM_PICKER ||
            currentTab->MenuItems[i].Type == MENUITEM_PICKER2)
        {
            color = normalItemTextColor;
            if (currentTab->SelectedItemIndex == i)
                color = selectedItemTextColor;

            ui3dsDrawStringWithNoWrapping(horizontalPadding, y, 160, y + fontHeight, color, HALIGN_LEFT, menuTextBuffer);

            if (currentTab->MenuItems[i].Type == MENUITEM_PICKER)
            {
                snprintf(selectedTextBuffer, 511, "????");
                if (currentTab->MenuItems[i].PickerItems != NULL)
                {
                    int j = 0;
                    SMenuItem *pickerItems = (SMenuItem *)currentTab->MenuItems[i].PickerItems;
                    for (j = 0; pickerItems[j].Type == MENUITEM_ACTION; j++)
                    {
                        if (pickerItems[j].ID == currentTab->MenuItems[i].Value)
                        {
                            snprintf(selectedTextBuffer, 511, "%s", pickerItems[j].Text, j);
                        }
                    }
                    ui3dsDrawStringWithNoWrapping(160, y, 320 - horizontalPadding, y + fontHeight, color, HALIGN_RIGHT, selectedTextBuffer);
                }
            }
        }

        line ++;
    }


    // Draw the "up arrow" to indicate more options available at top
    //
    if (currentTab->FirstItemIndex != 0)
    {
        ui3dsDrawStringWithNoWrapping(320 - horizontalPadding, menuStartY, 320, menuStartY + fontHeight, disabledItemTextColor, HALIGN_CENTER, "\xf8");
    }

    // Draw the "down arrow" to indicate more options available at bottom
    //
    if (currentTab->FirstItemIndex + maxItems < currentTab->ItemCount)
    {
        ui3dsDrawStringWithNoWrapping(320 - horizontalPadding, menuStartY + (maxItems - 1) * fontHeight, 320, menuStartY + maxItems * fontHeight, disabledItemTextColor, HALIGN_CENTER, "\xf9");
    }
    
}


//-------------------------------------------------------
// Draws a round button badge with the letter of a 3DS button.
//
// The bottom bar shows hints like "(A) Select" and
// "(B) Back", where (A) and (B) are small colored circles
// with a white letter, like the buttons of the console.
//
// The circle is drawn pixel by pixel, with a smooth
// ("anti-aliased") edge. A circle made only of solid
// pixels looks jagged at this size, so the pixels along
// the edge are drawn in a color between the circle's
// color and the background's, depending on how much of
// each pixel is inside the circle
//-------------------------------------------------------
#define BADGE_SIZE          12      // Diameter in pixels
#define BADGE_SUBSAMPLES    8       // Points tested inside each pixel, in each direction

// The background colors of the badges.
#define BADGE_RED       0x800D1D    // Button A
#define BADGE_YELLOW    0x999409    // Button B
#define BADGE_BLUE      0x0D5280    // Button X
#define BADGE_GREEN     0x0D8014    // Button Y
// Button Y is not used in the menu.

// Returns the width of a string in pixels. It is defined in 3dsui.cpp,
// but isn't declared in 3dsui.h, so we declare it here.
int ui3dsGetStringWidth(char *s, int startPos = 0, int endPos = 0xffff);

// The letters of the badges always use the Ronda font, in bold, whatever
// font the user chose for the menu. These are the font in use and the
// widths of its characters, defined in 3dsui.cpp.
extern uint8 *fontBitmap;
extern uint8 *fontWidth;

// Index of the Ronda font, as used by ui3dsSetFont.
#define BADGE_FONT          1

// Moves the letter left (negative) or right (positive), and up (negative)
// or down (positive), inside the circle.
#define BADGE_LETTER_DX     1
#define BADGE_LETTER_DY     -1

// How much of each pixel of the badge is inside the circle, from
// 0 (none of it) to 255 (all of it). Indexed as [row][column].
static unsigned char badgeCoverage[BADGE_SIZE][BADGE_SIZE];

// Set when badgeCoverage has been filled in.
static bool badgeCoverageReady = false;

//-------------------------------------------------------
// Works out how much of each pixel of the badge is inside
// the circle, and keeps the result in badgeCoverage.
//
// Only integers are used, so there is no floating point
// work on the 3DS CPU.
//-------------------------------------------------------
static void menu3dsPrepareBadge()
{
    // The circle fills the whole badge, so its center is at
    // (BADGE_SIZE / 2, BADGE_SIZE / 2) and its radius is
    // BADGE_SIZE / 2 pixels.
    //
    // To avoid fractions, distances are measured in units of
    // 1 / (2 * BADGE_SUBSAMPLES) of a pixel.
    const int radius = BADGE_SIZE * BADGE_SUBSAMPLES;

    for (int py = 0; py < BADGE_SIZE; py++)
        for (int px = 0; px < BADGE_SIZE; px++)
        {
            // Number of test points of this pixel that are inside the circle.
            int inside = 0;
            for (int sy = 0; sy < BADGE_SUBSAMPLES; sy++)
                for (int sx = 0; sx < BADGE_SUBSAMPLES; sx++)
                {
                    // Position of the test point relative to the center of the circle.
                    int dx = 2 * (px * BADGE_SUBSAMPLES + sx) + 1 - radius;
                    int dy = 2 * (py * BADGE_SUBSAMPLES + sy) + 1 - radius;

                    // Pythagoras: the point is inside the circle if its
                    // distance to the center is at most the radius. Both
                    // sides are squared, so no square root is needed.
                    if (dx * dx + dy * dy <= radius * radius)
                        inside++;
                }
            badgeCoverage[py][px] = inside * 255 / (BADGE_SUBSAMPLES * BADGE_SUBSAMPLES);
        }

    badgeCoverageReady = true;
}

//-------------------------------------------------------
// Draws a round button badge: a circle of the given color
// with a white letter in the middle.
//
//   x, y        top left corner of the badge. Like everything
//               else in the menu, it is relative to the
//               current translation (see ui3dsSetTranslate),
//               which is what makes the menu slide in and out.
//   color       color of the circle, in RGB888 (0xRRGGBB).
//   backColor   color of the background behind the badge, in RGB888.
//               The edge of the circle is blended with it, so it
//               has to be a flat color: the bottom bar's color.
//   letter      the text inside the circle, normally a single
//               letter such as "A".
//-------------------------------------------------------
void menu3dsDrawButtonBadge(int x, int y, int color, int backColor, char *letter)
{
    // Work out the coverage table the first time a badge is drawn.
    if (!badgeCoverageReady)
        menu3dsPrepareBadge();

    // Split both colors into their red, green and blue parts (0..255),
    // so that they can be blended below.
    int r = (color >> 16) & 0xff, g = (color >> 8) & 0xff, b = color & 0xff;
    int br = (backColor >> 16) & 0xff, bg = (backColor >> 8) & 0xff, bb = backColor & 0xff;

    // Draw the circle one row at a time, from top to bottom.
    for (int py = 0; py < BADGE_SIZE; py++)
    {
        // The pixels completely inside the circle are drawn in one go,
        // and the ones at the edge one by one, blended with the background.
        int first = -1, last = -1;
        for (int px = 0; px < BADGE_SIZE; px++)
        {
            // How much of this pixel is inside the circle (0..255).
            int a = badgeCoverage[py][px];

            // Completely outside: leave the background as it is.
            if (a == 0)
                continue;

            // Completely inside: just note it. It is drawn after this loop.
            if (a == 255)
            {
                if (first < 0)
                    first = px;
                last = px;
                continue;
            }

            // On the edge: mix the circle's color and the background's, in
            // the proportion given by the coverage. With a = 0 the result
            // would be the background, with a = 255 the circle's color.
            //
            // The blend is done here, with the known background, instead
            // of using the alpha version of ui3dsDrawRect, because that
            // one only has 9 levels of transparency (MAX_ALPHA is 8), and
            // this gives a smoother edge.
            //
            // Integer division is slow on the 3DS CPU, but only the few
            // dozen pixels on the edge of the circle need it.
            int mixed =
                (((br * (255 - a) + r * a) / 255) << 16) |
                (((bg * (255 - a) + g * a) / 255) << 8) |
                ((bb * (255 - a) + b * a) / 255);
            ui3dsDrawRect(x + px, y + py, x + px + 1, y + py + 1, mixed);
        }

        if (first >= 0)
            ui3dsDrawRect(x + first, y + py, x + last + 1, y + py + 1, color);
    }

    // The letter: white, in the Ronda font, and drawn twice, one pixel apart,
    // which makes it bold. The font of the menu is put back afterwards.
    uint8 *menuFontBitmap = fontBitmap;
    uint8 *menuFontWidth = fontWidth;
    ui3dsSetFont(BADGE_FONT);

    // Each copy is centered in a box one pixel narrower than the badge, so
    // that the two together are centered.
    int left = x + BADGE_LETTER_DX;
    int top = y + BADGE_LETTER_DY;

    ui3dsDrawStringWithNoWrapping(left, top, left + BADGE_SIZE - 1, 240, 0xFFFFFF, HALIGN_CENTER, letter);
    ui3dsDrawStringWithNoWrapping(left + 1, top, left + BADGE_SIZE, 240, 0xFFFFFF, HALIGN_CENTER, letter);

    fontBitmap = menuFontBitmap;
    fontWidth = menuFontWidth;
}

//-------------------------------------------------------
// Draws the button hints at the left of the bottom bar:
// "(A) Select" and "(B) Back". The color of the background
// comes from the theme.
//-------------------------------------------------------
void menu3dsDrawFooterHints(const STheme *t)
{
    // x is where the next thing is drawn. The first badge starts
    // 10 pixels from the left edge of the screen.
    int x = 10;
    int width;

    // The (A) badge. The bottom bar goes from y = 220 to 240, and the
    // badge is placed at y = 225 so that it sits in the middle of it.
    menu3dsDrawButtonBadge(x, 225, BADGE_RED, t->FooterBack, "A");

    // The text starts 5 pixels after the badge. It is drawn at y = 223,
    // the same as the text on the right side of the bar.
    x += BADGE_SIZE + 5;
    width = ui3dsGetStringWidth("Select");
    ui3dsDrawStringWithNoWrapping(x, 223, x + width + 2, 240, t->FooterText, HALIGN_LEFT, "Select");
    
    // The next badge starts 10 pixels after the end of the text.
    x += width + 10;

    menu3dsDrawButtonBadge(x, 225, BADGE_YELLOW, t->FooterBack, "B");
    x += BADGE_SIZE + 5;
    width = ui3dsGetStringWidth("Back");
    ui3dsDrawStringWithNoWrapping(x, 223, x + width + 2, 240, t->FooterText, HALIGN_LEFT, "Back");

    x += width + 10;

    menu3dsDrawButtonBadge(x, 225, BADGE_BLUE, t->FooterBack, "X");
    x += BADGE_SIZE + 5;
    width = ui3dsGetStringWidth("Fast Scroll Page");
    ui3dsDrawStringWithNoWrapping(x, 223, x + width + 2, 240, t->FooterText, HALIGN_LEFT, "Fast Scroll Page");
}

//-------------------------------------------------------
// Draws the menu including the tabs and the bottom bar.
//-------------------------------------------------------
void menu3dsDrawMenu(int menuItemFrame, int translateY)
{
    SMenuTab *currentTab = &menuTab[currentMenuTab];

    char tempBuffer[CONSOLE_WIDTH];
    const STheme *t = theme3dsCurrent();

    // Draw the flat background
    //
    ui3dsDrawRect(0, 0, 320, 24, t->HeaderBack);
    ui3dsDrawRect(0, 24, 320, 220, t->BodyBack);
    ui3dsDrawRect(0, 220, 320, 240, t->FooterBack);

    // Draw the tabs at the top
    //
    int tabWidth = 300 / menuTabCount;
    int tabPadding = (320 - tabWidth * menuTabCount) / 2;
    for (int i = 0; i < menuTabCount; i++)
    {
        int color = i == currentMenuTab ? t->TabActiveText : t->TabInactiveText;
        ui3dsDrawStringWithNoWrapping(i * tabWidth + tabPadding, 6, (i+1)*tabWidth + tabPadding, 21, color, HALIGN_CENTER, 
            menuTab[i].Title);

        if (i == currentMenuTab)
            ui3dsDrawRect(i * tabWidth + tabPadding, 21, (i+1)*tabWidth + tabPadding, 24, t->TabUnderline);
    }

    // Shadows
    //ui3dsDrawRect(0, 23, 320, 24, 0xaaaaaa);
    //ui3dsDrawRect(0, 24, 320, 25, 0xcccccc);
    //ui3dsDrawRect(0, 25, 320, 27, 0xeeeeee);

    menu3dsDrawFooterHints(t);
    ui3dsDrawStringWithNoWrapping(10, 223, 285, 240, t->FooterText, HALIGN_RIGHT,
        impl3dsTitleText);

    //battery display
    const int maxBatteryLevel = 5;
    const int battLevelWidth = 3;
    const int battFullLevelWidth = (maxBatteryLevel) * battLevelWidth + 1;
    const int battBorderWidth = 1;
    const int battY1 = 226;
    const int battY2 = 233;
    const int battX2 = 311;
    const int battYHeight = battY2 - battY1;
    const int battHeadWidth = 2;
    const int battHeadSpacing = 1;

    // battery positive end
    ui3dsDrawRect(
        battX2 - battFullLevelWidth - battBorderWidth - battHeadWidth, 
        battY1 + battHeadSpacing, 
        battX2 - battFullLevelWidth - battBorderWidth, 
        battY2 - battHeadSpacing, 
        t->BatteryBorder, 1.0f);
    // battery body
    ui3dsDrawRect(
        battX2 - battFullLevelWidth - battBorderWidth, 
        battY1 - battBorderWidth, 
        battX2 + battBorderWidth, 
        battY2 + battBorderWidth, 
        t->BatteryBorder, 1.0f);
    // battery's empty insides
    ui3dsDrawRect(
        battX2 - battFullLevelWidth, 
        battY1, 
        battX2, 
        battY2, 
        t->FooterBack, 1.0f);
    
    ptmuInit();
    
    u8 batteryChargeState = 0;
    u8 batteryLevel = 0;
    if(R_SUCCEEDED(PTMU_GetBatteryChargeState(&batteryChargeState)) && batteryChargeState) {
        ui3dsDrawRect(
            battX2-battFullLevelWidth + 1, battY1 + 1, 
            battX2 - 1, battY2 - 1, t->BatteryCharging, 1.0f);
    } else if(R_SUCCEEDED(PTMU_GetBatteryLevel(&batteryLevel))) {
        if (batteryLevel > 5)
            batteryLevel = 5;
        for (int i = 0; i < batteryLevel; i++)
        {
            ui3dsDrawRect(
                battX2-battLevelWidth*(i+1), battY1 + 1, 
                battX2-battLevelWidth*(i) - 1, battY2 - 1, t->BatteryLevel, 1.0f);
        }
    } else {
        //ui3dsDrawRect(battX2, battY1, battX2, battY2, 0xFFFFFF, 1.0f);
    }
 
    ptmuExit();        

    int line = 0;
    int maxItems = MENU_HEIGHT;
    int menuStartY = 29;

    ui3dsSetTranslate(menuItemFrame * 3, translateY);

    if (menuItemFrame == 0)
    {
        menu3dsDrawItems(
            currentTab, 20, menuStartY, maxItems,
            t->SelectedItemBack,        // selectedItemBackColor
            t->SelectedItemText,        // selectedItemTextColor
            t->SelectedItemDescText,    // selectedItemDescriptionTextColor
            t->CheckedItemText,         // checkedItemTextColor
            t->NormalItemText,          // normalItemTextColor      
            t->NormalItemDescText,      // normalItemDescriptionTextColor      
            t->DisabledItemText,        // disabledItemTextColor
            t->HeaderItemText,          // headerItemTextColor
            t->SubtitleText);           // subtitleTextColor
    }
    else
    {
        if (menuItemFrame < 0)
            menuItemFrame = -menuItemFrame;
        float alpha = (float)(ANIMATE_TAB_STEPS - menuItemFrame + 1) / (ANIMATE_TAB_STEPS + 1);

        int white = ui3dsApplyAlphaToColor(t->BodyBack, 1.0f - alpha);   // fade towards the body background
        
         menu3dsDrawItems(
            currentTab, 20, menuStartY, maxItems,
            ui3dsApplyAlphaToColor(t->SelectedItemBack, alpha) + white,
            ui3dsApplyAlphaToColor(t->SelectedItemText, alpha) + white,       // selectedItemTextColor
            ui3dsApplyAlphaToColor(t->SelectedItemDescText, alpha) + white,       // selectedItemDescriptionTextColor
            ui3dsApplyAlphaToColor(t->CheckedItemText, alpha) + white,       // checkedItemTextColor
            ui3dsApplyAlphaToColor(t->NormalItemText, alpha) + white,       // normalItemTextColor      
            ui3dsApplyAlphaToColor(t->NormalItemDescText, alpha) + white,       // normalItemDescriptionTextColor      
            ui3dsApplyAlphaToColor(t->DisabledItemText, alpha) + white,       // disabledItemTextColor
            ui3dsApplyAlphaToColor(t->HeaderItemText, alpha) + white,       // headerItemTextColor
            ui3dsApplyAlphaToColor(t->SubtitleText, alpha) + white);      // subtitleTextColor       
        //svcSleepThread((long)(1000000.0f * 1000.0f));
    }

      
/*
    ui3dsDrawStringWithWrapping(10, 10, 100, 70, 0xff0000, HALIGN_LEFT, "This is a long text that should wrap to a few lines!");
    ui3dsDrawStringWithWrapping(10, 90, 100, 150, 0xff0000, HALIGN_RIGHT, "This is a long text that should wrap and right justify itself!");
    ui3dsDrawStringWithWrapping(10, 170, 100, 230, 0xff0000, HALIGN_CENTER, "This is a long text that should wrap and center justify itself!");
    ui3dsDrawStringWithNoWrapping(110, 10, 200, 70, 0xff0000, HALIGN_LEFT, "This is a long text will be truncated");
    ui3dsDrawStringWithNoWrapping(110, 90, 200, 150, 0xff0000, HALIGN_CENTER, "This is a long text will be truncated");
    ui3dsDrawStringWithNoWrapping(110, 170, 200, 230, 0xff0000, HALIGN_RIGHT, "This is a long text will be truncated");
*/
}



//-------------------------------------------------------
// Draws the dialog. All the colors come from the theme.
//-------------------------------------------------------
void menu3dsDrawDialog()
{
    const STheme *t = theme3dsCurrent();

    // Dialog's Background
    ui3dsDrawRect(0, 0, 320, 75, t->DialogTopBack);
    ui3dsDrawRect(0, 75, 320, 160, t->DialogBodyBack);
    ui3dsDrawRect(0, 75, 320, 78, t->DialogAccent);

    // Left trim the dialog title
    int len = strlen(dialogTab.Title);
    int startChar = 0;
    for (int i = 0; i < len; i++)
        if (dialogTab.Title[i] != ' ')
        {
            startChar = i;
            break;
        }

    // Draw the dialog's title and descriptive text
    ui3dsDrawStringWithNoWrapping(30, 10, 290, 25, t->DialogTitleText, HALIGN_LEFT, &dialogTab.Title[startChar]);
    ui3dsDrawStringWithWrapping(30, 30, 290, 70, t->DialogText, HALIGN_LEFT, dialogTab.DialogText);

    // Draw the selectable items.
    menu3dsDrawItems(
        &dialogTab, 30, 80, DIALOG_HEIGHT,
        t->DialogSelectedItemBack,      // selectedItemBackColor
        t->DialogSelectedItemText,      // selectedItemTextColor
        t->DialogTitleText,             // selectedItemDescriptionColor
        t->DialogItemText,              // checkedItemTextColor
        t->DialogItemText,              // normalItemTextColor
        t->DialogTitleText,             // normalItemDescriptionTextColor
        t->DialogTitleText,             // disabledItemTextColor
        t->DialogItemText,              // headerItemTextColor
        t->DialogItemText               // subtitleTextColor
        );
}


//-------------------------------------------------------
// Draws the menu, and the dialog depending on the
// current state of the menu system. Also takes care of
// the transition animation between menu and dialog.
//-------------------------------------------------------
void menu3dsDrawEverything(int menuFrame = 0, int menuItemsFrame = 0, int dialogFrame = 0)
{
    if (!isDialog)
    {
        int y = 0 + menuFrame * menuFrame * 120 / 32;

        ui3dsSetViewport(0, 0, 320, 240);
        ui3dsSetTranslate(0, 0);
        ui3dsDrawRect(0, 0, 400, y, 0x000000);
        ui3dsSetTranslate(0, y);
        menu3dsDrawMenu(menuItemsFrame, y);
    }
    else
    {
        int y = 80 + dialogFrame * dialogFrame * 80 / 32;

        ui3dsSetViewport(0, 0, 320, y);
        //ui3dsBlitToFrameBuffer(savedBuffer, 1.0f - (float)(8 - dialogFrame) / 10);
        ui3dsSetTranslate(0, 0);
        menu3dsDrawMenu(0, 0);
        ui3dsDrawRect(0, 0, 320, y, 0x000000, (float)(8 - dialogFrame) / 10);

        ui3dsSetViewport(0, 0, 320, 240);
        ui3dsSetTranslate(0, y);
        menu3dsDrawDialog();
        ui3dsSetTranslate(0, 0);
    }
    swapBuffer = true;
}


//-------------------------------------------------------
// Animates the selection of the tabs.
//-------------------------------------------------------
SMenuTab *menu3dsAnimateTab(int direction)
{
    SMenuTab *currentTab = &menuTab[currentMenuTab];

    if (direction < 0)
    {
        for (int i = 1; i <= ANIMATE_TAB_STEPS; i++)
        {
            app3dsMainLoop();
            menu3dsDrawEverything(0, i, 0);
            menu3dsSwapBuffersAndWaitForVBlank();
        }

        currentMenuTab--;
        if (currentMenuTab < 0)
            currentMenuTab = menuTabCount - 1;
        currentTab = &menuTab[currentMenuTab];
        
        for (int i = -ANIMATE_TAB_STEPS; i <= 0; i++)
        {
            app3dsMainLoop();
            menu3dsDrawEverything(0, i, 0);
            menu3dsSwapBuffersAndWaitForVBlank();
        }
    }
    else if (direction > 0)
    {
        for (int i = -1; i >= -ANIMATE_TAB_STEPS; i--)
        {
            app3dsMainLoop();
            menu3dsDrawEverything(0, i, 0);
            menu3dsSwapBuffersAndWaitForVBlank();
        }

        currentMenuTab++;
        if (currentMenuTab >= menuTabCount)
            currentMenuTab = 0;
        currentTab = &menuTab[currentMenuTab];
        
        for (int i = ANIMATE_TAB_STEPS; i >= 0; i--)
        {
            app3dsMainLoop();
            menu3dsDrawEverything(0, i, 0);
            menu3dsSwapBuffersAndWaitForVBlank();
        }
    }
    return currentTab;
}


static u32 lastKeysHeld = 0xffffff;
static u32 thisKeysHeld = 0;

// Frames the highlight has to stay on an item before the highlight
// callback is called, so that scrolling through a list stays fast.
#define HIGHLIGHT_CALLBACK_DELAY   10

static void (*highlightCallback)(int tabIndex, int itemID) = NULL;

void menu3dsSetHighlightCallback(void (*callback)(int tabIndex, int itemID))
{
    highlightCallback = callback;
}

void menu3dsSetFrameCallback(void (*callback)())
{
    frameCallback = callback;
}

//-------------------------------------------------------
// Displays the menu and allows the user to select from
// a list of choices.
//-------------------------------------------------------
int menu3dsMenuSelectItem(bool (*itemChangedCallback)(int ID, int value))
{
    int framesDKeyHeld = 0;
    int returnResult = -1;
    int highlightTab = -1;
    int highlightIndex = -1;
    int framesHighlighted = 0;

    char menuTextBuffer[512];

    SMenuTab *currentTab = &menuTab[currentMenuTab];

    if (isDialog)
        currentTab = &dialogTab;

    for (int i = 0; i < 2; i ++)
    {
        app3dsMainLoop();
        menu3dsDrawEverything();
        menu3dsSwapBuffersAndWaitForVBlank();

        hidScanInput();
        lastKeysHeld = hidKeysHeld();
    }

    while (app3dsMainLoop())
    {   
        if (appExiting)
        {
            returnResult = -1;
            break;
        }

        if (highlightCallback && !isDialog)
        {
            SMenuTab *tab = &menuTab[currentMenuTab];
            if (currentMenuTab != highlightTab || tab->SelectedItemIndex != highlightIndex)
            {
                highlightTab = currentMenuTab;
                highlightIndex = tab->SelectedItemIndex;
                framesHighlighted = 0;
            }
            if (++framesHighlighted == HIGHLIGHT_CALLBACK_DELAY)
            {
                int id = -1;
                if (tab->SelectedItemIndex >= 0 && tab->SelectedItemIndex < tab->ItemCount)
                    id = tab->MenuItems[tab->SelectedItemIndex].ID;
                highlightCallback(currentMenuTab, id);
            }
        }

        gpu3dsCheckSlider();
        hidScanInput();
        thisKeysHeld = hidKeysHeld();

        u32 keysDown = (~lastKeysHeld) & thisKeysHeld;
        lastKeysHeld = thisKeysHeld;

        int maxItems = MENU_HEIGHT;
        if (isDialog)
            maxItems = DIALOG_HEIGHT;

        if (currentTab->SubTitle[0])
        {
            maxItems--;
        }

        if ((thisKeysHeld & KEY_UP) || (thisKeysHeld & KEY_DOWN) || (thisKeysHeld & KEY_LEFT) || (thisKeysHeld & KEY_RIGHT))
            framesDKeyHeld ++;
        else
            framesDKeyHeld = 0;
        if (keysDown & KEY_B)
        {
            returnResult = -1;
            break;
        }
        if ((keysDown & KEY_RIGHT) || (keysDown & KEY_R) || ((thisKeysHeld & KEY_RIGHT) && (framesDKeyHeld > 15) && (framesDKeyHeld % 2 == 0)))
        {
            if (!isDialog)
            {
                if (currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_GAUGE)
                {
                    if (keysDown & KEY_RIGHT || ((thisKeysHeld & KEY_RIGHT) && (framesDKeyHeld > 15) && (framesDKeyHeld % 2 == 0)))
                    {
                        // Gauge adjustment
                        if (currentTab->MenuItems[currentTab->SelectedItemIndex].Value <
                            currentTab->MenuItems[currentTab->SelectedItemIndex].GaugeMaxValue)
                        {
                            currentTab->MenuItems[currentTab->SelectedItemIndex].Value ++ ;
                        }
                        menu3dsDrawEverything();
                    }
                }
                else
                {
                    if ((keysDown & KEY_RIGHT) || (keysDown & KEY_R))
                        currentTab = menu3dsAnimateTab(+1);
                }
            }
        }
        if ((keysDown & KEY_LEFT) || (keysDown & KEY_L)|| ((thisKeysHeld & KEY_LEFT) && (framesDKeyHeld > 15) && (framesDKeyHeld % 2 == 0)))
        {
            if (!isDialog)
            {
                if (currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_GAUGE)
                {
                    if (keysDown & KEY_LEFT || ((thisKeysHeld & KEY_LEFT) && (framesDKeyHeld > 15) && (framesDKeyHeld % 2 == 0)))
                    {
                        // Gauge adjustment
                        if (currentTab->MenuItems[currentTab->SelectedItemIndex].Value >
                            currentTab->MenuItems[currentTab->SelectedItemIndex].GaugeMinValue)
                        {
                            currentTab->MenuItems[currentTab->SelectedItemIndex].Value -- ;
                        }
                        menu3dsDrawEverything();
                    }
                }
                else
                {
                    if ((keysDown & KEY_LEFT) || (keysDown & KEY_L))
                        currentTab = menu3dsAnimateTab(-1);
                }
            }
        }
        if (keysDown & KEY_START || keysDown & KEY_A)
        {
            if (currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_ACTION)
            {
                returnResult = currentTab->MenuItems[currentTab->SelectedItemIndex].ID;
                break;
            }
            if (currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_CHECKBOX)
            {
                if (currentTab->MenuItems[currentTab->SelectedItemIndex].Value == 0)
                    currentTab->MenuItems[currentTab->SelectedItemIndex].Value = 1;
                else
                    currentTab->MenuItems[currentTab->SelectedItemIndex].Value = 0;
                
                int resultValue = currentTab->MenuItems[currentTab->SelectedItemIndex].Value;
                if (itemChangedCallback)
                {
                    bool returnFromMenu = itemChangedCallback(currentTab->MenuItems[currentTab->SelectedItemIndex].ID, resultValue);
                    if (returnFromMenu)
                    {
                        menu3dsDrawEverything();
                        menu3dsHideDialog();
                        return -1;
                    }
                }
                menu3dsDrawEverything();
            }
            if (currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_PICKER ||
                currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_PICKER2)
            {
                snprintf(menuTextBuffer, 511, "%s", currentTab->MenuItems[currentTab->SelectedItemIndex].Text);
                int resultValue = menu3dsShowDialog(menuTextBuffer, 
                    currentTab->MenuItems[currentTab->SelectedItemIndex].PickerDescription,
                    currentTab->MenuItems[currentTab->SelectedItemIndex].PickerBackColor,
                    (SMenuItem *)currentTab->MenuItems[currentTab->SelectedItemIndex].PickerItems,
                    currentTab->MenuItems[currentTab->SelectedItemIndex].Value
                    );
                if (resultValue != -1)
                {
                    currentTab->MenuItems[currentTab->SelectedItemIndex].Value = resultValue;
                    if (itemChangedCallback)
                    {
                        bool returnFromMenu = itemChangedCallback(currentTab->MenuItems[currentTab->SelectedItemIndex].ID, resultValue);
                        if (returnFromMenu)
                        {
                            menu3dsDrawEverything();
                            menu3dsHideDialog();
                            return -1;
                        }
                    }
                }
                menu3dsDrawEverything();
                menu3dsHideDialog();
            }
        }
        if (keysDown & KEY_UP || ((thisKeysHeld & KEY_UP) && (framesDKeyHeld > 15) && (framesDKeyHeld % 2 == 0)))
        {
            int moveCursorTimes = 0;

            do
            {
                if (thisKeysHeld & KEY_X)
                {
                    currentTab->SelectedItemIndex -= 15;
                    if (currentTab->SelectedItemIndex < 0)
                        currentTab->SelectedItemIndex = 0;
                }
                else
                {
                    currentTab->SelectedItemIndex--;
                    if (currentTab->SelectedItemIndex < 0)
                    {
                        currentTab->SelectedItemIndex = currentTab->ItemCount - 1;
                        currentTab->FirstItemIndex = currentTab->ItemCount - maxItems;
                        if (currentTab->FirstItemIndex < 0)
                            currentTab->FirstItemIndex = 0;
                    }
                }
                moveCursorTimes++;
            }
            while (
                (currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_DISABLED ||
                currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_HEADER1 ||
                currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_HEADER2
                ) &&
                moveCursorTimes < currentTab->ItemCount);

            if (currentTab->SelectedItemIndex < currentTab->FirstItemIndex)
                currentTab->FirstItemIndex = currentTab->SelectedItemIndex;
            if (currentTab->SelectedItemIndex >= currentTab->FirstItemIndex + maxItems)
                currentTab->FirstItemIndex = currentTab->SelectedItemIndex - maxItems + 1;

            menu3dsDrawEverything();

        }
        if (keysDown & KEY_DOWN || ((thisKeysHeld & KEY_DOWN) && (framesDKeyHeld > 15) && (framesDKeyHeld % 2 == 0)))
        {
            int moveCursorTimes = 0;
            do
            {
                if (thisKeysHeld & KEY_X)
                {
                    currentTab->SelectedItemIndex += 15;
                    if (currentTab->SelectedItemIndex >= currentTab->ItemCount)
                        currentTab->SelectedItemIndex = currentTab->ItemCount - 1;
                }
                else
                {
                    currentTab->SelectedItemIndex++;
                    if (currentTab->SelectedItemIndex >= currentTab->ItemCount)
                    {
                        currentTab->SelectedItemIndex = 0;
                        currentTab->FirstItemIndex = 0;
                    }
                }
                moveCursorTimes++;
            }
            while (
                (currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_DISABLED ||
                currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_HEADER1 ||
                currentTab->MenuItems[currentTab->SelectedItemIndex].Type == MENUITEM_HEADER2
                ) &&
                moveCursorTimes < currentTab->ItemCount);

            if (currentTab->SelectedItemIndex < currentTab->FirstItemIndex)
                currentTab->FirstItemIndex = currentTab->SelectedItemIndex;
            if (currentTab->SelectedItemIndex >= currentTab->FirstItemIndex + maxItems)
                currentTab->FirstItemIndex = currentTab->SelectedItemIndex - maxItems + 1;

            menu3dsDrawEverything();
        }

        menu3dsSwapBuffersAndWaitForVBlank();
    }

    return returnResult;
    
}


//-------------------------------------------------------
// Adds a new tab to the menu. 
//-------------------------------------------------------
void menu3dsAddTab(char *title, SMenuItem *menuItems)
{
    // Count the number items
    int itemCount = 0;
    if (menuItems)
    {
        while (true)
        {
            if (menuItems[itemCount].Type != MENUITEM_LASTITEM && itemCount < 1000)
                itemCount ++;
            else
                break;
        }
    }    

    SMenuTab *currentTab = &menuTab[menuTabCount];

    currentTab->Title = title;
    currentTab->MenuItems = menuItems;
    currentTab->ItemCount = itemCount;

    currentTab->FirstItemIndex = 0;
    currentTab->SelectedItemIndex = 0;
    for (int i = 0; i < itemCount; i++)
    {
        if (menuItems[i].ID > -1)
        {
            currentTab->SelectedItemIndex = i;
            if (currentTab->SelectedItemIndex >= currentTab->FirstItemIndex + MENU_HEIGHT)
                currentTab->FirstItemIndex = currentTab->SelectedItemIndex - MENU_HEIGHT + 1;
            break;
        }
    }

    menuTabCount++;
}


//-------------------------------------------------------
// Sets the sub-title to the tab. Use this to Display
// the current directory the user is in.
//-------------------------------------------------------
void menu3dsSetTabSubTitle(int tabIndex, char *subtitle)
{
    SMenuTab *currentTab = &menuTab[tabIndex];

    currentTab->SubTitle[0] = 0;
    if (subtitle != NULL)
        strncpy(currentTab->SubTitle, subtitle, 255);
}


//-------------------------------------------------------
// Sets the current viewing tab.
//-------------------------------------------------------
void menu3dsSetCurrentMenuTab(int tabIndex)
{
    currentMenuTab = tabIndex;
}


//-------------------------------------------------------
// Moves the cursor to the item by its ID.
//-------------------------------------------------------
void menu3dsSetSelectedItemIndexByID(int tabIndex, int ID)
{
    currentMenuTab = tabIndex;

    SMenuTab *currentTab = &menuTab[tabIndex];

    int maxItems = MENU_HEIGHT;
    if (currentTab->SubTitle[0])
        maxItems--;

    for (int i = 0; i < currentTab->ItemCount; i++)
    {
        if (currentTab->MenuItems[i].ID == ID)
        {
            currentTab->SelectedItemIndex = i;

            if (currentTab->SelectedItemIndex < currentTab->FirstItemIndex)
                currentTab->FirstItemIndex = currentTab->SelectedItemIndex;
            if (currentTab->SelectedItemIndex >= currentTab->FirstItemIndex + maxItems)
                currentTab->FirstItemIndex = currentTab->SelectedItemIndex - maxItems + 1;

            break;
        }
    }
}


//-------------------------------------------------------
// Sets the value of the menu item by its ID.
//-------------------------------------------------------
void menu3dsSetValueByID(int tabIndex, int ID, int value)
{
    if (tabIndex == -1)
    {
        for (int j = 0; j < menuTabCount; j++)
        {
            SMenuTab *currentTab = &menuTab[j];

            for (int i = 0; i < currentTab->ItemCount; i++)
            {
                if (currentTab->MenuItems[i].ID == ID)
                {
                    currentTab->MenuItems[i].Value = value;
                    break;
                }
            }
        }
    }
    else
    {
        SMenuTab *currentTab = &menuTab[tabIndex];

        for (int i = 0; i < currentTab->ItemCount; i++)
        {
            if (currentTab->MenuItems[i].ID == ID)
            {
                currentTab->MenuItems[i].Value = value;
                break;
            }
        }
    }
}


//-------------------------------------------------------
// Gets the value of the menu item by its ID.
//-------------------------------------------------------
int menu3dsGetValueByID(int tabIndex, int ID)
{
    if (tabIndex == -1)
    {
        for (int j = 0; j < menuTabCount; j++)
        {
            SMenuTab *currentTab = &menuTab[j];

            for (int i = 0; i < currentTab->ItemCount; i++)
            {
                if (currentTab->MenuItems[i].ID == ID)
                {
                    return currentTab->MenuItems[i].Value;
                }
            }
        }
    }
    else
    {
        SMenuTab *currentTab = &menuTab[tabIndex];

        for (int i = 0; i < currentTab->ItemCount; i++)
        {
            if (currentTab->MenuItems[i].ID == ID)
            {
                return currentTab->MenuItems[i].Value;
            }
        }
    }
    return -1;
}


//-------------------------------------------------------
// Gets the menu item by its ID.
//-------------------------------------------------------
SMenuItem* menu3dsGetMenuItemByID(int tabIndex, int ID)
{
    if (tabIndex == -1)
    {
        for (int j = 0; j < menuTabCount; j++)
        {
            SMenuTab *currentTab = &menuTab[j];

            for (int i = 0; i < currentTab->ItemCount; i++)
            {
                if (currentTab->MenuItems[i].ID == ID)
                {
                    return &currentTab->MenuItems[i];
                }
            }
        }

    }
    else
    {
        SMenuTab *currentTab = &menuTab[tabIndex];

        for (int i = 0; i < currentTab->ItemCount; i++)
        {
            if (currentTab->MenuItems[i].ID == ID)
            {
                return &currentTab->MenuItems[i];
            }
        }
    }
    return NULL;
}



//-------------------------------------------------------
// Clear and remove all tabs and menus. 
//-------------------------------------------------------
void menu3dsClearMenuTabs()
{
    menuTabCount = 0;
    currentMenuTab = 0;
}



//-------------------------------------------------------
// Slides the menu up and runs it until an action is 
// selected, or the user quits by pressing B.
//
// NOTE: You must call menu3dsHideMenu to transit
//       the menu away.
//-------------------------------------------------------
int menu3dsShowMenu(bool (*itemChangedCallback)(int ID, int value), bool animateMenu)
{
    isDialog = false;

    if (animateMenu)
    {
        for (int f = 8; f >= 0; f--)
        {
            app3dsMainLoop();
            menu3dsDrawEverything(f, 0, 0);    
            menu3dsSwapBuffersAndWaitForVBlank();  
        }
    }

    return menu3dsMenuSelectItem(itemChangedCallback);

}


//-------------------------------------------------------
// Hides the menu by sliding it down.
//-------------------------------------------------------
void menu3dsHideMenu()
{
    for (int f = 0; f <= 8; f++)
    {
        app3dsMainLoop();
        menu3dsDrawEverything(f, 0, 0);    
        menu3dsSwapBuffersAndWaitForVBlank();  
    }    
    ui3dsSetTranslate(0, 0);
}


//-------------------------------------------------------
// Slides the dialog up and runs it until an action is 
// selected, or the user quits by pressing B.
//
// NOTE: You must call menu3dsHideMenu to transit
//       the menu away.
//-------------------------------------------------------
int menu3dsShowDialog(char *title, char *dialogText, int newDialogBackColor, SMenuItem *menuItems, int selectedID)
{
    // Count the number items
    int itemCount = 0;
    if (menuItems)
    {
        while (true)
        {
            if (menuItems[itemCount].Type != MENUITEM_LASTITEM && itemCount < 1000)
                itemCount ++;
            else
                break;
        }
    }

    SMenuTab *currentTab = &dialogTab;

    currentTab->Title = title;
    currentTab->DialogText = dialogText;
    currentTab->MenuItems = menuItems;
    currentTab->ItemCount = itemCount;

    currentTab->FirstItemIndex = 0;
    currentTab->SelectedItemIndex = 0;

    for (int i = 0; i < itemCount; i++)
    {
        if ((selectedID == -1 && menuItems[i].ID > -1) || 
            menuItems[i].ID == selectedID)
        {
            currentTab->SelectedItemIndex = i;
            if (currentTab->SelectedItemIndex >= currentTab->FirstItemIndex + DIALOG_HEIGHT)
                currentTab->FirstItemIndex = currentTab->SelectedItemIndex - DIALOG_HEIGHT + 1;
            break;
        }
    }

    // fade the dialog fade in
    //
    app3dsMainLoop();
    menu3dsDrawEverything();
    menu3dsSwapBuffersAndWaitForVBlank();  
    //ui3dsCopyFromFrameBuffer(savedBuffer);

    isDialog = true;
    for (int f = 8; f >= 0; f--)
    {
        app3dsMainLoop();
        menu3dsDrawEverything(0, 0, f);    
        menu3dsSwapBuffersAndWaitForVBlank();  
    }

    // Execute the dialog and return result.
    //
    if (itemCount > 0)
    {
        int result = menu3dsMenuSelectItem(NULL);

        return result;
    }
    return 0;
}


//-------------------------------------------------------
// Hides the dialog by sliding it down.
//-------------------------------------------------------
void menu3dsHideDialog()
{
    // fade the dialog out
    //
    for (int f = 0; f <= 8; f++)
    {
        app3dsMainLoop();
        menu3dsDrawEverything(0, 0, f);        
        menu3dsSwapBuffersAndWaitForVBlank();    
    }

    isDialog = false;
    
    // draw the updated menu
    //
    app3dsMainLoop();
    menu3dsDrawEverything();
    menu3dsSwapBuffersAndWaitForVBlank();  
    
}


//-------------------------------------------------------
// Takes a screenshot and saves it to the specified
// path as a BMP file.
//-------------------------------------------------------
bool menu3dsTakeScreenshot(const char* path)
{
    int x, y;

    FILE *pFile = fopen(path, "wb");
    if (pFile == NULL) return false;

    // Modified this to take only the top screen
    //
    u32 bitmapsize = 400*240*2;
    u8* tempbuf = (u8*)linearAlloc(0x8A + 400*240*2);
    if (tempbuf == NULL)
    {
        fclose(pFile);
        return false;
    }
    memset(tempbuf, 0, 0x8A + bitmapsize);

    *(u16*)&tempbuf[0x0] = 0x4D42;
    *(u32*)&tempbuf[0x2] = 0x8A + bitmapsize;
    *(u32*)&tempbuf[0xA] = 0x8A;
    *(u32*)&tempbuf[0xE] = 0x28;
    *(u32*)&tempbuf[0x12] = 400;
    *(u32*)&tempbuf[0x16] = 240;
    *(u32*)&tempbuf[0x1A] = 0x1;
    *(u32*)&tempbuf[0x1C] = 0x10;
    *(u32*)&tempbuf[0x1E] = 0x3;
    *(u32*)&tempbuf[0x22] = bitmapsize;
    *(u32*)&tempbuf[0x36] = 0x0000F800;
    *(u32*)&tempbuf[0x3A] = 0x000007E0;
    *(u32*)&tempbuf[0x3E] = 0x0000001F;
    *(u32*)&tempbuf[0x42] = 0x00000000;

    u8* framebuf = (u8*)gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    for (y = 0; y < 240; y++)
    {
        for (x = 0; x < 400; x++)
        {
            int si = 1 + (((239 - y) + (x * 240)) * 4);
            int di = 0x8A + (x + ((239 - y) * 400)) * 2;

            u16 word = RGB8_to_565(framebuf[si++], framebuf[si++], framebuf[si++]);
            tempbuf[di++] = word & 0xFF;
            tempbuf[di++] = word >> 8;
        }
    }

    fwrite(tempbuf, sizeof(char), 0x8A + bitmapsize, pFile);
    fclose(pFile);

    linearFree(tempbuf);
    return true;
}


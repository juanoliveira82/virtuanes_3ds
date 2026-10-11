#include "3dstheme.h"

static const STheme themes[THEME_COUNT] =
{
    // THEME_LIGHT: the original look of the menu
    {
        "Light",

		// Flat background areas of the menu
        0x1976D2,		// Header (Strip with the tabs)
		0xFFFFFF,		// Body (Area with the items)
		0x1976D2,		// Footer (Bar at the bottom)

		// Tabs
        0xFFFFFF,		// Tab Active text
		0x90CAF9,		// Tab Inactive text
		0xFFFFFF,		// Tab Underline
        
		// Bottom bar
		0xFFFFFF,		// Footer Text
		0xFFFFFF,		// Battery Border
		0xFF9900,		// Battery Level
		0x8BC34A,     	// Battery Charging
        
		// Items
		0x333333,		// Selected Item background
		0xFFFFFF,		// Selected Item text
		0x777777,		// Selected Item description text
        0x000000,		// Checked Item text
		0x333333,		// Normal Item text
		0x777777,		// Normal Item description text
        0x888888,		// Disabled Item text
		0x1E88E5,		// Header Item text
		0x1E88E5,		// Subtitle Item text

		// Dialogs
		0xE6E6E6,		// Dialog Top background (title and description)
		0xFFFFFF,		// Dialog Body background (list of choices)
		0x1976D2,		// Dialog Accent line
		0x333333,		// Dialog Title text
		0x000000,		// Dialog Text
		0x000000,		// Dialog Item text
		0x333333,		// Dialog Selected Item background
		0xFFFFFF		// Dialog Selected Item text
    },

    // THEME_DARK: Snes9x 3DS dark theme
	{
		"Dark",
		
		// Flat background areas of the menu
		0x2E2E2E,		// Header (Strip with the tabs)
		0x1C1C1C,		// Body (Area with the items)
		0x2E2E2E,		// Footer (Bar at the bottom)
		
		// Tabs
		0xFFFFFF,		// Tab Active text
		0x999999,		// Tab Inactive text
		0x1AA1FA,		// Tab Underline
		
		// Bottom bar
		0xFFFFFF,		// Footer Text
		0xFFFFFF,		// Battery Border
		0xFF9900,		// Battery Level
		0x8BC34A,     	// Battery Charging
		
		// Items
		0x2E2E2E,		// Selected Item background
		0xFFFFFF,		// Selected Item text
		0x949494,		// Selected Item description text
		0xFFFFFF,		// Checked Item text
		0xAAAAAA,		// Normal Item text
		0x999999,		// Normal Item description text
		0x787878,		// Disabled Item text
		0x1AA1FA,		// Header Item text
		0x1AA1FA,		// Subtitle Item text

		// Dialogs
		0x222222,		// Dialog Top background (title and description)
		0x181818,		// Dialog Body background (list of choices)
		0x1AA1FA,		// Dialog Accent line
		0xA0A0A0,		// Dialog Title text
		0xFFFFFF,		// Dialog Text
		0xFFFFFF,		// Dialog Item text
		0x292929,		// Dialog Selected Item background
		0xFFFFFF		// Dialog Selected Item text
	}
};

static int currentIndex = THEME_DEFAULT;


const STheme *theme3dsCurrent()
{
    return &themes[currentIndex];
}


int theme3dsGetIndex()
{
    return currentIndex;
}


void theme3dsSet(int index)
{
    if (index < 0 || index >= THEME_COUNT)
        index = THEME_DEFAULT;
    currentIndex = index;
}

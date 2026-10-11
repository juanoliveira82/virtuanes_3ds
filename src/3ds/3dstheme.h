#ifndef _3DSTHEME_H_
#define _3DSTHEME_H_

//---------------------------------------------------------------
// Color themes for the menu on the bottom screen.
// All colors are in the 888 (RGB) format, like in 3dsui.h.
//
// The chosen theme is saved with the global settings in
// virtuanes_3ds.cfg (see impl3dsReadWriteSettingsGlobal).
//---------------------------------------------------------------

#define THEME_LIGHT     0
#define THEME_DARK      1
#define THEME_COUNT     2

// Theme used when there is no saved choice yet.
#define THEME_DEFAULT   THEME_DARK

typedef struct
{
    const char *Name;
    // New fields go at the end, because the themes in 3dstheme.cpp are initialized by position.

    // Flat background areas of the menu
    int HeaderBack;                 // Strip with the tabs
    int BodyBack;                   // Area with the items
    int FooterBack;                 // Bar at the bottom

    // Tabs
    int TabActiveText;
    int TabInactiveText;
    int TabUnderline;

    // Bottom bar
    int FooterText;
    int BatteryBorder;
    int BatteryLevel;
    int BatteryCharging;

    // Items
    int SelectedItemBack;
    int SelectedItemText;
    int SelectedItemDescText;
    int CheckedItemText;
    int NormalItemText;
    int NormalItemDescText;
    int DisabledItemText;
    int HeaderItemText;
    int SubtitleText;

    // Dialogs (pop-ups of pickers and messages).
    int DialogTopBack;              // Area with the title and description
    int DialogBodyBack;             // Area with the list of choices
    int DialogAccent;               // Line between the two areas
    int DialogTitleText;
    int DialogText;                 // Description under the title
    int DialogItemText;
    int DialogSelectedItemBack;
    int DialogSelectedItemText;
} STheme;


//---------------------------------------------------------------
// Returns the active theme.
//---------------------------------------------------------------
const STheme *theme3dsCurrent();


//---------------------------------------------------------------
// Returns the index (THEME_*) of the active theme.
//---------------------------------------------------------------
int theme3dsGetIndex();


//---------------------------------------------------------------
// Activates the theme with the given index (THEME_*).
//---------------------------------------------------------------
void theme3dsSet(int index);

#endif

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

//includes
#include <stdint.h>
#include <graphx.h>
#include "../optix/elements/window.h"
#include "../optix/elements/menu.h"
#include "gfx/output/vysion_gfx.h"

#define MAX_NUM_WINDOWS                10
#define WINDOW_MANAGER_MENU_ROW_HEIGHT 16
#define WINDOW_MANANGER_MENU_WIDTH     56

//window types
#define WINDOW_FILE_EXPLORER           0
#define WINDOW_SETTINGS                1
#define WINDOW_ABOUT                   2
//names
#define WINDOW_FILE_EXPLORER_NAME      "Files"
#define WINDOW_SETTINGS_NAME           "Settings"
#define WINDOW_ABOUT_NAME              "About"
//icons (should be 12 x 12)
#define WINDOW_FILE_EXPLORER_ICON      start_files
#define WINDOW_SETTINGS_ICON           start_settings
//how many elements are in the stack
#define WINDOW_TITLE_BAR_ELEMENTS      2
//other things
#define WINDOW_TITLE_BAR_MENU_OPTIONS  3
#define WINDOW_CLOSE_STRING            "X"
#define WINDOW_MAXIMIZE_STRING         "^"
#define WINDOW_MINIMIZE_STRING         "-"

//generic struct that will be used to start ALL windows
//this is mostly important so that their core arguments are saved, and can be restored later
//we'll start with a basic file explorer window

//Idiot epsilon: This is why the program is breaking, you need to free the whole window, not just the window title bar
//okay?
struct vysion_window_widget {
    //it's easier this way, just a cast or something
    struct optix_window_title_bar window_title_bar;
    uint8_t type;
    gfx_sprite_t *icon;
    //I think this is true?
    //all windows should have a window title bar, a window, and some things in the window title bar
    //namely, an icon, a title, and some buttons to open/close it
    //the way this will work though is that everything will be allocated out at runtime (sorry)
    //so I think it's only pointers that are needed for now, and then the optix_CopyWidget function
    //will handle everything for us
};

struct vysion_window {
    struct vysion_window_widget widget;
};

struct vysion_window_manager_menu {
    struct optix_menu menu;
    //so we know when to update?
    int last_num_windows;
    bool needs_update;
    //since this will sometimes be visible, and sometimes not
    char **text_save;
};

//a global
extern char *window_title_bar_menu_text[WINDOW_TITLE_BAR_MENU_OPTIONS + 1];
extern char *window_name[];
extern gfx_sprite_t *window_icon[];
extern gfx_sprite_t *window_title_bar_action_icon[];

//functions
//note: this function automatically calls optix_RecursiveAlign, so you don't have to
void vysion_AddWindow(struct vysion_window_widget *widget);
size_t vysion_GetWindowSize(uint8_t type);
void vysion_FormatWindowTitleBar(struct optix_window_title_bar *window_title_bar);
void vysion_UpdateWindowManagerMenu(struct optix_widget *widget);
void vysion_RenderWindowManagerMenu(struct optix_widget *widget);
void vysion_WindowManagerMenuClickAction(void *args);
void vysion_CloseWindow(struct vysion_window **window);
void vysion_CloseAllWindows(struct optix_context *context);

#endif
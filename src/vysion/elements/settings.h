#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <graphx.h>
#include "../window_manager.h"
#include "../filesystem.h"

//the offset in the stack in which the different menus start
#define SETTINGS_MENUS_STACK_OFFSET  1
#define SETTINGS_NUM_MENUS           6
//and the offset of the main menu
#define SETTINGS_SIDEBAR_MENU_OFFSET 0
//number of children total in the window
#define SETTINGS_WINDOW_NUM_CHILDREN (SETTINGS_NUM_MENUS + 1)

//seems like the logical place to put this?
struct vysion_settings_window {
    struct vysion_window window;
};

//icons for settings menu
extern gfx_sprite_t *settings_menu_spr[];
extern char *settings_menu_text[];

void vysion_AddSettingsWindow(void *config);
void vysion_SettingsMenuSideBarMenuClickAction(void *args);

#endif
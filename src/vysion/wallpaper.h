#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <debug.h>
#include <graphx.h>
#include <fileioc.h>
#include <string.h>
#include <compression.h>
#include "../optix/gui_control.h"
#include "../optix/elements/sprite.h"
#include "../optix/elements/menu.h"

#define WALLPAPER_HEADER_STRING   "VYSWALL"
#define HD_WALLAPER_HEADER_STRING "VYSWALLHD"
#define MAX_NUM_WALLPAPERS        50
#define WALLPAPER_NAME_MAX_LENGTH (8 + 1)
#define WALLPAPER_WIDTH           160
#define WALLPAPER_HEIGHT          120
#define WALLPAPER_PALETTE_OFFSET  32
//there will be 6 of these, in rows across the screen
#define HD_WALLPAPER_WIDTH        320
#define HD_WALLPAPER_HEIGHT       40
#define HD_WALLPAPER_ROWS         LCD_HEIGHT / HD_WALLPAPER_HEIGHT
#define HD_WALLPAPER_LUT_SIZE     HD_WALLPAPER_ROWS * sizeof(uint16_t) + 6
//this should maybe be somewhere else
#define GFX_VBUFFER               0xE30014

void vysion_TestWallpaper(char *name);
void vysion_SetWallpaper(char *name, struct optix_sprite *sprite[HD_WALLPAPER_ROWS]);
void vysion_InitializeWallpaper(struct optix_sprite *sprite[HD_WALLPAPER_ROWS]);
void vysion_RenderWallpaper(struct optix_widget *widget);
void vysion_WallpaperPicker(void);

struct vysion_wallpaper_picker_menu {
    struct optix_menu menu;
    //this is the wallpaper that is currently being displayed (saved for when we exit)
    int selection;
};

#endif
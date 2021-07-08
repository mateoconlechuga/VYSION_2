#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <debug.h>
#include <graphx.h>
#include <fileioc.h>
#include <string.h>
#include <compression.h>

//defines
#define WALLPAPER_HEADER_STRING   "VYSWALL"
#define HD_WALLAPER_HEADER_STRING "VYSWALLHD"
#define WALLPAPER_WIDTH           160
#define WALLPAPER_HEIGHT          120
#define WALLPAPER_PALETTE_OFFSET  32
//there will be 6 of these, in rows across the screen
#define HD_WALLPAPER_WIDTH        320
#define HD_WALLPAPER_HEIGHT       20

//functions
void vysion_TestWallpaper(char *name);

#endif
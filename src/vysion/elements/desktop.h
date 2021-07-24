#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdint.h>
#include "../wallpaper.h"
//this is a test
//more test
//is there another source control service
//quick test please work

//just 3 for now-the super buttons, the start menu, and the clock
#define DESKTOP_ELEMENTS (4 + HD_WALLPAPER_ROWS)

//some dimension things
#define TASKBAR_HEIGHT   16

void vysion_SuperButtonClickAction(void *args);
void vysion_StartMenuClickAction(void *args);
void vysion_Desktop(void);

#endif
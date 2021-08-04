#ifndef DESKTOP_H
#define DESKTOP_H

#include <stdint.h>
#include "../wallpaper.h"
//this is a test
//more test
//is there another source control service
//quick test please work

//just 3 for now-the super buttons, the start menu, and the clock
#define DESKTOP_ELEMENTS                (5 + HD_WALLPAPER_ROWS)
//this is so hacky and Mateo will loathe me for it
//but desktop elements will be signified by the element after the NULL in the children being this sequence
//so yeah
//this only applies to elements that can move within the stack, like the start menu window
#define DESKTOP_ELEMENT_MAGIC_SIGNIFIER 1

//some dimension things
#define TASKBAR_HEIGHT                  16

void vysion_SuperButtonClickAction(void *args);
void vysion_StartMenuClickAction(void *args);
void vysion_Desktop(void);

#endif
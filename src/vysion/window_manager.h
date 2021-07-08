#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

//includes
#include <stdint.h>
#include "../optix/elements/window.h"

#define MAX_NUM_WINDOWS 10

//window types
#define WINDOW_FILE_EXPLORER      0
#define WINDOW_SETTINGS           1
#define WINDOW_ABOUT              2
//names
#define WINDOW_FILE_EXPLORER_NAME "Welcome!"
//how many elements are in the stack
#define WINDOW_TITLE_BAR_ELEMENTS 2
//other things
#define WINDOW_CLOSE_STRING       "X"
#define WINDOW_MINIMIZE_STRING    "-"

//generic struct that will be used to start ALL windows
//this is mostly important so that their core arguments are saved, and can be restored later
//we'll start with a basic file explorer window

struct vysion_window_widget {
    //it's easier this way, just a cast or something
    struct optix_window_title_bar *window_title_bar;
    uint8_t type;
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

//functions
void vysion_AddWindow(struct vysion_window_widget *widget);
size_t vysion_GetWindowSize(uint8_t type);
void vysion_FormatWindowTitleBar(struct optix_window_title_bar *window_title_bar);

#endif
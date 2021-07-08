#ifndef FILE_EXPLORER_H
#define FILE_EXPLORER_H

//includes
#include <stdint.h>
#include "../window_manager.h"

//structs
struct vysion_file_explorer_window_config {
    //for the menu
    int selection;
    int min;
    //and then the index of the folder we should open
    int index;
    //sort type should go here later
};

struct vysion_file_explorer_window {
    struct vysion_window window;
};

//functions
void vysion_AddFileExplorerWindow(void *config);

//global stuff
extern unsigned char start_icon_rotated[1154];
extern unsigned char *template_text_text;

#endif
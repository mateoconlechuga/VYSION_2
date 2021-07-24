#ifndef FILE_EXPLORER_H
#define FILE_EXPLORER_H

//includes
#include <stdint.h>
#include "../window_manager.h"
#include "../filesystem.h"
#include "../../optix/elements/menu.h"

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

//seems like the logical place to put this?
struct vysion_file_explorer_menu {
    struct optix_menu menu;
    bool needs_update;
    //the index of the folder to use
    int index;
    //whether or not it can be used to explore to deeper levels of the filesystem
    bool nest;
    //the folder, for easy access
    struct vysion_folder *folder;
};

//functions
void vysion_AddFileExplorerWindow(void *config);
void vysion_UpdateFileExplorerMenu(struct optix_widget *widget);
void vysion_FileExplorerMenuClickAction(struct optix_widget *widget);

//global stuff
extern unsigned char start_icon_rotated[1154];
extern unsigned char *template_text_text;

#endif
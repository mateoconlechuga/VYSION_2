#ifndef FILE_EXPLORER_H
#define FILE_EXPLORER_H

//includes
#include <stdint.h>
#include <tice.h>
#include "../window_manager.h"
#include "../filesystem.h"
#include "../../optix/elements/menu.h"

#define FOLDER_NOT_SPECIAL 0
#define FOLDER_PROGRAMS    1
#define FOLDER_APPVARS     2

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
    //the special folder to use (e.g. programs and appvars)
    uint8_t special_folder;
    //the initial filesystem offset to use
    void *offset;
    //whether or not it can be used to explore to deeper levels of the filesystem
    bool nest;
    //the folder, for easy access
    struct vysion_folder *folder;
    //this is for doing folder actions, so we don't have to loop through the VAT
    //just 10 bytes so who cares really
    char current_selection_name[9];
    uint8_t current_selection_vysion_type;
};

//functions
void vysion_AddFileExplorerWindow(void *config);
void vysion_FileExplorerMenuClickAction(struct optix_widget *widget);
void vysion_RenderFileExplorerMenu(struct optix_widget *widget);

//global stuff
//extern unsigned char start_icon_rotated[1154];
//extern unsigned char *template_text_text;

#endif
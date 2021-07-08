#ifndef CONTROL_H
#define CONTROL_H

#include "defines.h"
#include "filesystem.h"
#include "window_manager.h"

//the master control file for the whole shell
#define VYSION_VERSION            "2.0.0"
#define VYSION_SETTINGS_VERSION   2
#define VYSION_HOOKS_VERSION      2
#define VYSION_STATE_VERSION      2

struct vysion_filesystem_info_save {
    int version;
    int num_files;
    int num_folders;
    //number of folder indexes (so deleting doesn't break things later on)
    int num_folder_indices;
};

struct vysion_context {
    struct vysion_filesystem_info_save filesystem_info_save;
    //array of files and folders
    struct vysion_file **file;
    struct vysion_folder **folder;
};

//globals (should only need one)
extern struct vysion_context *vysion_current_context;

//functions
void vysion_SetContext(struct vysion_context *context);

#endif
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

//includes
#include <stdint.h>
#include <graphx.h>
#include "control.h"
#include "window_manager.h"


//version
#define VYSION_FILESYSTEM_VERSION   2

//the most primitive types in the filesystem
#define VYSION_FILE                 0
#define VYSION_FOLDER               1

//types for files
//start with TI_BASIC and derivatives
#define VYSION_BASIC_TYPE           0
#define VYSION_PROTECTED_BASIC_TYPE 1
#define VYSION_ICE_SOURCE_TYPE      2
//asm and derivatives
#define VYSION_ASM_TYPE             3
#define VYSION_C_TYPE               4
#define VYSION_ICE_TYPE             5
//other types (just appvars for now)
//maybe could include config files or something later
#define VYSION_APPVAR_TYPE          6
//maybe have this be detected?click_action
#define VYSION_WALLPAPER_TYPE       7

//default locations
#define DEFAULT_LOCATIONS           5
#define VYSION_ROOT                 0
#define VYSION_PROGRAMS             1
#define VYSION_APPVARS              2
#define VYSION_DESKTOP              3
#define VYSION_TEST                 4
//their names
#define VYSION_ROOT_NAME            "Root"
#define VYSION_PROGRAMS_NAME        "Programs"
#define VYSION_APPVARS_NAME         "Appvars"
#define VYSION_DESKTOP_NAME         "Desktop"
#define VYSION_WALLPAPERS_NAME      "Bckgrnds"
#define VYSION_TEST_NAME            "Test"

//things for saving
#define VYSION_FILESYSTEM_APPVAR    "VYSFILES"

//some important byte sequences to know
#define ASM_HEX_SEQUENCE            ((uint16_t) 31727) //0xEF7B
#define C_HEX_SEQUENCE              ((uint8_t) 0)      //0x00
#define ICE_HEX_SEQUENCE            ((uint8_t) 127)    //0x7F
#define TI_BASIC_SEQUENCE_LEN       6
//#define TI_BASIC_SEQUENCE           68462908227370     //0x3E4443533F2A (the same as :DCS\n", used for icons)

//other things
#define MAX_ICON_SEARCH_LEN         20
#define ICON_WIDTH                  16
#define ICON_HEIGHT                 16

//structs and things
//master things
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
    struct vysion_window *window[MAX_NUM_WINDOWS];
};

//used for files and folders
struct vysion_file_widget {
    //the type, these should be the most primitive types (e.g. VYSION_FILE or VYSION_FOLDER)
    uint8_t type;
    char name[9];
    int location;
};

//file specific things
//what will be saved
struct vysion_file_save {
    struct vysion_file_widget widget;
    uint8_t ti_type;
    //whether it is pinned to the taskbar or start menu
    bool taskbar_pinned : 1;
    bool start_pinned   : 1;
};
//the struct that will be used at runtime
struct vysion_file {
    struct vysion_file_save save;
    //same as for folders-set to false to make the file not be written to the appvar on exit
    bool deleted;
    gfx_sprite_t *icon;
    //in case this is needed (which is might be)
    char icon_alternate[258];
    //the VYSION type for the variable
    uint8_t vysion_type;
    //the description
    char *description;
    uint16_t size;
    //whether it is protected, archived, etc.
    bool protected : 1;
    bool archived : 1;
};

//folder things
struct vysion_folder_save {
    struct vysion_file_widget widget;
    //don't really know what would go here
    //the folder-specific index for this folder
    //we'll probably need 2 variables, one for the number of folder indices and one for the number of folders
    int index;
    bool requires_password : 1;
    bool index_assigned : 1;
};
//the actual folder
struct vysion_folder {
    struct vysion_folder_save save;
    //if this is set to true then the file won't be saved
    bool deleted;
    gfx_sprite_t *icon;
    //this should speed things up a bit, have the folders keep track of which files and folders they have in them
    //this will be detected at the beginning
    //we need to free this
    struct vysion_file **contents;
};

//functions
//general
void vysion_DetectAllFiles(struct vysion_context *context);
void vysion_DetectAllFolders(struct vysion_context *context);
void vysion_GetFileInfo(struct vysion_file *file);
void vysion_GetFileInfo_Asm(struct vysion_file *file, void *data);
void vysion_GetFileInfo_Basic(struct vysion_file *file, void *data);
//saving/loading
void vysion_SaveFilesystem(struct vysion_context *context);
void vysion_LoadFilesystem(struct vysion_context *context);
void vysion_InitializeFilesystem(struct vysion_context *context);
//adding/deleting
struct vysion_file *vysion_AddFile(struct vysion_context *context);
struct vysion_folder *vysion_AddFolder(struct vysion_context *context);
//hash table stuff
int vysion_CreateFilesystemHashTable(struct vysion_context *context, struct vysion_file ***hash_table);
int vysion_FilesystemHashSearch(char *name, uint8_t type, struct vysion_file **hash_table, int hash_table_size);
//other things
struct vysion_folder *vysion_GetFolderByIndex(struct vysion_context *context, int index);

#endif
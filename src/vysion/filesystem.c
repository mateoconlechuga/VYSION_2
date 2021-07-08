#include "defines.h"
#include "filesystem.h"
#include "util.h"
#include "control.h"
#include <fileioc.h>
#include <graphx.h>

//NOTE: This should be called AFTER vysion_LoadFilesystem or bad things could happen
void vysion_DetectAllFiles(struct vysion_context *context) {
    //start by creating a hash table of everything
    struct vysion_file **hash_table = NULL;
    int hash_table_size = context->filesystem_info_save.num_files ? vysion_CreateFilesystemHashTable(context, &hash_table) : 0;
    void *search_pos = NULL;
    char *name = NULL;
    uint8_t type;
    //count, remove this later
    int count = 0;
    gfx_FillScreen(255);
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;
    timer_1_Counter = 0;
    while ((name = ti_DetectAny(&search_pos, NULL, &type)) != NULL) {
        int index;
        struct vysion_file *file;
        //check to make sure it's an actual program
        if (*name == '#' || *name == '!' || !(type == TI_PRGM_TYPE || type == TI_PPRGM_TYPE || type == TI_APPVAR_TYPE)) continue;
        /*gfx_SetTextXY(2, count * 10 + 2);
        gfx_PrintUInt(count, 1);
        gfx_PrintString(" ");
        gfx_PrintString(name);*/
        //if it's already been saved in the filesystem
        if (hash_table_size && ((index = vysion_FilesystemHashSearch(name, type, hash_table, hash_table_size)) != INVALID)) {
            file = hash_table[index];
            /*gfx_PrintString(" ");
            gfx_PrintUInt(index, 2);*/
            //refresh some information
            //we're assuming it's in the table already
        } else {
            //otherwise add it
            file = vysion_AddFile(context);
            //copy its data in
            file->save.ti_type = type;
            strcpy(file->save.widget.name, name);
            //the location should be initialized as well (programs, appvars)
            if (type == TI_APPVAR_TYPE) file->save.widget.location = VYSION_APPVARS;
            else file->save.widget.location = VYSION_PROGRAMS;
        }
        //get the file's information
        vysion_GetFileInfo(file);
        //increment the count
        ++count;
    }
    gfx_PrintStringXY("Done. ", 2, 2);
    gfx_PrintUInt(timer_1_Counter / 33, 4);
    while (!os_GetCSC());
}

//saving and loading things
void vysion_SaveFilesystem(struct vysion_context *context) {
    ti_var_t slot;
    int version = VYSION_FILESYSTEM_VERSION;
    if ((slot = ti_Open(VYSION_FILESYSTEM_APPVAR, "w+"))) {
        //start by writing the version, because we need to
        ti_Write(&version, sizeof(int), 1, slot);
        ti_Write(&context->filesystem_info_save, sizeof(struct vysion_filesystem_info_save), 1, slot);
        //just look through and do the files first
        //the save struct is the first entry in the struct, we already established that
        for (int i = 0; i < context->filesystem_info_save.num_files; i++) ti_Write(context->file[i], sizeof(struct vysion_file_save), 1, slot);
        //same for the folders
        for (int i = 0; i < context->filesystem_info_save.num_folders; i++) ti_Write(context->folder[i], sizeof(struct vysion_folder_save), 1, slot);
        ti_Close(slot);
    }
}

//load the filesystem
void vysion_LoadFilesystem(struct vysion_context *context) {
    ti_var_t slot;
    int version;
    if ((slot = ti_Open(VYSION_FILESYSTEM_APPVAR, "r"))) {
        int num_files;
        int num_folders;
        //read the version first
        ti_Read(&version, sizeof(int), 1, slot);
        //read the filesystem info
        ti_Read(&context->filesystem_info_save, sizeof(struct vysion_filesystem_info_save), 1, slot);
        //preserve these, which will be destroyed by the AddFile() and AddFolder() methods
        num_files = context->filesystem_info_save.num_files;
        num_folders = context->filesystem_info_save.num_folders;
        //loop through and read everything, adding files as we go
        //set them both to 0
        context->filesystem_info_save.num_files = context->filesystem_info_save.num_folders = 0;
        for (int i = 0; i < num_files; i++) {
            struct vysion_file *file = vysion_AddFile(context);
            ti_Read(file, sizeof(struct vysion_file_save), 1, slot);
        }
        //FOLDERS TOO
        for (int i = 0; i < num_folders; i++) {
            struct vysion_folder *folder = vysion_AddFolder(context);
            ti_Read(folder, sizeof(struct vysion_folder_save), 1, slot);
        }
        ti_Close(slot);
    }
}

//get the file info (size, archived/unarchived, locked, icon, etc.)
void vysion_GetFileInfo(struct vysion_file *file) {
    ti_var_t slot;
    if ((slot = ti_OpenVar(file->save.widget.name, "r", file->save.ti_type))) {
        //generic things
        file->icon = NULL;
        file->size = ti_GetSize(slot);
        file->archived = ti_IsArchived(slot);
        //now get more information
        if (file->save.ti_type == TI_PPRGM_TYPE) vysion_GetFileInfo_Asm(file, ti_GetDataPtr(slot));
        else if (file->save.ti_type == TI_PRGM_TYPE) vysion_GetFileInfo_Basic(file, ti_GetDataPtr(slot));
        //close that slot
        ti_Close(slot);
    }
}

//takes a pointer to a file and a pointer to the data for the file (probably slightly faster than opening and closing the slot another time)
void vysion_GetFileInfo_Asm(struct vysion_file *file, void *data) {
    //firstly, we want to check if this is really a protected TI-Basic file
    //if it's ASM, that will be indicated by the first 2 bytes being 0xef7b
    if (*((uint16_t *) data) != ASM_HEX_SEQUENCE) {
        vysion_GetFileInfo_Basic(file, data);
        return;
    }
    //the data ptr + 2 will give us what type it is (specifically, C or ICE if that is true)
    switch (*((uint8_t *) data + 2)) {
        case C_HEX_SEQUENCE:
            file->vysion_type = VYSION_C_TYPE;
            break;
        case ICE_HEX_SEQUENCE:
            file->vysion_type = VYSION_ICE_TYPE;
            break;
        default:
            file->vysion_type = VYSION_ASM_TYPE;
            break;
    }
    //see if we have an icon
    //this will be indicated by the width x height bytes, which are 16 x 16
    //will also make it null if none is found, so thanks C
    file->icon = memchr(data, 0x10, MAX_ICON_SEARCH_LEN);
}

void vysion_GetFileInfo_Basic(struct vysion_file *file, void *data) {
    uint8_t palette[] = {223, 24, 224, 0, 248, 6, 228, 96, 16, 29, 231, 255, 222, 189, 148, 74};
    uint8_t ti_basic_sequence[] = {0x3E, 0x44, 0x43, 0x53, 0x3F, 0x2A};
    char temp[] = " ";
    //start with an easy check to see if it is protected or not
    if (file->save.ti_type == TI_PRGM_TYPE) file->vysion_type = VYSION_BASIC_TYPE;
    else file->vysion_type = VYSION_PROTECTED_BASIC_TYPE;
    if (memcmp(ti_basic_sequence, data, TI_BASIC_SEQUENCE_LEN) == 0) {
        //get the icon
        file->icon = gfx_MallocSprite(ICON_WIDTH, ICON_HEIGHT);
        for (int i = 0; i < ICON_WIDTH * ICON_HEIGHT; i++) {
            temp[0] = *((uint8_t *) data + i + TI_BASIC_SEQUENCE_LEN);
            file->icon->data[i] = palette[strtol(temp, NULL, 16)];
        }
    }
}

//adding files
//adds a new file, allocates it
//probably better than the old system, but who knows (less fragmentation)
//returns a pointer to the new file
struct vysion_file *vysion_AddFile(struct vysion_context *context) {
    int new_num_files = ++context->filesystem_info_save.num_files;
    context->file = realloc(context->file, new_num_files * sizeof(struct vysion_file *));
    //this will need to be freed later
    return context->file[new_num_files - 1] = malloc(sizeof(struct vysion_file));
}

struct vysion_folder *vysion_AddFolder(struct vysion_context *context) {
    int new_num_folders = ++context->filesystem_info_save.num_folders;
    context->folder = realloc(context->folder, new_num_folders * sizeof(struct vysion_folder *));
    context->folder[new_num_folders]->save.index = context->filesystem_info_save.num_folder_indices++;
    return context->folder[new_num_folders - 1] = malloc(sizeof(struct vysion_folder));
}

//useful things
//this will need a free afterward, on the top level
//returns the size of the created table, in entries
int vysion_CreateFilesystemHashTable(struct vysion_context *context, struct vysion_file ***hash_table) {
    //apparently hash tables work best when they are at least twice the size of the data set
    int hash_table_size = context->filesystem_info_save.num_files * 2;
    if (!hash_table_size) return 0;
    *hash_table = malloc(hash_table_size * sizeof(struct vysion_file *));
    //gfx_FillScreen(255);
    //set all the values to null
    for (int i = 0; i < hash_table_size; i++) (*hash_table)[i] = NULL;
    for (int i = 0; i < context->filesystem_info_save.num_files; i++) {
        int target_index = vysion_util_GetStringHash(context->file[i]->save.widget.name) % hash_table_size;
        //gfx_SetTextXY(2 + (i / 20) * 30, (i % 20) * 10 + 2);
        //gfx_PrintUInt(target_index, 3);
        //iterate through until we find a valid index (NULL) or reach the end of the table
        while ((*hash_table)[target_index] && target_index <= hash_table_size) {
            target_index++;
            //wrap around the table
            target_index %= hash_table_size;
        }
        //put it in that table (probably need some error checking here eventually)
        (*hash_table)[target_index] = context->file[i];
    }
    /*for (int i = 0; i < hash_table_size; i++) {
        gfx_SetTextXY(30 + (i % 20) * 30 , i * 10 + 2);
        gfx_PrintUInt(i, 3);
        gfx_PrintString(" ");
        if ((*hash_table)[i]) gfx_PrintString((*hash_table)[i]->save.widget.name);
    }*/
    //while (!os_GetCSC());
    return hash_table_size;
}

//returns the index of the searched-for element or -1 on failure
int vysion_FilesystemHashSearch(char *name, uint8_t type, struct vysion_file **hash_table, int hash_table_size) {
    bool found_file = false;
    int target_index;
    if (!hash_table_size) return INVALID;
    target_index = vysion_util_GetStringHash(name) % hash_table_size;
    for (int i = 0; i < hash_table_size; i++) {
        if (strcmp(hash_table[target_index]->save.widget.name, name) == 0 && hash_table[target_index]->save.ti_type == type) {
            found_file = true;
            break;
        }
        target_index++;
        target_index %= hash_table_size;
    }
    return found_file ? target_index : INVALID;
}
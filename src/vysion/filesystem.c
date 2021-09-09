#include "filesystem.h"
#include "util.h"
#include "control.h"
#include "gfx/output/vysion_gfx.h"
#include <fileioc.h>
#include <graphx.h>
#include <debug.h>
#include "defines.h"

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
    timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;
    timer_1_Counter = 0;
    while ((name = ti_DetectAny(&search_pos, NULL, &type)) != NULL) {
        int index;
        struct vysion_file *file;
        //check to make sure it's an actual program
        if (*name == '#' || *name == '!' || !(type == TI_PRGM_TYPE || type == TI_PPRGM_TYPE || type == TI_APPVAR_TYPE)) continue;
        //if it's already been saved in the filesystem
        dbg_sprintf(dbgout, "Name: %s New: ", name);
        if (hash_table_size && ((index = vysion_FilesystemHashSearch(name, type, hash_table, hash_table_size)) != INVALID)) {
            dbg_sprintf(dbgout, "0 ");
            file = hash_table[index];
            /*gfx_PrintString(" ");
            gfx_PrintUInt(index, 2);*/
            //refresh some information
            //we're assuming it's in the table already
        } else {
            dbg_sprintf(dbgout, "1 ");
            //otherwise add it
            file = vysion_AddFile(context);
            //copy its data in
            file->save.ti_type = type;
            file->save.widget.type = VYSION_FILE;
            strcpy(file->save.widget.name, name);
            //the location should be initialized as well (programs, appvars)
            if (type == TI_APPVAR_TYPE) file->save.widget.location = VYSION_APPVARS;
            else file->save.widget.location = VYSION_PROGRAMS;
        }
        dbg_sprintf(dbgout, "Location: %d\n", file->save.widget.location);
        //get the file's information
        timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_UP;
        timer_1_Counter = 0;
        file->deleted = false;
        vysion_GetFileInfo(file);
        dbg_sprintf(dbgout, "File info took %d ms.\n", timer_1_Counter);
        //increment the count
        ++count;
    }
}

//so this HAS to be done after the vysion_DetectAllFiles function is called
//bad things will probably happen otherwise
void vysion_DetectAllFolders(struct vysion_context *context) {
    struct vysion_file_widget *temp[context->filesystem_info_save.num_files + context->filesystem_info_save.num_folders];
    for (int i = 0; i < context->filesystem_info_save.num_folders; i++) {
        //start by getting how many files there are, and then allocate accordingly
        int count = 0;
        for (int j = 0; j < context->filesystem_info_save.num_files; j++) {
            if (context->file[j]->save.widget.location == context->folder[i]->save.index && !context->file[j]->deleted) {
                temp[count] = context->file[j];
                count++;
            }
        }
        for (int j = 0; j < context->filesystem_info_save.num_folders; j++) {
            //basically folder can't be inside itself
            if (context->folder[j]->save.widget.location == context->folder[i]->save.index && context->folder[j]->save.widget.location != context->folder[j]->save.index) {
                temp[count] = context->folder[j];
                count++;
            }
        }
        dbg_sprintf(dbgout, "Count was %d\n", count);
        //now copy everything into the folder1
        //we want a NULL at the end
        context->folder[i]->contents = malloc((count + 1) * sizeof(struct vysion_file_widget *));
        context->folder[i]->contents[count] = NULL;
        memcpy(context->folder[i]->contents, temp, count * sizeof(struct vysion_file_widget *));
        //handle the folder icon too
        context->folder[i]->icon = icon_folder;
    }
    dbg_sprintf(dbgout, "Success.\n");
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
        for (int i = 0; i < context->filesystem_info_save.num_files; i++)
            if (!context->file[i]->deleted) ti_Write(context->file[i], sizeof(struct vysion_file_save), 1, slot);
        //same for the folders
        for (int i = 0; i < context->filesystem_info_save.num_folders; i++)
            ti_Write(context->folder[i], sizeof(struct vysion_folder_save), 1, slot);
        ti_Close(slot);
    }
}

//load the filesystem
void vysion_LoadFilesystem(struct vysion_context *context) {
    ti_var_t slot;
    int version;
    //sort the VAT before we do anything else
    vysion_asm_SortVAT();
    if ((slot = ti_Open(VYSION_FILESYSTEM_APPVAR, "r"))) {
        int num_files;
        int num_folders;
        int num_folder_indices;
        //read the version first
        ti_Read(&version, sizeof(int), 1, slot);
        //read the filesystem info
        ti_Read(&context->filesystem_info_save, sizeof(struct vysion_filesystem_info_save), 1, slot);
        //preserve these, which will be destroyed by the AddFile() and AddFolder() methods
        num_files = context->filesystem_info_save.num_files;
        num_folders = context->filesystem_info_save.num_folders;
        num_folder_indices = context->filesystem_info_save.num_folder_indices;
        //loop through and read everything, adding files as we go
        //set them both to 0
        context->filesystem_info_save.num_files = context->filesystem_info_save.num_folders = 0;
        for (int i = 0; i < num_files; i++) {
            struct vysion_file *file = vysion_AddFile(context);
            ti_Read(file, sizeof(struct vysion_file_save), 1, slot);
            //let's do this here
            file->deleted = true;
        }
        //FOLDERS TOO
        for (int i = 0; i < num_folders; i++) {
            struct vysion_folder *folder = vysion_AddFolder(context);
            ti_Read(folder, sizeof(struct vysion_folder_save), 1, slot);
            dbg_sprintf(dbgout, "Loaded index: %d\n", folder->save.index);
            folder->deleted = true;
        }
        context->filesystem_info_save.num_folder_indices = num_folder_indices;
        ti_Close(slot);
    } else vysion_InitializeFilesystem(context);
}

void vysion_InitializeFilesystem(struct vysion_context *context) {
    char *name[DEFAULT_LOCATIONS] = {
        //root
        VYSION_ROOT_NAME,
        //programs
        VYSION_PROGRAMS_NAME,
        //appvars
        VYSION_APPVARS_NAME,
        //desktop
        VYSION_DESKTOP_NAME,
        //test
        VYSION_TEST_NAME,
    };
    //then do this
    for (int i = 0; i < DEFAULT_LOCATIONS; i++) {
        struct vysion_folder *folder;
        folder = vysion_AddFolder(context);
        strcpy(folder->save.widget.name, name[i]);
        folder->save.index = i;
        folder->save.widget.location = (i == 4) ? VYSION_DESKTOP : VYSION_ROOT;
        folder->save.widget.type = VYSION_FOLDER;
        folder->icon = icon_folder;
    }
}


//get the file info (size, archived/unarchived, locked, icon, etc.)
void vysion_GetFileInfo(struct vysion_file *file) {
    ti_var_t slot;
    //default file icons
    gfx_sprite_t *default_icon[] = {
        //basic
        icon_basic,
        //protected basic
        icon_basic,
        //ICE source
        icon_ices,
        //assembly
        icon_ez80,
        //C
        icon_c,
        //ICE
        icon_ice,
        //appvar
        icon_appvar,
    };
    ti_CloseAll();
    if ((slot = ti_OpenVar(file->save.widget.name, "r", file->save.ti_type))) {
        //generic things
        file->icon = NULL;
        file->custom_icon = false;
        file->size = ti_GetSize(slot);
        file->archived = ti_IsArchived(slot);
        //now get more information
        if (file->save.ti_type == TI_PPRGM_TYPE) vysion_GetFileInfo_Asm(file, ti_GetDataPtr(slot));
        else if (file->save.ti_type == TI_PRGM_TYPE) vysion_GetFileInfo_Basic(file, ti_GetDataPtr(slot));
        else if (file->save.ti_type == TI_APPVAR_TYPE) file->vysion_type = VYSION_APPVAR_TYPE;
        //if the file icon hasn't been set then assign it one of the default ones
        if (!file->custom_icon) {
            file->icon = default_icon[file->vysion_type];
        } else {
            //convert it
            if (file->vysion_type != VYSION_BASIC_TYPE && file->vysion_type != VYSION_PROTECTED_BASIC_TYPE)
                vysion_ConvertXlibcToPalette(file->icon);
        }
        //close that slot
        ti_Close(slot);
    }
}

//takes a pointer to a file and a pointer to the data for the file (probably slightly faster than opening and closing the slot another time)
void vysion_GetFileInfo_Asm(struct vysion_file *file, void *data) {
    gfx_sprite_t *icon_ptr;
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
    icon_ptr = memchr(data, 0x10, MAX_ICON_SEARCH_LEN);
    if (icon_ptr) {
        file->icon = gfx_AllocSprite(0x10, 0x10, malloc);
        file->custom_icon = true;
        memcpy(file->icon, icon_ptr, 258);
    }
}

void vysion_GetFileInfo_Basic(struct vysion_file *file, void *data) {
    uint8_t palette[] = {223, 24, 224, 0, 248, 6, 228, 96, 16, 29, 231, 255, 222, 189, 148, 74};
    uint8_t ti_basic_sequence[] = {0x3E, 0x44, 0x43, 0x53, 0x3F, 0x2A};
    char temp[] = " ";
    for (int i = 0; i < 16; i++) palette[i] = xlibc_condensed->data[palette[i]];
    //start with an easy check to see if it is protected or not
    if (file->save.ti_type == TI_PRGM_TYPE) file->vysion_type = VYSION_BASIC_TYPE;
    else file->vysion_type = VYSION_PROTECTED_BASIC_TYPE;
    if (memcmp(ti_basic_sequence, data, TI_BASIC_SEQUENCE_LEN) == 0) {
        //get the icon
        file->icon = gfx_AllocSprite(0x10, 0x10, malloc);
        file->custom_icon = true;
        //file->icon = gfx_MallocSprite(ICON_WIDTH, ICON_HEIGHT);
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
    context->folder[new_num_folders - 1] = malloc(sizeof(struct vysion_folder));
    context->folder[new_num_folders - 1]->save.index = context->filesystem_info_save.num_folder_indices++;
    dbg_sprintf(dbgout, "Index: %d Indices: %d Folders: %d\n", context->folder[new_num_folders - 1]->save.index, context->filesystem_info_save.num_folder_indices, context->filesystem_info_save.num_folders);
    return context->folder[new_num_folders - 1];
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

struct vysion_folder *vysion_GetFolderByIndex(struct vysion_context *context, int index) {
    int i = 0;
    struct vysion_folder *ptr = NULL;
    for (i = 0; i < context->filesystem_info_save.num_folders; i++) {
        if (context->folder[i]->save.index == index) {
            ptr = context->folder[i];
            break;
        }
    }
    return ptr;
}
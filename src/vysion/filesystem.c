#include "filesystem.h"

#include <stdint.h>
#include <string.h>

#include <fileioc.h>
#include <tice.h>
#include <debug.h>

#include "util.h"
#include "gfx/output/vysion_gfx.h"

//returns the nth entry of the current directory
//as per usual ptr should be to a folder but it shouldn't break if you want to do something weird
struct vysion_widget *vysion_GetDirectoryEntryByIndex(struct vysion_widget *ptr, int index) {
    for (int i = 0; i < index; i++) ptr = vysion_GetNextFile(ptr);
    return ptr;
}



//returns the number of files in the directory, or more accurately, the number of files until the folder ends
//so if you call it when ptr is a pointer to a folder, it will give the number of files in that folder
//not a good idea to call when ptr is not folder, that'll probably break something
struct vysion_widget *vysion_GetNumFilesInDirectory(struct vysion_widget *ptr) {
    int i = 0;
    ptr += 1;
    while (ptr->type != VYSION_FOLDER_END) {
        ptr = vysion_GetNextFile(ptr);
        i++;
    }
    dbg_sprintf(dbgout, "Num files: %d Ptr type: %d\n", i, ptr->type);
    return i;
}

//returns the next file or folder, starting the search at ptr
//it will skip over the contents of a folder if ptr->type == VYSION_FOLDER_TYPE
//intended to be used in a loop
struct vysion_widget *vysion_GetNextFile(struct vysion_widget *ptr) {
	if (ptr->type == VYSION_FOLDER_TYPE) {
		struct vysion_widget *temp_ptr = ptr + 1;
        dbg_sprintf(dbgout, "Test 3 %d\n", (int) *((uint8_t *) temp_ptr));
        dbg_sprintf(dbgout, "Offset: %d\n", temp_ptr - ptr);
        dbg_sprintf(dbgout, "Temp ptr type: %d Offset: %d Sizeof widget: %d\n", temp_ptr->type, temp_ptr - (struct vysion_widget *) vysion_current_context->filesystem_start, sizeof(struct vysion_widget));
		while (temp_ptr->type != VYSION_FOLDER_END) {
            //dbg_sprintf(dbgout, "This ran at least once.\n");
			temp_ptr = vysion_GetNextFile(temp_ptr);
        }
        dbg_sprintf(dbgout, "Success. %d\n", ((struct vysion_widget *) ((char *) temp_ptr + 1))->type);
		//we need to add 1 here because VYSION_FOLDER_END is 1 byte
		return (char *) temp_ptr + 1;
	} else if (ptr->type == VYSION_FOLDER_END) {
        dbg_sprintf(dbgout, "This was true, apparently.\n");
        return (char *) ptr + 1;
    } else return ptr + 1;
}


//returns the previous file or folder, starting the search at ptr
//it will skip over the contents of folders if VYSION_FOLDER_END is encountered
//intended to be used in a loop; effectively the opposite of vysion_GetNextFile
struct vysion_widget *vysion_GetPreviousFile(struct vysion_widget *ptr) {
	if ((ptr - sizeof(uint8_t))->type == VYSION_FOLDER_END) {
		struct vysion_widget *temp_ptr = ptr - sizeof(uint8_t);
		while (temp_ptr->type != VYSION_FOLDER_TYPE)
			temp_ptr = vysion_GetPreviousFile(temp_ptr);
		return temp_ptr;
	} else return ptr - sizeof(struct vysion_widget);
}

//returns the source directory of the current directory (e.g. trying on /Desktop would return /)
struct vysion_widget *vysion_GetSourceDirectory(struct vysion_widget *ptr) {
	//the logic that I have for this is fairly simple-we'll know that it is the source directory if
	//vysion_GetPreviousFile encounters a VYSION_FOLDER without encountering a VYSION_FOLDER_END first
	//we can see if this is the case if vysion_GetPreviousFile returns a folder AND the value returns equals
	//ptr - sizeof(vysion_widget)
	struct vysion_widget *previous_ptr;
	struct vysion_widget *temp_ptr; 
	while (previous_ptr - sizeof(struct vysion_widget) != temp_ptr) {
		previous_ptr = temp_ptr;
		temp_ptr = vysion_GetPreviousFile(temp_ptr);	
	}
	return temp_ptr;
}

struct vysion_file_info *vysion_GetFileInfo(struct vysion_file_info *file_info, char *name, uint8_t ti_type) {
    ti_var_t slot;
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
    if (slot = ti_OpenVar(name, "r", ti_type)) {
        void *data = ti_GetDataPtr(slot);
        gfx_sprite_t *icon_buffer = (gfx_sprite_t *) file_info->icon_buffer;
        file_info->size = ti_GetSize(slot);
        file_info->archived = ti_IsArchived(slot);
        file_info->icon = NULL;
        //this could potentially cause issues if it's left as 0, so account for that
        icon_buffer->width = ICON_WIDTH;
        icon_buffer->height = ICON_HEIGHT;
        switch (ti_type) {
            case TI_PRGM_TYPE:
                vysion_GetFileInfo_Basic(data, file_info);
                break;
            case TI_PPRGM_TYPE:
                vysion_GetFileInfo_Asm(data, file_info);
                break;
            case TI_APPVAR_TYPE:
                file_info->vysion_type = VYSION_APPVAR_TYPE;
                break;
            default:
                break;
        }
        if (!file_info->icon) file_info->icon = default_icon[file_info->vysion_type];
        ti_Close(slot);
        return file_info;
    } else return NULL;
}

void vysion_GetFileInfo_Basic(void *data, struct vysion_file_info *file_info) {
    uint8_t palette[] = {223, 24, 224, 0, 248, 6, 228, 96, 16, 29, 231, 255, 222, 189, 148, 74};
    uint8_t ti_basic_sequence[] = {0x3E, 0x44, 0x43, 0x53, 0x3F, 0x2A};
    int i;
    file_info->vysion_type = VYSION_BASIC_TYPE;
    //this way we don't have to run the palette conversion function every time (everything is known)
    if (!memcmp(data, ti_basic_sequence, TI_BASIC_SEQUENCE_LEN)) {
        for (i = 0; i < 16; i++) palette[i] = xlibc_condensed->data[palette[i]];
        for (i = 0; i < ICON_WIDTH * ICON_HEIGHT; i++) {
            uint8_t *ptr = (uint8_t *) (data + i);
            //credit to Mateo for this formula
            ((gfx_sprite_t *)file_info->icon_buffer)->data[i] = vysion_TIOSToXlibc(ptr);

        }
    }
}

void vysion_GetFileInfo_Asm(void *data, struct vysion_file_info *file_info) {
    //firstly, we want to check if this is really a protected TI-Basic file
    //if it's ASM, that will be indicated by the first 2 bytes being 0xef7b
    if (*((uint16_t *) data) != ASM_PROGRAM_TOKEN) {
        file_info->vysion_type = VYSION_PROTECTED_BASIC_TYPE;
        vysion_GetFileInfo_Basic(data, file_info);
        return;
    }
    //the data ptr + 2 will give us what type it is (specifically, C or ICE if that is true)
    switch (*((uint8_t *) data + 2)) {
        case C_HEX_SEQUENCE:
            file_info->vysion_type = VYSION_C_TYPE;
            break;
        case ICE_HEX_SEQUENCE:
            file_info->vysion_type = VYSION_ICE_TYPE;
            break;
        default:
            file_info->vysion_type = VYSION_ASM_TYPE;
            break;
    }
    //see if we have an icon
    //this will be indicated by the width x height bytes, which are 16 x 16
    //will also make it null if none is found, so thanks C
    file_info->icon = memchr(data, 0x10, MAX_ICON_SEARCH_LEN);
    dbg_sprintf(dbgout, "File icon: %d\n", file_info->icon);
    if (file_info->icon) {
        memcpy(file_info->icon_buffer, file_info->icon, ICON_SIZE);
        vysion_ConvertXlibcToPalette(file_info->icon_buffer);
        file_info->icon = file_info->icon_buffer;
    } else {
        //assign a default icon here

    }
}

uint8_t vysion_VysionTypeToTIType(uint8_t vysion_type) {
    switch (vysion_type) {
        case VYSION_ASM_TYPE:
        case VYSION_C_TYPE:
        case VYSION_ICE_TYPE:
        case VYSION_PROTECTED_BASIC_TYPE:
            return TI_PPRGM_TYPE;
            break;
        case VYSION_BASIC_TYPE:
            return TI_PRGM_TYPE;
            break;
        case VYSION_APPVAR_TYPE:
            return TI_APPVAR_TYPE;
            break;
    }
}


//initialize the filesystem, getting the number of programs and appvars so we know how big the menus are later
void vysion_InitializeFilesystem(struct vysion_context *context) {
    uint8_t type;
    void *search_pos = NULL;
    char *var_name;
    vysion_asm_SortVAT();
    context->num_appvars = 0;
    context->num_programs = 0;
    while ((var_name = ti_DetectAny(&search_pos, NULL, &type))) {
        if (*var_name == '!' || *var_name == '#') continue;
        switch (type) {
            case TI_PRGM_TYPE:
            case TI_PPRGM_TYPE:
                context->num_programs++;
                break;
            case TI_APPVAR_TYPE:
                context->num_appvars++;
                break;
            default:
                break;
        }
    }
}
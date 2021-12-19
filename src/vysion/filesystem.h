#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <graphx.h>
#include <stdint.h>

#include "control.h"

//version
#define VYSION_FILESYSTEM_VERSION   (uint8_t) 3

//types for files
//start with TI_BASIC and derivatives
#define VYSION_BASIC_TYPE           (uint8_t) 0
#define VYSION_PROTECTED_BASIC_TYPE (uint8_t) 1
#define VYSION_ICE_SOURCE_TYPE      (uint8_t) 2
//asm and derivatives
#define VYSION_ASM_TYPE             (uint8_t) 3
#define VYSION_C_TYPE               (uint8_t) 4
#define VYSION_ICE_TYPE             (uint8_t) 5
//other types (just appvars for now)
//maybe could include config files or something later
#define VYSION_APPVAR_TYPE          (uint8_t) 6
//maybe have this be detected?
#define VYSION_WALLPAPER_TYPE       (uint8_t) 7
//this will ensure we have room for more types later if required
#define VYSION_FOLDER_TYPE          (uint8_t) 254
#define VYSION_FOLDER_END           (uint8_t) 255

//OS defines
#define ASM_PROGRAM_TOKEN           ((uint16_t) 31727) //0xEF7B
#define MAX_ICON_SEARCH_LEN         20
#define TI_BASIC_SEQUENCE_LEN       6
#define ICON_WIDTH                  16
#define ICON_HEIGHT                 16
#define ICON_SIZE                   (uint16_t) ICON_WIDTH * ICON_HEIGHT + 2

//program types
#define C_HEX_SEQUENCE              (uint8_t) 0x00
#define ICE_HEX_SEQUENCE            (uint8_t) 0x7f

//default filesystem state:
/*
	Programs
	Appvars
	Root
		Desktop
		Taskbar*/
//as Programs and Appvars are "special" directories (outside of the main directory system, and access the VAT
//directly, they will be placed outside of the filesystem, which is known to start at Root (or index 2)
//so, Programs is known to be at index 0 (base ptr) and Appvars at index 1 (base ptr + sizeof(vysion_widget))
//if one of these is attempted to be accessed, we'll use the special flag in the filesystem menu to display it 


#define vysion_TIOSToXlibc(color)   (uint8_t) ((color[0] - '0') << 8) | (color[1] - '0')          

struct vysion_file_info {
    //VYSION types, as defined above
    uint8_t vysion_type;
    gfx_sprite_t *icon;
    char icon_buffer[ICON_SIZE];
    uint16_t size;
    //bools
    bool archived : 1;
    bool protected : 1;
};


//if this is a folder, the filesystem will assume that it continues, including all of
//the files and folders that follow until FOLDER_END is encountered
//this will allow for multi-leveled folders as well
struct vysion_widget {
    //file or folder
    uint8_t type;
    char name[9];
};


struct vysion_widget *vysion_GetNextFile(struct vysion_widget *ptr);
struct vysion_widget *vysion_GetPreviousFile(struct vysion_widget *ptr);
struct vysion_widget *vysion_GetSourceDirectory(struct vysion_widget *ptr);
struct vysion_file_info *vysion_GetFileInfo(struct vysion_file_info *file_info, char *name, uint8_t ti_type);
void vysion_GetFileInfo_Basic(void *data, struct vysion_file_info *file_info);
void vysion_GetFileInfo_Asm(void *data, struct vysion_file_info *file_info);
uint8_t vysion_VysionTypeToTIType(uint8_t vysion_type);
void vysion_InitializeFilesystem(struct vysion_context *context);

#endif
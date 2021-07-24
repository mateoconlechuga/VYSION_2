/*[NAME]    VYSION CE
  [VERSION] 2.0.0
  [AUTHOR]  epsilon5
  [DATE]    2021-04-05
*/

#include "main.h"
#include "vysion/defines.h"
#include "vysion/filesystem.h"
#include "vysion/control.h"
#include "vysion/wallpaper.h"
#include "vysion/init.h"
#include <fileioc.h>
#include <graphx.h>
#include <debug.h>
#include "vysion/gfx/output/gfx.h"
#include "vysion/elements/file_explorer.h"
#include "vysion/elements/desktop.h"


void vysion_asm_ConfigureRunProgram(char *test, bool is_asm, bool use_reloader);
//  void *vRam = (void *) 0xD40000;
//  memcpy(vRam, test, 10);
//  *((uint8_t *) vRam + 10) = (uint8_t) is_asm;
//  *((uint8_t *) vRam + 11) = (uint8_t) use_reloader;
//};


void vysion_asm_RunProgram(void);

void vysion_SetOP1(uint8_t type, char name[]) {
    //does this work?
    char temp[10] = " ";
    temp[0] = type;
    strcat(temp, name);
    strcpy(os_OP1, temp);
}


void main(void) {
    //initialize the context and OPTIX environment
    struct vysion_context vysion_context = {
        .filesystem_info_save = {.num_files = 0, .num_folders = 0, .num_folder_indices = 0},
    };
    char *name[] = {"File", "Folder"};
    //OPTIX stuff
    struct optix_cursor cursor;
    struct optix_colors colors;
    struct optix_settings settings;
    struct optix_gui_data data;
    //the final context (we'll add the stack last)
    struct optix_context context = {
        .settings = &settings,
        .cursor = &cursor,
        .data = &data,
        .colors = &colors,
    };
    vysion_SetContext(&vysion_context);
    //do filesystem things
    vysion_LoadFilesystem(&vysion_context);
    vysion_DetectAllFiles(&vysion_context);
    vysion_DetectAllFolders(&vysion_context);
    for (int i = 0; i < vysion_context.filesystem_info_save.num_folders; i++) {
        int j = 0;
        dbg_sprintf(dbgout, "Name: %s Index: %d\n", vysion_context.folder[i]->save.widget.name, vysion_context.folder[i]->save.index);
        while (vysion_context.folder[i]->contents[j]) {
            struct vysion_file_widget *widget = vysion_context.folder[i]->contents[j];
            dbg_sprintf(dbgout, "   Name: %s Type: %s\n", widget->name, name[widget->type]);
            j++;
        }
    }
    vysion_InitializeGraphics();
    vysion_InitializeOPTIX(&context);
    vysion_Desktop();
    vysion_SaveFilesystem(&vysion_context);
    vysion_CloseAllWindows(&context);
}
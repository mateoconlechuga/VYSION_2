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
#include "vysion/gfx/output/vysion_gfx.h"
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
    strcpy((char *) os_OP1, temp);
}


int main(void) {
    //initialize the context and OPTIX environment
    struct vysion_context vysion_context;
    struct optix_cursor cursor;
    struct optix_colors colors;
    struct optix_settings settings;
    struct optix_gui_data data;
    struct optix_input input;
    //the final context (we'll add the stack last)
    struct optix_context context = {
        .settings = &settings,
        .cursor = &cursor,
        .data = &data,
        .colors = &colors,
        .input = &input,
    };
    vysion_SetContext(&vysion_context);
    vysion_InitializeGraphics();
    vysion_InitializeOPTIX(&context);
    vysion_InitializeFilesystem(&vysion_context);
    for (int i = 0; i < MAX_NUM_WINDOWS; i++) vysion_current_context->window[i] = NULL;
    vysion_Desktop();
    vysion_CloseAllWindows(&context);
}
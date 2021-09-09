#include "os.h"
#include <stdint.h>
#include <debug.h>
#include <string.h>
#include <tice.h>
#include <fileioc.h>
#include "filesystem.h"
#include "../main.h"

void vysion_SetOP1(uint8_t type, char *name) {
    //does this work?
    char temp[10] = " ";
    temp[0] = type;
    strcat(temp, name);
    strcpy(os_OP1, temp);
}

void vysion_RunProgramReturn(void) {
    os_PutStrFull("Successfully returned.");
    while (!os_GetCSC());
    main();
}


void vysion_RunProgram(struct vysion_file_save *save) {
    struct vysion_file *file = (struct vysion_file *) save;
    gfx_End();
    os_ClrHome();
    //vysion_SetOP1(save->ti_type, save->widget.name);
    if (file->vysion_type != VYSION_BASIC_TYPE && file->vysion_type != VYSION_PROTECTED_BASIC_TYPE) {
        os_PutStrFull("Running...");
        //vysion_asm_RunAsmProgram();
        os_RunPrgm(save->widget.name, NULL, 0, vysion_RunProgramReturn);
    } else {
        os_PutStrFull("Installing stop hook...");
        os_NewLine();
        vysion_asm_InstallFixStopHook();
        os_PutStrFull("Running...");
        os_RunPrgm(save->widget.name, NULL, 0, vysion_RunProgramReturn);
    }
}
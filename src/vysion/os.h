#ifndef OS_H
#define OS_H

//file for general TI-OS integrated functions
//for example, program running, editing, archiving, and the like

#include "filesystem.h"

void vysion_SetOP1(uint8_t type, char *name);
void vysion_RunProgram(struct vysion_file_save *save);

#endif
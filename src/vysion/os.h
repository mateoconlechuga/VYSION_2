#ifndef OS_H
#define OS_H

#include "filesystem.h"

//file for general TI-OS integrated functions
//for example, program running, editing, archiving, and the like

#define boot_IsCharging ((unsigned char (*)(void))0x3CC)

void vysion_RunProgram(char *name);

#endif
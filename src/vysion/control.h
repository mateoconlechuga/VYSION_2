#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>
#include "filesystem.h"

//the master control file for the whole shell
#define VYSION_VERSION            "2.0.0"
#define VYSION_SETTINGS_VERSION   2
#define VYSION_HOOKS_VERSION      2
#define VYSION_STATE_VERSION      2

//globals (should only need one)
extern struct vysion_context *vysion_current_context;

//functions
void vysion_SetContext(struct vysion_context *context);

#endif
#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>
#include "filesystem.h"
#include "settings.h"
#include "window_manager.h"

//the master control file for the whole shell
#define VYSION_VERSION_TEXT       "VYSION 2 CE Development\n0.0.38 (private, experimental)\nDo not distribute."
#define VYSION_SETTINGS_VERSION   2
#define VYSION_HOOKS_VERSION      2
#define VYSION_STATE_VERSION      2

struct vysion_context {
    struct vysion_settings settings;
    struct vysion_window *window[MAX_NUM_WINDOWS];
    int num_programs;
    int num_appvars;
    char *filesystem_start;
};

//globals (should only need one)
extern struct vysion_context *vysion_current_context;

//functions
void vysion_SetContext(struct vysion_context *context);
void vysion_End(void);


#endif
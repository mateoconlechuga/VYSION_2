#include "os.h"
#include <stdint.h>
#include <debug.h>
#include <string.h>
#include <tice.h>
#include <fileioc.h>
#include "filesystem.h"
#include "../main.h"

void vysion_RunProgram(char *name) {
    gfx_End();
    vysion_CloseAllWindows(vysion_current_context);
    os_ClrHome();
    os_RunPrgm(name, NULL, 0, main);
}
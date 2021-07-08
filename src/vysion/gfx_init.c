#include "gfx_init.h"
//includes
#include <stdint.h>
#include <graphx.h>
#include "vysion/gfx/output/gfx.h"

//functions
void vysion_InitializeGraphics(void) {
    gfx_Begin();
    gfx_SetDraw(gfx_buffer);
    //set up the palette
    gfx_SetPalette(palette, sizeof_palette, myimages_palette_offset);
}
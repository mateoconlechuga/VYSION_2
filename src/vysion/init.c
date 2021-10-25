#include "init.h"
//includes
#include <stdint.h>
#include <graphx.h>
#include "vysion/gfx/output/vysion_gfx.h"

#include "../optix/elements/text.h"
#include "../optix/colors.h"
#include "../optix/cursor.h"
#include "../optix/input.h"

//functions
void vysion_InitializeGraphics(void) {
    gfx_Begin();
    gfx_SetDraw(gfx_buffer);
    //set up the palette
    gfx_SetPalette(palette, sizeof_palette, myimages_palette_offset);
    gfx_SetPalette(xlibc_condensed_palette, sizeof_xlibc_condensed_palette, xlibc_condensed_palette_offset);

}

void vysion_InitializeOPTIX(struct optix_context *context) {
    optix_InitializeColors(context->colors);
    optix_InitializeCursor(context->cursor);
    optix_InitializeSettings(context->settings);
    optix_InitializeData(context->data);
    optix_InitializeInput(context->input);
    optix_SetContext(context);
    optix_InitializeGUIState();
    optix_InitializeFont();
}
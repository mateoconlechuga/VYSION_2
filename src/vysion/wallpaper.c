#include "wallpaper.h"

//quick test
void vysion_TestWallpaper(char *name) {
    ti_var_t slot;
    ti_CloseAll();
    if ((slot = ti_Open(name, "r"))) {
        uint16_t palette_size = 0;
        void *read_location = ti_GetDataPtr(slot) + strlen(HD_WALLAPER_HEADER_STRING);
        //note: this will be in bytes, not entries
        palette_size = *((uint16_t *) read_location);
        dbg_sprintf(dbgout, "Palette size is %d\n", palette_size);
        gfx_SetPalette(read_location + 2, palette_size, WALLPAPER_PALETTE_OFFSET);
        read_location += palette_size + 2;
        dbg_sprintf(dbgout, "Made it here. %d\n", (int) gfx_vbuffer);
        zx7_Decompress(gfx_vbuffer, read_location);
        //gfx_BlitBuffer();
        dbg_sprintf(dbgout, "Made it this far...\n");
    }
}
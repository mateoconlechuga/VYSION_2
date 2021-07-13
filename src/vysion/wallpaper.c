#include "wallpaper.h"
#include <debug.h>
#include "../optix/elements/sprite.h"

/*Some quick notes on the wallpaper system and format:
    -all high definition wallpapers will be prepended with VYSWALLHD
    -all low definition wallpapers (if implemented) will be prepended with
     VYSWALL or something
    -high definition wallpapers will be broken into 6 320x40 sections, for
     faster redrawing
    -high definition wallpapers will include a LUT at the beginning of the
     appvar
    -all wallpapers will include an embedded palette of not more than 128
     entries or 256 bytes
*/




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
        ti_Close(slot);
    }
}

//takes an array of optix_sprite structs, because that's how it will be set probably
//there will be another function to initialize these sprites' transforms and callbacks
void vysion_SetWallpaper(char *name, struct optix_sprite *sprite[HD_WALLPAPER_ROWS]) {
    ti_var_t slot;
    //we're assuming these have already been initialized
    ti_CloseAll();
    if ((slot = ti_Open(name, "r"))) {
        void *lut_location = ti_GetDataPtr(slot) + strlen(HD_WALLAPER_HEADER_STRING);
        void *lut_data_location = lut_location + 3 * sizeof(uint16_t);
        dbg_sprintf(dbgout, "Length of string: %d\n", strlen(HD_WALLAPER_HEADER_STRING));
        //so we have the LUT now
        //the offset to the first one (not including the header string) is the second byte of the LUT
        //this doesn't include the size
        dbg_sprintf(dbgout, "First tile: %d\n", *((uint16_t *) lut_location + sizeof(uint16_t)));
        void *first_tile_location = lut_location + *((uint16_t *) lut_location + sizeof(uint16_t));
        //the size of the lut should be a constant 18 for 6 sprites, we'll have it be a define
        //that's 2 bytes for size, 4 bytes for god knows what, andd then 12 bytes for the actual offsets
        void *palette_location = lut_location + HD_WALLPAPER_LUT_SIZE;
        uint16_t palette_size = *((uint16_t *) palette_location);
        dbg_sprintf(dbgout, "Palette size is %d\n", palette_size);
        //now set the palette
        gfx_SetPalette(palette_location + sizeof(uint16_t), palette_size, WALLPAPER_PALETTE_OFFSET);
        //and now for the actual things
        for (int i = 0; i < HD_WALLPAPER_ROWS; i++) {
            uint16_t *arr = lut_data_location;
            dbg_sprintf(dbgout, "Offset: %d\n", (int) arr[i]);
            //we have to add sizeof(uint16_t) to this because there are 2 bytes indicating the size of the palette
            //that won't otherwise be included
            sprite[i]->spr = first_tile_location + arr[i] + sizeof(uint16_t);
        }
    }
}

void vysion_InitializeWallpaper(struct optix_sprite *sprite[HD_WALLPAPER_ROWS]) {
    for (int i = 0; i < HD_WALLPAPER_ROWS; i++) {
        struct optix_sprite template = {
            .widget = {
                .transform = {
                    .x = 0,
                    .y = i * HD_WALLPAPER_HEIGHT,
                    .width = HD_WALLPAPER_WIDTH,
                    .height = HD_WALLPAPER_HEIGHT,
                },
            },
            //OPTIX just won't display it if it's null
            .spr = NULL,
            .transparent = false,
            .x_scale = 1,
            .y_scale = 1,
        };
        //template.widget.transform.y = i * HD_WALLPAPER_HEIGHT;
        dbg_sprintf(dbgout, "Y: %d\n", template.widget.transform.y);
        optix_InitializeWidget(&template.widget, OPTIX_SPRITE_TYPE);
        template.widget.render = vysion_RenderWallpaper;
        memcpy((void *) sprite[i], (void *) &template, sizeof(struct optix_sprite));
    }
}

//this is the function that will actually render the wallpaper
//pretty basic
void vysion_RenderWallpaper(struct optix_widget *widget) {
    struct optix_sprite *sprite = (struct optix_sprite *) widget;
    //then do the thing
    if (widget->state.visible && sprite->spr && widget->state.needs_redraw) {
        dbg_sprintf(dbgout, "Rendering wallpaper...\n");
        //let's just not talk about this line alright
        zx7_Decompress((void *) &gfx_vbuffer[widget->transform.y][0], sprite->spr);
        dbg_sprintf(dbgout, "Success.\n");
    }
}
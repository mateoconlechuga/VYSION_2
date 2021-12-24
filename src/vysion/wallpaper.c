#include "wallpaper.h"

#include <debug.h>

#include "filesystem.h"
#include "control.h"

#include "../optix/elements/sprite.h"
#include "../optix/elements/divider.h"
#include "../optix/elements/menu.h"
#include "../optix/gui_control.h"
#include "../optix/cursor.h"
#include "../optix/init.h"


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
        //so we have the LUT now
        //the offset to the first one (not including the header string) is the second byte of the LUT
        //this doesn't include the size
        void *first_tile_location = lut_location + *((uint16_t *) lut_location + sizeof(uint16_t));
        //the size of the lut should be a constant 18 for 6 sprites, we'll have it be a define
        //that's 2 bytes for size, 4 bytes for god knows what, andd then 12 bytes for the actual offsets
        void *palette_location = lut_location + HD_WALLPAPER_LUT_SIZE;
        uint16_t palette_size = *((uint16_t *) palette_location);
        //now set the palette
        gfx_SetPalette(palette_location + sizeof(uint16_t), palette_size, WALLPAPER_PALETTE_OFFSET);
        //and now for the actual things
        for (int i = 0; i < HD_WALLPAPER_ROWS; i++) {
            uint16_t *arr = lut_data_location;
            //we have to add sizeof(uint16_t) to this because there are 2 bytes indicating the size of the palette
            //that won't otherwise be included
            sprite[i]->spr = first_tile_location + arr[i] + sizeof(uint16_t);
        }
        ti_Close(slot);
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
        //let's just not talk about this line alright
        zx7_Decompress((void *) &gfx_vbuffer[widget->transform.y][0], sprite->spr);
    }
}

//this assumes that the wallpaper is going to be the first entry in the stack
//please don't have it be somewhere else or it will probably break in a very impressive fashion
void vysion_WallpaperPickerMenu_ClickAction(void *args) {
    struct vysion_wallpaper_picker_menu *menu = (struct optix_menu *) args;
    char *name = menu->menu.text[menu->menu.selection];
    dbg_sprintf(dbgout, "Click action...\n");
    menu->selection = menu->menu.selection;
    vysion_SetWallpaper(name, current_context->stack);
    current_context->data->gui_needs_full_redraw = true;
    dbg_sprintf(dbgout, "What the ....?");
}



//this is going to run independently of OPTIX (maybe make it its own context later)
//this will set the wallpaper name in the settings struct
void vysion_WallpaperPicker(void) {
    //first, we'll get a list of the available wallpapers
    //just assume that this is the number of files for the sake of argument
    static int i = 0;
    static int wallpapers_found = 0;
    static struct optix_cursor old_cursor_state;
    static struct optix_widget *stack[9];
    struct optix_widget **old_stack = current_context->stack;
    static char wallpaper_name[MAX_NUM_WALLPAPERS][WALLPAPER_NAME_MAX_LENGTH];
    static char *wallpaper_name_ptr[MAX_NUM_WALLPAPERS];
    static struct optix_sprite wallpaper[HD_WALLPAPER_ROWS];
    static struct optix_sprite *wallpaper_ptr[HD_WALLPAPER_ROWS];
    static void *search_pos = NULL;
    static char *var_name = NULL;
    static struct vysion_wallpaper_picker_menu wallpaper_menu = {
        .menu = {
            .widget = {
                .transform = {
                    .width = 90 - 12,
                    .height = LCD_HEIGHT,
                },
                .child = NULL,
            },
            .text_args = {
                .widget = {
                    .centering = {
                        .x_centering = OPTIX_CENTERING_LEFT,
                        .x_offset = 4,
                        .y_centering = OPTIX_CENTERING_CENTERED,
                    },
                },
            },
            .rows = LCD_HEIGHT / 16,
            .columns = 1,
            .text = (char **) wallpaper_name_ptr,
            .click_action = {
                .click_action = vysion_WallpaperPickerMenu_ClickAction,
            },
            .pass_self = true,
        },
        .selection = 0,
    };
    static struct optix_divider wallpaper_menu_divider = {
        .alignment = DIVIDER_ALIGNMENT_RIGHT,
        .reference = &wallpaper_menu.menu.widget,
    };
    optix_InitializeWidget(&wallpaper_menu_divider.widget, OPTIX_DIVIDER_TYPE);
    for (i = 0; i < HD_WALLPAPER_ROWS; i++) stack[i] = wallpaper_ptr[i] = &wallpaper[i];
    vysion_InitializeWallpaper(wallpaper_ptr);
    while ((var_name = ti_DetectVar(&search_pos, HD_WALLAPER_HEADER_STRING, TI_APPVAR_TYPE))) {
        dbg_sprintf(dbgout, "%s ", var_name);
        strcpy(wallpaper_name[wallpapers_found], var_name);
        wallpaper_name_ptr[wallpapers_found] = wallpaper_name[wallpapers_found];
        dbg_sprintf(dbgout, "%s\n", wallpaper_menu.menu.text[wallpapers_found]);
        wallpapers_found++;
    }
    //another if statement here eventually that compares the name to the active wallpaper's name and sets
    //the menu selection accordingly if it is
    wallpaper_name_ptr[wallpapers_found] = NULL;
    optix_InitializeWidget(&wallpaper_menu.menu.widget, OPTIX_MENU_TYPE);
    dbg_sprintf(dbgout, "Menu options: %d\n", wallpaper_menu.menu.num_options);
    //OPTIX things
    stack[6] = &wallpaper_menu.menu.widget;
    stack[7] = &wallpaper_menu_divider.widget;
    stack[8] = NULL;
    //save the old cursor state
    memcpy(&old_cursor_state, current_context->cursor, sizeof(struct optix_cursor));
    current_context->stack = stack;
    current_context->data->gui_needs_full_redraw = true;
    vysion_SetWallpaper("DEFAULT4", wallpaper_ptr);
    optix_SetCurrentSelection(&wallpaper_menu.menu.widget);
    do {
        optix_UpdateGUI();
        optix_RenderGUI();
    } while (!(kb_Data[6] & kb_Clear));
    //restore the old stack
    current_context->stack = old_stack;
    wallpaper_menu.menu.selection = wallpaper_menu.selection;
    vysion_WallpaperPickerMenu_ClickAction(&wallpaper_menu.menu.widget);
    //bring back the old cursor state
    memcpy(current_context->cursor, &old_cursor_state, sizeof(struct optix_cursor));
}
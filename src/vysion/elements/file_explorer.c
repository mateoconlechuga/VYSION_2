#include "file_explorer.h"
#include <graphx.h>
#include "../window_manager.h"
#include "../../optix/init.h"
#include "../gfx/output/gfx.h"

unsigned char *template_text_text = "Welcome to VYSION 2! Here is some more text, maybe that will change something.";
unsigned char start_icon_rotated[1154];

void vysion_AddFileExplorerWindow(void *config) {
    struct vysion_file_explorer_window_config *file_explorer_window_config = (struct vysion_file_explorer_window_config *) config; 
    struct vysion_file_explorer_window window = {
        .window = {.widget = {.type = WINDOW_FILE_EXPLORER}},
    };
    gfx_RotateSpriteC(start_icon, start_icon_rotated);
    //now for the actual window
    //window text (just a test)
    struct optix_text template_text = {
        .widget = {.transform = {.width = 94, .height = 14}},
        .text = template_text_text,
        .alignment = OPTIX_CENTERING_CENTERED,
        .x_offset = 1,
        .min = 0,
        .needs_offset_update = true,
        .background_rectangle = false,
    };
    optix_InitializeWidget(&template_text.widget, OPTIX_TEXT_TYPE);
    template_text.background_rectangle = false;
    template_text.widget.centering.y_centering = OPTIX_CENTERING_BOTTOM;
    struct optix_sprite template_sprite = {
        .widget = {
            .transform = {.width = start_icon->height, .height = start_icon->width},
            .centering = {.x_centering = OPTIX_CENTERING_LEFT, .y_centering = OPTIX_CENTERING_CENTERED},
        },
        .spr = start_icon_rotated,
        .transparent = false,
        .x_scale = 1,
        .y_scale = 1,
    };
    optix_InitializeWidget(&template_sprite.widget, OPTIX_SPRITE_TYPE);
    template_sprite.widget.centering.y_centering = OPTIX_CENTERING_TOP;
    struct optix_window template = {
        .widget = {
            .transform = {
                .x = 110,
                .y = 70,
                .width = 96,
                .height = 28,
            },
            .child = (struct optix_widget *[]) {&template_sprite, &template_text, NULL},
        },
        .resize_info = {
            .resizable = true,
            .min_width = 50,
            .min_height = 50,
        },
    };
    optix_InitializeWidget(&template.widget, OPTIX_WINDOW_TYPE);
    //optix_RecursiveAlign(&template.widget);
    //and now for the window title bar
    struct optix_window_title_bar template_title_bar = {
        .widget = {.child = NULL},
        .window = &template,
    };
    optix_InitializeWidget(&template_title_bar.widget, OPTIX_WINDOW_TITLE_BAR_TYPE);
    window.window.widget.window_title_bar = &template_title_bar;
    //and now copy the element
    vysion_AddWindow(&window);
}

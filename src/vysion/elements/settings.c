#include "settings.h"
#include <graphx.h>
#include "../window_manager.h"
#include "../../optix/init.h"
#include "../gfx/output/vysion_gfx.h"
#include "../control.h"


void vysion_AddSettingsWindow(void *config) {
    struct vysion_settings_window window = {
        .window = {.widget = {.type = WINDOW_SETTINGS}},
    };
    struct optix_sprite template_sprite = {
        .widget = {.centering = {.x_centering = OPTIX_CENTERING_CENTERED, .y_centering = OPTIX_CENTERING_CENTERED}},
        .spr = settings_system,
        .transparent = false,
    };
    struct optix_window template = {
        .widget = {
            .transform = {
                .x = 50,
                .y = 50,
                .width = 100,
                .height = 100,
            },
            //.child = (struct optix_widget *[]) {&template_sprite, NULL},
            .child = NULL,
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
    memcpy(&window.window.widget.window_title_bar, &template_title_bar, sizeof(struct optix_window_title_bar));
    vysion_AddWindow(&window);
}
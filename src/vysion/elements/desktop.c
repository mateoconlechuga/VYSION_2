#include "desktop.h"

//standard defines
#include <stdint.h>
#include <keypadc.h>

#include "../../optix/elements/sprite.h"
#include "../../optix/elements/rectangle.h"
#include "../../optix/elements/menu.h"
#include "../../optix/elements/button.h"
#include "../../optix/elements/text.h"
#include "../../optix/elements/window.h"
#include "../../optix/gui_control.h"
#include "../../optix/util.h"
#include "../../optix/init.h"
#include "../control.h"
#include "../gfx/output/gfx.h"
#include "../wallpaper.h"
#include "../window_manager.h"

//so this file is where all of the main loop stuff is going to be
//the desktop, which probably makes sense if you can read

void vysion_SuperButtonClickAction(void *args) {
    struct optix_widget *widget = (struct optix_widget *) args;
    dbg_sprintf(dbgout, "RUnning...\n");
    widget->state.visible ^= true;
    widget->state.needs_redraw = widget->state.selected = ((struct optix_window *) widget)->needs_focus = widget->state.visible;
    if (widget->state.visible && !current_context->settings->cursor_active) {
        dbg_sprintf(dbgout, "Setting current selection.\n");
        optix_SetCurrentSelection(widget);
        current_context->cursor->direction = OPTIX_CURSOR_FORCE_UPDATE;
    }
    current_context->data->gui_needs_full_redraw = true;
    //optix_IntelligentRecursiveSetNeedsRedraw((*current_context->stack), widget);
    dbg_sprintf(dbgout, "I fucking hate this motherfucker\n");
    //current_context->data->gui_needs_full_redraw = true;
}

void vysion_UpdateStartMenu(struct optix_widget *widget) {
    



}

//the action the start menu does when clicked
void vysion_StartMenuClickAction(void *args) {
    int *selection = (int *) args;
    switch (*selection) {
        case 5:
            gfx_End();
            exit(0);
            break;
        default:
            vysion_AddFileExplorerWindow(NULL);
            break;
    }
}




//we're assuming the context has been initialized before this, of course
void vysion_Desktop(void) {
    //make a stack of things to update, just start with a rectangle and a button
    //elements
    struct optix_sprite wallpaper[HD_WALLPAPER_ROWS];
    struct optix_sprite *wallpaper_ptr[HD_WALLPAPER_ROWS];
    for (int i = 0; i < HD_WALLPAPER_ROWS; i++) wallpaper_ptr[i] = &wallpaper[i];
    vysion_InitializeWallpaper(wallpaper_ptr);
    vysion_SetWallpaper("R32", wallpaper_ptr);
    //a window
    struct optix_sprite start_sidebar_icon = {
        .widget = {
            .transform = {.width = start_icon->width, .height = start_icon->height},
            .centering = {.x_centering = OPTIX_CENTERING_LEFT, .y_centering = OPTIX_CENTERING_CENTERED},
        },
        .spr = start_icon,
        .transparent = false,
        .x_scale = 1,
        .y_scale = 1,
    };
    optix_InitializeWidget(&start_sidebar_icon.widget, OPTIX_SPRITE_TYPE);
    start_sidebar_icon.widget.centering.x_centering = OPTIX_CENTERING_LEFT;
    //the actual menu for this part
    char *start_menu_text[] = {"Files", "Search", "Settings", "About", "Refresh", "Exit", NULL};
    gfx_sprite_t *start_menu_icon[] = {start_files, start_search, start_settings, start_about, start_refresh, start_exit, NULL};
    struct optix_menu start_menu = {
        .widget = {
            .transform = {
                .width = 90 - 12,
                .height = 96,
            },
        },
        .resize_info = {
            .resizable = true,
            .x_lock = true,
            .y_lock = true,
            .min_width = 25,
            .min_height = 25,
        },
        .text_centering = {.y_centering = OPTIX_CENTERING_CENTERED, .x_centering = OPTIX_CENTERING_LEFT, .x_offset = 20},
        .sprite_centering = {.y_centering = OPTIX_CENTERING_CENTERED, .x_centering = OPTIX_CENTERING_LEFT, .x_offset = 4},
        .rows = 6,
        .columns = 1,
        .text = start_menu_text,
        .spr = start_menu_icon,
        .click_action = vysion_StartMenuClickAction,
    };
    start_menu.click_args = (void *) &start_menu.selection;
    optix_InitializeWidget(&start_menu.widget, OPTIX_MENU_TYPE);
    start_menu.widget.centering.x_centering = OPTIX_CENTERING_RIGHT;
    start_menu.widget.centering.y_centering = OPTIX_CENTERING_TOP;
    struct optix_window start_window = {
        .widget = {.transform = {.x = 1, .y = 223 - 96, .width = 90, .height = 96}, .child = (struct optix_widget *[]) {&start_sidebar_icon.widget, &start_menu.widget, NULL}},
    };
    optix_InitializeWidget(&start_window.widget, OPTIX_WINDOW_TYPE);
    optix_RecursiveAlign(&start_window.widget);
    start_window.widget.state.visible = false;
    struct optix_sprite super_icon = {
        .widget = {
            .transform = {.width = taskbar_icon->width, .height = taskbar_icon->height},
            .centering = {.x_centering = OPTIX_CENTERING_CENTERED, .y_centering = OPTIX_CENTERING_CENTERED},
        },
        .spr = taskbar_icon,
        .transparent = true,
        .transparent_color = myimages_palette_offset,
        .x_scale = 1,
        .y_scale = 1,
    };
    optix_InitializeWidget(&super_icon.widget, OPTIX_SPRITE_TYPE);
    struct optix_button super_button = {
        .widget = {
            .transform = {
                .x = 0,
                .y = LCD_HEIGHT - 16,
                .width = 32,
                .height = 16,
            },
            .child = (struct optix_widget *[]) {&super_icon.widget, NULL},
        },
        .click_action = vysion_SuperButtonClickAction,
        .click_args = (void *) &start_window.widget,
        .alternate_key = sk_Yequ,
    };
    optix_InitializeWidget(&super_button.widget, OPTIX_BUTTON_TYPE);
    struct optix_text clock_text = {
        .widget = {
            .transform = {
                .x = LCD_WIDTH - 46,
                .y = LCD_HEIGHT - 14,
                .width = 40,
                .height = 14,
            },
        },
        .alignment = OPTIX_CENTERING_CENTERED,
        .text = "12:00 AM",
        .background_rectangle = false,
    };
    optix_InitializeWidget(&clock_text.widget, OPTIX_TEXT_TYPE);
    clock_text.background_rectangle = false;
    optix_RecursiveAlign(&super_button.widget);
    //dbg_sprintf(dbgout, "Type: %d\n", vysion_context.window[0]->widget.window_title_bar->widget.type);
    struct optix_widget *master_stack[HD_WALLPAPER_ROWS + MAX_NUM_WINDOWS] = {&super_button.widget, &start_window.widget, &clock_text.widget, NULL};
    //inititialize that too
    for (int i = 0; i < HD_WALLPAPER_ROWS + MAX_NUM_WINDOWS; i++) {
        if (i < HD_WALLPAPER_ROWS) master_stack[i] = wallpaper_ptr[i];
        else {
            switch (i - HD_WALLPAPER_ROWS) {
                case 0:
                    master_stack[i] = &super_button.widget;
                    break;
                case 1:
                    master_stack[i] = &start_window.widget;
                    break;
                case 2:
                    master_stack[i] = &clock_text.widget;
                    break;
            }
        }
    }
    //add it to the context
    current_context->stack = &master_stack;
    optix_SetCurrentSelection(&super_button.widget);
    optix_RefreshCursorBackground((struct optix_widget *) current_context->cursor);
    do {
        optix_UpdateGUI();
        optix_RenderGUI();
        gfx_Blit(1);
    } while (!(kb_Data[6] & kb_Clear));
    //ti_CloseAll();
    //vysion_LoadFilesystem(&context);
    //vysion_DetectAllFiles(&context);
    //end, saving everything
    //vysion_SaveFilesystem(&context);
    gfx_End();
}

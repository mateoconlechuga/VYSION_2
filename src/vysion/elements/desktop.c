#include "desktop.h"

//standard defines
#include <stdint.h>
#include <keypadc.h>
#include <tice.h>

#include "../../optix/elements/sprite.h"
#include "../../optix/elements/rectangle.h"
#include "../../optix/elements/menu.h"
#include "../../optix/elements/button.h"
#include "../../optix/elements/text.h"
#include "../../optix/elements/window.h"
#include "../../optix/gui_control.h"
#include "../../optix/util.h"
#include "../../optix/init.h"
#include "../../optix/cursor.h"
#include "../../optix/loop.h"
#include "../control.h"
#include "../gfx/output/vysion_gfx.h"
#include "../wallpaper.h"
#include "../window_manager.h"
#include "file_explorer.h"
#include "settings.h"
#include "clock.h"
#include "battery.h"

//so this file is where all of the main loop stuff is going to be
//the desktop, which probably makes sense if you can read

void vysion_SuperButtonClickAction(void *args) {
    struct optix_widget *widget = (struct optix_widget *) args;
    static struct optix_widget *last_selection;
    widget->state.visible ^= true;
    widget->state.needs_redraw = widget->state.selected = widget->state.visible;
    if (widget->state.visible) {
        optix_MoveWidgetToTop(widget);
        ((struct optix_window *) widget)->active = true;
        if (!current_context->settings->cursor_active) {
            last_selection = current_context->cursor->current_selection;
            optix_SetCurrentSelection(widget->child[1]);
        }
    } else {
        if (!current_context->settings->cursor_active) optix_SetCurrentSelection(last_selection);
        ((struct optix_window *) widget)->active = false;
    }
    current_context->data->gui_needs_full_redraw = true;
    //current_context->data->gui_needs_full_redraw = true;
}

void vysion_UpdateStartWindow(struct optix_widget *widget) {
    struct optix_window *window = (struct optix_window *) widget;
    if (window->widget.state.visible && !window->active) {
        window->widget.state.visible = false;
        current_context->data->gui_needs_full_redraw = true;
    }
    optix_UpdateWindow_default(widget);
}

//the action the start menu does when clicked
void vysion_StartMenuClickAction(void *args) {
    int *selection = (int *) args;
    switch (*selection) {
        case 5:
            gfx_End();
            exit(0);
            break;
        case 1:
            vysion_AddSettingsWindow(NULL);
            break;
        case 2:
            dbg_sprintf(dbgout, "What");
            vysion_WallpaperPicker();
            break;
        default:
            vysion_AddFileExplorerWindow(NULL);
            break;

    }
    //just close it
    current_context->data->key = sk_Yequ;
}




//we're assuming the context has been initialized before this, of course
void vysion_Desktop(void) {
    //make a stack of things to update, just start with a rectangle and a button
    //elements
    struct optix_sprite wallpaper[HD_WALLPAPER_ROWS];
    struct optix_sprite *wallpaper_ptr[HD_WALLPAPER_ROWS];
    for (int i = 0; i < HD_WALLPAPER_ROWS; i++) wallpaper_ptr[i] = NULL;
    for (int i = 0; i < HD_WALLPAPER_ROWS; i++) wallpaper_ptr[i] = &wallpaper[i];
    vysion_InitializeWallpaper(wallpaper_ptr);
    vysion_SetWallpaper("CONSTRUCT", wallpaper_ptr);
    //a window
    //window manager thing
    char *desktop_window_manager_text[MAX_NUM_WINDOWS + 1];
    gfx_sprite_t *desktop_window_manager_spr[MAX_NUM_WINDOWS + 1];
    struct vysion_window_manager_menu desktop_window_manager = {
        .menu = {
            .widget = {
                .transform = {
                    .x = 32,
                    .y = LCD_HEIGHT - TASKBAR_HEIGHT,
                    .height = WINDOW_MANAGER_MENU_ROW_HEIGHT,
                },
                .centering = {.x_centering = OPTIX_CENTERING_LEFT, .x_offset = 34, .y_centering = OPTIX_CENTERING_BOTTOM},
            },
            .text_args = {
                .widget = {
                    .centering = {
                        .x_centering = OPTIX_CENTERING_LEFT,
                        .x_offset = 16,
                        .y_centering = OPTIX_CENTERING_CENTERED,
                    },
                },
            },
            .sprite_args = {
                .widget = {
                    .centering = {
                        .x_centering = OPTIX_CENTERING_LEFT,
                        .x_offset = 2,
                        .y_centering = OPTIX_CENTERING_CENTERED,
                    },
                },
                .transparent = true,
                .transparent_color = myimages_palette_offset,
            },
            .text = desktop_window_manager_text,
            .spr = desktop_window_manager_spr,
            .rows = 1,
            .transparent_background = true,
            .hide_selection_box = true,
            //click args
            .click_action = {.click_action = vysion_WindowManagerMenuClickAction},
            .pass_self = true,
        },
        .last_num_windows = 0,
        .needs_update = true,
    };
    optix_InitializeWidget(&desktop_window_manager, OPTIX_MENU_TYPE);
    desktop_window_manager.menu.widget.state.selectable = false;
    desktop_window_manager.menu.widget.update = vysion_UpdateWindowManagerMenu;
    desktop_window_manager.menu.widget.render = vysion_RenderWindowManagerMenu;
    //desktop_window_manager.menu.spr_x_scale = desktop_window_manager.menu.spr_y_scale = 2;
    //the main file explorer thing
    struct vysion_file_explorer_menu desktop_file_menu = {
        .menu = {
            .widget = {
                .transform = {
                    //.x = WINDOW_MANAGER_MENU_ROW_HEIGHT,
                    .x = 0,
                    .y = 0,
                    //.width = LCD_WIDTH - WINDOW_MANAGER_MENU_ROW_HEIGHT,
                    .width = LCD_WIDTH,
                    .height = LCD_HEIGHT - TASKBAR_HEIGHT,
                },
            },
            .text_args = {
                .widget = {
                    .centering = {
                        .x_centering = OPTIX_CENTERING_CENTERED,
                        .y_centering = OPTIX_CENTERING_BOTTOM,
                        .y_offset = -5,
                    },
                },
            },
            .sprite_args = {
                .widget = {
                    .centering = {
                        .x_centering = OPTIX_CENTERING_CENTERED,
                        .y_centering = OPTIX_CENTERING_TOP,
                        .y_offset = 5,
                    },
                },
                .x_scale = 2,
                .y_scale = 2,
            },
            .text = NULL,
            .spr = NULL,
            .rows = 4,
            .columns = 5,
            .click_action = {.click_action = vysion_FileExplorerMenuClickAction},
            .pass_self = true,
            .hide_selection_box = true,
        },
        .offset = 0,
        .needs_update = true,
        .nest = true,
    };
    //vysion_UpdateFileExplorerMenu(&desktop_file_menu.menu.widget);
    optix_InitializeWidget(&desktop_file_menu.menu.widget, OPTIX_MENU_TYPE);
    //desktop_file_menu.menu.widget.update = vysion_UpdateFileExplorerMenu;
    desktop_file_menu.menu.transparent_background = true;
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
            .child = NULL,
        },
        .resize_info = {
            .resizable = true,
            .x_lock = true,
            .y_lock = true,
            .min_width = 25,
            .min_height = 25,
        },
        .text_args = {
            .widget = {
                .centering = {
                    .x_centering = OPTIX_CENTERING_LEFT,
                    .x_offset = 20,
                    .y_centering = OPTIX_CENTERING_CENTERED,
                },
            },
            .background_rectangle = false,
        },
        .sprite_args = {
            .widget = {
                .centering = {
                    .x_centering = OPTIX_CENTERING_LEFT,
                    .x_offset = 4,
                    .y_centering = OPTIX_CENTERING_CENTERED,
                },
            },
            .transparent = true,
            .transparent_color = myimages_palette_offset,
        },
        .rows = 6,
        .columns = 1,
        .text = start_menu_text,
        .spr = start_menu_icon,
        .click_action = {.click_action = vysion_StartMenuClickAction},
    };
    start_menu.click_action.click_args = (void *) &start_menu.selection;
    optix_InitializeWidget(&start_menu.widget, OPTIX_MENU_TYPE);
    start_menu.widget.centering.x_centering = OPTIX_CENTERING_RIGHT;
    start_menu.widget.centering.y_centering = OPTIX_CENTERING_TOP;
    struct optix_window start_window = {
        .widget = {.transform = {.x = 1, .y = 223 - 96, .width = 90, .height = 96}, .child = (struct optix_widget *[]) {&start_sidebar_icon.widget, &start_menu.widget, NULL}},
    };
    optix_InitializeWidget(&start_window.widget, OPTIX_WINDOW_TYPE);
    optix_RecursiveAlign(&start_window.widget);
    start_window.widget.state.visible = false;
    start_window.widget.update = vysion_UpdateStartWindow;
    struct optix_sprite super_icon = {
        .widget = {
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
                .y = LCD_HEIGHT - TASKBAR_HEIGHT,
                .width = 32,
                .height = 16,
            },
            .child = (struct optix_widget *[]) {&super_icon.widget, NULL},
        },
        .click_action = {
            .click_action = vysion_SuperButtonClickAction,
            .click_args = (void *) &start_window.widget,
        },
        .alternate_key = sk_Yequ,
        .transparent_background = true,
        .hide_selection_box = true,
    };
    optix_InitializeWidget(&super_button.widget, OPTIX_BUTTON_TYPE);
    struct vysion_clock_text clock_text = {
        .text = {
            .widget = {
                .transform = {
                    .x = LCD_WIDTH - 46,
                    .y = LCD_HEIGHT - 13,
                    .width = 40,
                    .height = fontlib_GetCurrentFontHeight(),
                },
            },
            .alignment = OPTIX_CENTERING_RIGHT,
            .text = clock_text_buffer,
            .background_rectangle = false,
        },
        .minutes = 255,
    };
    optix_InitializeWidget(&clock_text.text.widget, OPTIX_TEXT_TYPE);
    clock_text.text.widget.update = vysion_UpdateClockText;
    clock_text.text.widget.update(&clock_text.text.widget);
    clock_text.text.widget.state.selectable = false;
    struct vysion_battery_icon battery_icon = {
        .sprite = {
            .widget = {
                .transform = {
                    .x = clock_text.text.widget.transform.x - battery_0->width,
                    .y = LCD_HEIGHT - (TASKBAR_HEIGHT / 2) - (battery_0->height / 2),
                },
            },
            .spr = battery_0,
            .x_scale = 1,
            .y_scale = 1,
            .transparent = true,
            .transparent_color = myimages_palette_offset,
        },
        .status = 0,
        .minutes = &(clock_text.minutes),
        .last_minutes = 255,
    };
    optix_InitializeWidget(&battery_icon.sprite.widget, OPTIX_SPRITE_TYPE);
    battery_icon.sprite.widget.update = vysion_UpdateBatteryIcon;
    dbg_sprintf(dbgout, "So that broke.");
    struct optix_text version_text = {
        .widget = {
            .transform = {
                .x = 2,
                .y = 1,
                .width = 200,
                .height = 100,
            },
        },
        .text = VYSION_VERSION_TEXT,
        .background_rectangle = false,
    };
    optix_InitializeWidget(&version_text.widget, OPTIX_TEXT_TYPE);
    version_text.widget.state.selectable = false;
    //version_text.num_lines = 2;
    struct optix_rectangle taskbar_background = {
        .widget = {
            .transform = {
                .0,
                .y = LCD_HEIGHT - 16,
                .width = LCD_WIDTH,
                .height = 16,
            },
        },
        .filled = true,
        .border_color = WINDOW_BORDER_BEVEL_LIGHT_INDEX,
        .fill_color = WINDOW_BG_COLOR_INDEX,
    };
    optix_InitializeWidget(&taskbar_background, OPTIX_RECTANGLE_TYPE);
    optix_RecursiveAlign(&super_button.widget);
    struct optix_widget *master_stack[DESKTOP_ELEMENTS + MAX_NUM_WINDOWS];
    //inititialize that too
    for (int i = 0; i < DESKTOP_ELEMENTS + 1; i++) {
        if (i < HD_WALLPAPER_ROWS) master_stack[i] = wallpaper_ptr[i];
        else {
            switch (i - HD_WALLPAPER_ROWS) {
                case 0:
                    master_stack[i] = &super_button.widget;
                    break;
                case 1:
                    master_stack[i] = &battery_icon.sprite.widget;
                    break;
                case 2:
                    master_stack[i] = &clock_text.text.widget;
                    break;
                case 3:
                    master_stack[i] = &desktop_window_manager.menu.widget;
                    break;
                case 4:
                    master_stack[i] = &start_window.widget;
                    break;
                case 5:
                    master_stack[i] = &version_text.widget;
                    break;
                default:
                    master_stack[i] = NULL;
                    break;
            }
        }
    }
    //add it to the context
    current_context->stack = master_stack;
    optix_SetCurrentSelection(&super_button.widget);
    optix_RefreshCursorBackground((struct optix_widget *) current_context->cursor);
    //we'll see
    /*int i = 0;
    for (;;) {
        dbg_sprintf(dbgout, "%d\n", i++);
        vysion_AddFileExplorerWindow(NULL);
        vysion_CloseWindow(&master_stack[DESKTOP_ELEMENTS]);
    }*/
    do {
        optix_UpdateGUI();
        optix_RenderGUI();
    } while (!((kb_Data[6] & kb_Clear) && kb_Data[1] & kb_Yequ));
    gfx_End();
}

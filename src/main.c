/*[NAME]    VYSION CE
  [VERSION] 2.0.0
  [AUTHOR]  epsilon5
  [DATE]    2021-04-05
*/

#include "main.h"
#include "vysion/defines.h"
#include "vysion/filesystem.h"
#include "vysion/control.h"
#include "vysion/wallpaper.h"
#include "vysion/gfx_init.h"
#include <fileioc.h>
#include <graphx.h>
#include <debug.h>
#include "vysion/gfx/output/gfx.h"
#include "vysion/elements/file_explorer.h"



void vysion_asm_ConfigureRunProgram(char *test, bool is_asm, bool use_reloader);
//  void *vRam = (void *) 0xD40000;
//  memcpy(vRam, test, 10);
//  *((uint8_t *) vRam + 10) = (uint8_t) is_asm;
//  *((uint8_t *) vRam + 11) = (uint8_t) use_reloader;
//};


void vysion_asm_RunProgram(void);

void vysion_SetOP1(uint8_t type, char name[]) {
    //does this work?
    char temp[10] = " ";
    temp[0] = type;
    strcat(temp, name);
    strcpy(os_OP1, temp);
}

void button_click_action(void *args) {
    struct optix_widget *widget = (struct optix_widget *) args;
    dbg_sprintf(dbgout, "RUnning...\n");
    widget->state.visible = true;
    widget->state.needs_redraw = true;
    widget->state.selected = true;
    dbg_sprintf(dbgout, "I fucking hate this motherfucker\n");
    //current_context->data->gui_needs_full_redraw = true;
}

void vysion_UpdateStartMenu(struct optix_widget *widget) {
    



}

void start_menu_click_action(void *args) {
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


void main(void) {
    //initialize the context and OPTIX environment
    struct vysion_context vysion_context = {
        .filesystem_info_save = {.num_files = 5, .num_folders = 0},
    };
    vysion_SetContext(&vysion_context);
    vysion_InitializeGraphics();
    //OPTIX stuff
    struct optix_cursor cursor;
    struct optix_colors colors;
    struct optix_settings settings;
    struct optix_gui_data data;
    //the final context (we'll add the stack last)
    struct optix_context context = {
        .settings = &settings,
        .cursor = &cursor,
        .data = &data,
        .colors = &colors,
    };
    optix_InitializeColors(&colors);
    optix_InitializeCursor(&cursor);
    optix_InitializeSettings(&settings);
    optix_InitializeData(&data);
    optix_SetContext(&context);
    optix_InitializeGUIState();
    optix_InitializeFont();
    //make a stack of things to update, just start with a rectangle and a button
    //elements
    //a window
    struct optix_sprite test_sprite_2 = {
        .widget = {
            .transform = {.width = start_icon->width, .height = start_icon->height},
            .centering = {.x_centering = OPTIX_CENTERING_LEFT, .y_centering = OPTIX_CENTERING_CENTERED},
        },
        .spr = start_icon,
        .transparent = false,
        .x_scale = 1,
        .y_scale = 1,
    };
    optix_InitializeWidget(&test_sprite_2.widget, OPTIX_SPRITE_TYPE);
    test_sprite_2.widget.centering.x_centering = OPTIX_CENTERING_LEFT;
    //the actual menu for this part
    char *test_text_b[] = {"Files", "Search", "Settings", "About", "Refresh", "Exit"};
    gfx_sprite_t *test_sprite_b[] = {start_files, start_search, start_settings, start_about, start_refresh, start_exit};
    struct optix_menu test_menu_b = {
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
        .text = test_text_b,
        .spr = test_sprite_b,
        .click_action = start_menu_click_action,
    };
    test_menu_b.click_args = (void *) &test_menu_b.selection;
    optix_InitializeWidget(&test_menu_b.widget, OPTIX_MENU_TYPE);
    test_menu_b.widget.centering.x_centering = OPTIX_CENTERING_RIGHT;
    test_menu_b.widget.centering.y_centering = OPTIX_CENTERING_TOP;
    struct optix_window test_window = {
        .widget = {.transform = {.x = 1, .y = 223 - 96, .width = 90, .height = 96}, .child = (struct optix_widget *[]) {&test_sprite_2.widget, &test_menu_b.widget, NULL}},
    };
    optix_InitializeWidget(&test_window.widget, OPTIX_WINDOW_TYPE);
    optix_RecursiveAlign(&test_window.widget);
    test_window.widget.state.visible = false;
    struct optix_rectangle test_rectangle = {
        .widget = {
            .transform = {
                .x = 0,
                .y = 224,
                .width = LCD_WIDTH,
                .height = 16,
            },
        },
        .filled = true,
        .border_color = 0,
        .fill_color = 0,
    };
    optix_InitializeWidget(&test_rectangle.widget, OPTIX_RECTANGLE_TYPE);
    struct optix_sprite test_sprite = {
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
    optix_InitializeWidget(&test_sprite.widget, OPTIX_SPRITE_TYPE);
    struct optix_button test_button = {
        .widget = {
            .transform = {
                .x = 0,
                .y = LCD_HEIGHT - 16,
                .width = 32,
                .height = 16,
            },
            .child = (struct optix_widget *[]) {&test_sprite.widget, NULL},
        },
        .click_action = button_click_action,
        .click_args = (void *) &test_window.widget,
        .alternate_key = sk_Yequ,
    };
    optix_InitializeWidget(&test_button.widget, OPTIX_BUTTON_TYPE);
    struct optix_text test_text = {
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
    optix_InitializeWidget(&test_text.widget, OPTIX_TEXT_TYPE);
    test_text.background_rectangle = false;
    optix_RecursiveAlign(&test_button.widget);
    //dbg_sprintf(dbgout, "Type: %d\n", vysion_context.window[0]->widget.window_title_bar->widget.type);
    struct optix_widget *test_stack[MAX_NUM_WINDOWS] = {&test_button.widget, &test_window.widget, &test_text.widget, NULL};
    //add it to the context
    context.stack = &test_stack;
    dbg_sprintf(dbgout, "Type (main): %d Type (right): %d\n", (*(context.stack))[0]->type, test_stack[0]->type);
    //vysion_AddFileExplorerWindow(NULL);
    //vysion_AddFileExplorerWindow(NULL);
    optix_SetCurrentSelection(&test_button.widget);
    vysion_TestWallpaper("BLUELINE");
    optix_RefreshCursorBackground(&cursor.widget);
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
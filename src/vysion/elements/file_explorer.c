#include "file_explorer.h"
#include <graphx.h>
#include "../window_manager.h"
#include "../../optix/init.h"
#include "../gfx/output/vysion_gfx.h"
#include "../control.h"
#include "../os.h"

//unsigned char *template_text_text = "Welcome to VYSION 2!";
//unsigned char start_icon_rotated[1154];

void vysion_AddFileExplorerWindow(void *config) {
    struct vysion_file_explorer_window_config *file_explorer_window_config = (struct vysion_file_explorer_window_config *) config; 
    struct vysion_file_explorer_window window = {
        .window = {.widget = {.type = WINDOW_FILE_EXPLORER}},
    };
    //now for the actual window
    //window text (just a test)
    struct vysion_file_explorer_menu template_file_menu = {
        .menu = {
            .widget = {
                .transform = {
                    .width = 100,
                    .height = 100,
                },
                .state = {
                    .override_size = true,
                    .size = sizeof(struct vysion_file_explorer_menu),
                },
                .child = NULL,
            },
            .dynamic = true,
            .text = NULL,
            .spr = NULL,
            .text_centering = {.y_centering = OPTIX_CENTERING_CENTERED, .x_centering = OPTIX_CENTERING_LEFT, .x_offset = 24},
            .sprite_centering = {.y_centering = OPTIX_CENTERING_CENTERED, .x_centering = OPTIX_CENTERING_LEFT, .x_offset = 4},
            .rows = 5,
            .columns = 1,
            .click_action = {.click_action = vysion_FileExplorerMenuClickAction},
            .pass_self = true,
        },
        .index = VYSION_ROOT,
        .needs_update = true,
        .nest = true,
    };
    //do this now, or there will be a slight delay until the correct options are shown
    vysion_UpdateFileExplorerMenu(&template_file_menu.menu.widget);
    optix_InitializeWidget(&template_file_menu.menu.widget, OPTIX_MENU_TYPE);
    template_file_menu.menu.widget.update = vysion_UpdateFileExplorerMenu;
    //vysion_UpdateFileExplorerMenu(&template_file_menu.menu.widget);
    struct optix_window template = {
        .widget = {
            .transform = {
                .x = 110,
                .y = 70,
                .width = 100,
                .height = 100,
            },
            .child = (struct optix_widget *[]) {&template_file_menu, NULL},
            //.child = NULL,
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

//a custom callback for a file explorer window
void vysion_UpdateFileExplorerMenu(struct optix_widget *widget) {
    struct vysion_file_explorer_menu *menu = (struct vysion_file_explorer_menu *) widget;
    //do this too (update the thing's stuff)
    //start out by changing the index if necessary
    if (menu->menu.widget.state.selected && current_context->data->key == sk_Mode) {
        menu->index = menu->folder->save.widget.location;
        menu->needs_update = true;
        menu->menu.widget.state.needs_redraw = true;
        if (menu->menu.transparent_background) optix_IntelligentRecursiveSetNeedsRedraw(current_context->stack, widget);
        menu->menu.min = menu->menu.selection = menu->menu.last_selection = 0;
    }
    if (menu->needs_update || widget->state.needs_redraw) {
        struct vysion_folder *folder = menu->folder = vysion_GetFolderByIndex(vysion_current_context, menu->index);
        int num_options = optix_GetNumElementsInStack((struct optix_widget **) folder->contents);
        menu->menu.num_options = num_options;
        //update it
        //pretty easy, I think
        menu->menu.text = realloc(menu->menu.text, (num_options + 1) * sizeof(char *));
        menu->menu.spr = realloc(menu->menu.spr, (num_options + 1) * sizeof(char *));
        menu->menu.text[num_options] = menu->menu.spr[num_options] = NULL;
        for (int i = 0; i < num_options; i++) {
            struct vysion_file *file_widget = (struct vysion_file *) folder->contents[i];
            menu->menu.text[i] = folder->contents[i]->save.widget.name;
            if (file_widget->save.widget.type == VYSION_FILE) menu->menu.spr[i] = file_widget->icon;
            else if (file_widget->save.widget.type == VYSION_FOLDER) menu->menu.spr[i] = ((struct vysion_folder *) file_widget)->icon;
        }
        menu->needs_update = false;
    }
    //still this as well
    optix_UpdateMenu_default(widget);
}

void vysion_FileExplorerMenuClickAction(struct optix_widget *widget) {
    struct vysion_file_explorer_menu *menu = (struct vysion_file_explorer_menu *) widget;
    struct vysion_file_widget *file_widget = (struct vysion_file_widget *) menu->folder->contents[menu->menu.selection];
    //so if it was a folder and was clicked, it should move to the next level if applicable
    dbg_sprintf(dbgout, "This function called.\n");
    switch (file_widget->type) {
        case VYSION_FOLDER:
            if (menu->nest) {
                menu->menu.widget.state.needs_redraw = true;
                if (menu->menu.transparent_background) optix_IntelligentRecursiveSetNeedsRedraw(current_context->stack, widget);
                menu->index = ((struct vysion_folder *) file_widget)->save.index;
                menu->needs_update = true;
                //just set the index and min to 0
                menu->menu.min = menu->menu.selection = 0;
                //otherwise it'll immediately switch to the new one
                menu->menu.pressed = true;
            }
            break;
        default:
            //run it I guess
            vysion_RunProgram((struct vysion_file_save *) file_widget);
            break;
    }
}
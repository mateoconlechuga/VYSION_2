#include "file_explorer.h"
#include <graphx.h>
#include <fileioc.h>
#include "../window_manager.h"
#include "../../optix/init.h"
#include "../gfx/output/vysion_gfx.h"
#include "../control.h"
#include "../os.h"
#include "../filesystem.h"

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
                .child = NULL,
            },
            .dynamic = true,
            .text = NULL,
            .spr = NULL,
            .text_args = {
                .widget = {
                    .centering = {
                        .y_centering = OPTIX_CENTERING_CENTERED,
                        .x_centering = OPTIX_CENTERING_LEFT,
                        .x_offset = 24,
                    },
                },
            },
            .sprite_args = {
                .widget = {
                    .centering = {
                        .y_centering = OPTIX_CENTERING_CENTERED,
                        .x_centering = OPTIX_CENTERING_LEFT,
                        .x_offset = 4,
                    },
                },
            },
            .rows = 5,
            .columns = 1,
            .click_action = {.click_action = vysion_FileExplorerMenuClickAction},
            .pass_self = true,
            .hide_selection_box = false,
            .transparent_background = false,
        },
        .pos = vysion_current_context->filesystem_start,
        .current_selection = vysion_current_context->filesystem_start,
        .needs_update = true,
        .nest = true,
        .special_folder = false,
        .special_folder_index = FOLDER_PROGRAMS,
    };
    //do this now, or there will be a slight delay until the correct options are shown
    //vysion_UpdateFileExplorerMenu(&template_file_menu.menu.widget);
    optix_InitializeWidget(&template_file_menu.menu.widget, OPTIX_MENU_TYPE);
    template_file_menu.menu.widget.state.override_size = true;
    template_file_menu.menu.widget.state.size = sizeof(struct vysion_file_explorer_menu);
    //vysion_UpdateFileExplorerMenu(&template_file_menu.menu.widget);
    template_file_menu.menu.widget.render = vysion_RenderAndUpdateFileExplorerMenu;
    template_file_menu.menu.num_options = 3;
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
        .widget = {
            .state = {.override_size = false},
            .child = NULL,
        },
        .window = &template,
    };
    optix_InitializeWidget(&template_title_bar.widget, OPTIX_WINDOW_TITLE_BAR_TYPE);
    memcpy(&window.window.widget.window_title_bar, &template_title_bar, sizeof(struct optix_window_title_bar));
    //and now copy the element
    vysion_AddWindow(&window);
}

void vysion_FileExplorerMenuClickAction(struct optix_widget *widget) {
    struct vysion_file_explorer_menu *menu = (struct vysion_file_explorer_menu *) widget;
    int offset = (int) menu->current_selection - (int) vysion_current_context->filesystem_start - (int) sizeof(struct vysion_widget);
    //struct vysion_file_widget *file_widget = (struct vysion_file_widget *) menu->folder->contents[menu->menu.selection];
    //so if it was a folder and was clicked, it should move to the next level if applicable
    switch (menu->current_selection_vysion_type) {
        case VYSION_BASIC_TYPE:
        case VYSION_PROTECTED_BASIC_TYPE:
        case VYSION_ASM_TYPE:
        case VYSION_C_TYPE:
        case VYSION_ICE_TYPE:
            vysion_RunProgram(menu->current_selection_name);
            break;
        case VYSION_APPVAR_TYPE:
        case VYSION_ICE_SOURCE_TYPE:
            break;
        case VYSION_FOLDER_TYPE:
            menu->pos = menu->current_selection;
            //in this case, it will be a special folder
            dbg_sprintf(dbgout, "Offset: %d\n", offset);
            if (offset / (sizeof(struct vysion_widget) + sizeof(uint8_t)) < NUM_SPECIAL_FOLDERS) {
                menu->special_folder = true;
                menu->special_folder_index = offset / (sizeof(struct vysion_widget) + sizeof(uint8_t));
                menu->menu.num_options = menu->special_folder_index == FOLDER_PROGRAMS ? vysion_current_context->num_programs : vysion_current_context->num_appvars;
                dbg_sprintf(dbgout, "Num files: %d\n", menu->menu.num_options);
            } else {
                menu->special_folder = false;
                menu->menu.num_options = vysion_GetNumFilesInDirectory(menu->pos);
            }
            menu->menu.min = menu->menu.selection = 0;
            menu->menu.widget.state.needs_redraw = true;
            break;
        default:
            break;

    }
}

//it seems most logical for this function to both render and update-the standard update callback will
//simply handle updating the current selection, scrolling, and stuff
void vysion_RenderAndUpdateFileExplorerMenu(struct optix_widget *widget) {
    struct vysion_file_explorer_menu *menu = (struct vysion_file_explorer_menu *) widget;
    uint8_t type;
    void *search_pos = NULL;
    char *var_name;
    int i = 0;
    if (!(widget->state.needs_redraw || (menu->menu.selection != menu->menu.last_selection && widget->state.selected) || menu->menu.needs_partial_redraw)) return;
    if (menu->special_folder) {
        while ((var_name = ti_DetectAny(&search_pos, NULL, &type))) {
            if (*var_name == '#' || *var_name == '!') continue;
            if ((menu->special_folder_index == FOLDER_PROGRAMS && (type == TI_PPRGM_TYPE || type == TI_PRGM_TYPE)) || (menu->special_folder_index == FOLDER_APPVARS && type == TI_APPVAR_TYPE)) {
                struct vysion_file_info file_info;
                dbg_sprintf(dbgout, "Special folder: %d Name: %s Type: %d\n", menu->special_folder, var_name, type);
                if (i >= menu->menu.min) {
                    vysion_GetFileInfo(&file_info, var_name, type);
                    optix_RenderMenuOption(i, menu, var_name, file_info.icon);
                    //so that we can use it later
                    if (i == menu->menu.selection) {
                        strcpy(menu->current_selection_name, var_name);
                        menu->current_selection_vysion_type = file_info.vysion_type;
                    }
                }
                //this will only be run if we didn't return earlier, which will happen if the selection is different or we need a redraw of some sort
                i++;
                if (i >= menu->menu.num_options || i >= (menu->menu.min + menu->menu.rows * menu->menu.columns)) break;
            }
        }
        //make sure everything that needs to be redrawn gets redrawn
        //this is for the full redraw case-we could leave some graphical artifacts otherwise
        while (i < menu->menu.min + menu->menu.rows * menu->menu.columns) optix_RenderMenuOption(i++, menu, NULL, NULL);
    } else {
        //struct vysion_widget *min = vysion_GetDirectoryEntryByIndex(menu->pos, menu->menu.min);
        struct vysion_widget *ptr = menu->pos + 1;
        for (int i = menu->menu.min; i < menu->menu.min + menu->menu.rows * menu->menu.columns; i++) {
            if (i == menu->menu.selection) {
                dbg_sprintf(dbgout, "Current name: %s Current type: %d\n", ptr->name, ptr->type);
                menu->current_selection = ptr;
                menu->current_selection_vysion_type = ptr->type;
            }
            dbg_sprintf(dbgout, "Type: %d\n", ptr->type);
            if (i < menu->menu.num_options) {
                if (ptr->type == VYSION_FOLDER_TYPE)
                    optix_RenderMenuOption(i, menu, ptr->name, icon_folder);
                else {
                    struct vysion_file_info file_info;
                    vysion_GetFileInfo(&file_info, ptr->name, vysion_VysionTypeToTIType(ptr->type));
                    optix_RenderMenuOption(i, menu, ptr->name, file_info.icon);
                }
            } else {
                //the "No files..." message
                if (!menu->menu.num_options && i == menu->menu.min) {
                    //because OPTIX 3 is smart enough to not render if there's no options
                    //so we need to trick it
                    menu->menu.num_options = 1;
                    optix_RenderMenuOption(i, menu, "No files...", settings_shell);
                    menu->menu.num_options = 0;
                } else optix_RenderMenuOption(i, menu, NULL, NULL);
            }
            ptr = vysion_GetNextFile(ptr);
        }
    }
    current_context->data->needs_blit = true;
    menu->menu.needs_partial_redraw = false; 
    menu->menu.widget.state.needs_redraw = false;
}
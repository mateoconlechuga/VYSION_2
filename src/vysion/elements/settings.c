#include "settings.h"
#include <graphx.h>
#include "../window_manager.h"
#include "../../optix/init.h"
#include "../gfx/output/vysion_gfx.h"
#include "../control.h"

#include "../../optix/elements/text.h"

gfx_sprite_t *settings_menu_spr[] = {settings_system, settings_shell, settings_colors, settings_taskbar, settings_reset, settings_desktop, NULL};
char *settings_menu_text[] = {"System", "Shell", "Colors", "Taskbar", "Reset", "Desktop", NULL};


void vysion_AddSettingsWindow(void *config) {
    //settings configurations and structs (at the top for simplicity)
    struct vysion_settings_window window = {
        .window = {.widget = {.type = WINDOW_SETTINGS}},
    };
    struct optix_menu template_sidebar_menu = {
        .widget = {
            .transform = {
                .width = 20,
                .height = 100,
            },
            .child = NULL,
        },
        .text = NULL,
        .spr = settings_menu_spr,
        .sprite_args = {
            .widget = {
                .centering = {
                    .y_centering = OPTIX_CENTERING_CENTERED,
                    .x_centering = OPTIX_CENTERING_LEFT,
                    .x_offset = 2,
                },
            },
        },
        .rows = 5,
        .columns = 1,
        .click_action = {
            .click_action = vysion_SettingsMenuSideBarMenuClickAction,
        },
        //we do know that this will be the first element, so this should work?
        //arr == &arr[0], if I remember correctly
        .pass_self = true,
    };
    optix_InitializeWidget(&template_sidebar_menu.widget, OPTIX_MENU_TYPE);
    struct optix_widget *template_children[SETTINGS_WINDOW_NUM_CHILDREN + 1];
    struct optix_window template = {
        .widget = {
            .transform = {
                .x = 50,
                .y = 50,
                .width = 200,
                .height = 100,
            },
            .child = template_children,
            //.child = NULL,
        },
    };
    optix_InitializeWidget(&template.widget, OPTIX_WINDOW_TYPE);
    //and now put all of those children in
    struct optix_menu template_settings_menu[SETTINGS_NUM_MENUS];
    for (int i = 0; i < SETTINGS_WINDOW_NUM_CHILDREN; i++) {
        if (i < SETTINGS_MENUS_STACK_OFFSET) {
            switch (i) {
                case 0:
                    template_children[i] = &template_sidebar_menu.widget;
                    break;
                default:
                    break;
            }
        } else {
            //system
            static const char *system[] = {"Set time", NULL};
            static const struct optix_text system_set_time = {
                .widget = {
                    .centering = {
                        .x_centering = OPTIX_CENTERING_RIGHT,
                        .x_offset = -4,
                        .y_centering = OPTIX_CENTERING_CENTERED,
                    },
                },
                .text = "TEST",
            };
            optix_InitializeTextTransform(&system_set_time);
            optix_InitializeWidget(&system_set_time.widget, OPTIX_TEXT_TYPE);
            struct optix_widget *system_child[] = {&system_set_time.widget, NULL, NULL, NULL, NULL};
            //shell
            static const char *shell[] = {"Use cursor", NULL};
            //colors
            static const char *colors[] = {"Customize colors", NULL};
            //taskbar
            static const char *taskbar[] = {"Height", "Transparent", "Display names", NULL};
            //reset
            static const char *reset[] = {"Reset settings", "Reset filesystem", "Reset all", "Report issue", "Please work", "Uninstall", NULL};
            //desktop
            static const char *desktop[] = {"Show folders", "Show files", "Wallpaper", NULL};
            struct optix_menu *menu = (struct optix_menu *) &template_settings_menu[i - SETTINGS_MENUS_STACK_OFFSET];
            template_children[i] = menu;
            menu->widget.transform.width = 180;
            menu->widget.transform.height = 100;
            menu->rows = 5;
            menu->columns = 1;
            menu->spr = NULL;
            menu->widget.centering.x_centering = OPTIX_CENTERING_RIGHT;
            menu->text_args.widget.centering.x_centering = OPTIX_CENTERING_LEFT;
            menu->text_args.widget.centering.x_offset = 4;
            menu->text_args.widget.centering.y_centering = OPTIX_CENTERING_CENTERED;
            //initialize this
            menu->widget.child = NULL;
            //this would probably be better as just an array instead of these horrible switch statements
            switch (i - SETTINGS_MENUS_STACK_OFFSET) {
                case 0:
                    menu->text = system;
                    menu->widget.child = system_child;
                    break;
                case 1:
                    menu->text = shell;
                    break;
                case 2:
                    menu->text = colors;
                    break;
                case 3:
                    menu->text = taskbar;
                    break;
                case 4:
                    menu->text = reset;
                    break;
                case 5:
                    menu->text = desktop;
                    break;
                default:
                    break;
            }
            dbg_sprintf(dbgout, "Initializing child %d\n", i);
            optix_InitializeWidget((struct optix_widget *) menu, OPTIX_MENU_TYPE);
            //start with just the first one visible
            template_children[i]->state.visible = (i == SETTINGS_MENUS_STACK_OFFSET);
        }
    }
    //get the last one
    template_children[SETTINGS_WINDOW_NUM_CHILDREN] = NULL;
    //optix_RecursiveAlign(&template.widget);
    //and now for the window title bar
    struct optix_window_title_bar template_title_bar = {
        .widget = {.child = NULL},
        .window = &template,
    };
    optix_InitializeWidget(&template_title_bar.widget, OPTIX_WINDOW_TITLE_BAR_TYPE);
    memcpy(&window.window.widget.window_title_bar, &template_title_bar, sizeof(struct optix_window_title_bar));
    dbg_sprintf(dbgout, "Adding window...\n");
    vysion_AddWindow(&window);
    dbg_sprintf(dbgout, "Window added.\n");
}

void vysion_SettingsMenuSideBarMenuClickAction(void *args) {
    struct optix_widget **stack = (struct optix_widget **) args;
    int selection = ((struct optix_menu *) stack[SETTINGS_SIDEBAR_MENU_OFFSET])->selection;
    dbg_sprintf(dbgout, "Selection was %d\n", selection);
    for (int i = SETTINGS_MENUS_STACK_OFFSET; i < SETTINGS_MENUS_STACK_OFFSET + SETTINGS_NUM_MENUS; i++)
        stack[i]->state.visible = false;
    //set the selected one to visible
    stack[selection + SETTINGS_SIDEBAR_MENU_OFFSET]->state.visible = true;
    //it'll need to be redrawn now as well
    stack[selection + SETTINGS_SIDEBAR_MENU_OFFSET]->state.needs_redraw = true;
}
#include "window_manager.h"
#include "control.h"
#include "optix/gui_control.h"
#include "optix/init.h"
#include "optix/util.h"
#include <debug.h>
//window structs
#include "elements/file_explorer.h"
#include "elements/desktop.h"

//actual code
void vysion_AddWindow(struct vysion_window_widget *widget) {
    int index = 0;
    struct optix_widget *(*stack)[] = current_context->stack;
    while ((*stack)[index]) index++;
    //so we have the index now
    //so just copy it
    //this is cool
    //make the stack one bigger
    //vysion_current_context->stack = realloc(vysion_current_context->stack, sizeof(struct optix_widget *) * (index + 2));
    (*stack)[index + 1] = NULL;
    (*stack)[index] = malloc(vysion_GetWindowSize(widget->type));
    //let's do this here
    dbg_sprintf(dbgout, "Formatting title bar...\n");
    vysion_FormatWindowTitleBar(widget->window_title_bar);
    //align everything
    dbg_sprintf(dbgout, "Aligning title bar...\n");
    optix_RecursiveAlign((struct optix_widget *) widget->window_title_bar);
    dbg_sprintf(dbgout, "Copying elements...\n");
    optix_CopyElement(&(*stack)[index], widget->window_title_bar);
    //now switch the selection to it if in box-based mode
    if (!current_context->settings->cursor_active) {
        optix_SetCurrentSelection((*stack)[index]);
        (*stack)[index]->state.selected = true;
        current_context->cursor->direction = OPTIX_CURSOR_FORCE_UPDATE;
    }
    dbg_sprintf(dbgout, "Finished adding windows.\n");
}

size_t vysion_GetWindowSize(uint8_t type) {
    switch (type) {
        case WINDOW_FILE_EXPLORER:
            return sizeof(struct vysion_file_explorer_window);
            break;
        default:
            return 0;
            break;
    }
}

//generic function, just adds a window title and minimize/close buttons to it
void vysion_FormatWindowTitleBar(struct optix_window_title_bar *window_title_bar) {
    char *name[] = {WINDOW_FILE_EXPLORER_NAME};
    char *str = name[((struct vysion_window_widget *) window_title_bar)->type];
    //text part
    struct optix_text template_text = {
        .text = str,
        .alignment = OPTIX_CENTERING_CENTERED,
        .x_offset = 0,
        .min = 0,
        .needs_offset_update = true,
        .background_rectangle = false,
    };
    optix_InitializeTextTransform(&template_text);
    optix_InitializeWidget(&template_text.widget, OPTIX_TEXT_TYPE);
    template_text.background_rectangle = false;
    template_text.widget.centering.x_centering = OPTIX_CENTERING_LEFT;
    template_text.widget.centering.x_offset = 4;
    template_text.widget.centering.y_centering = OPTIX_CENTERING_CENTERED;
    struct optix_menu template_menu = {
        .widget = {
            .transform = {
                .width = 36,
                .height = 12,
            },
        },
        .text_centering = {.y_centering = OPTIX_CENTERING_CENTERED, .x_centering = OPTIX_CENTERING_CENTERED},
        .rows = 1,
        .columns = 3,
        .text = (char *[]) {WINDOW_MINIMIZE_STRING, WINDOW_MAXIMIZE_STRING, WINDOW_CLOSE_STRING, NULL},
        .spr = NULL,
    };
    optix_InitializeWidget(&template_menu.widget, OPTIX_MENU_TYPE);
    template_menu.widget.centering.x_centering = OPTIX_CENTERING_RIGHT;
    template_menu.widget.centering.y_centering = OPTIX_CENTERING_CENTERED;
    struct optix_widget *child[] = {&template_text.widget, &template_menu.widget};
    //so we need an array now
    window_title_bar->widget.child = malloc((WINDOW_TITLE_BAR_ELEMENTS + 1) * sizeof(struct optix_widget *));
    for (int i = 0; i < WINDOW_TITLE_BAR_ELEMENTS; i++) optix_CopyElement(&window_title_bar->widget.child[i], child[i]);
    window_title_bar->widget.child[WINDOW_TITLE_BAR_ELEMENTS] = NULL;
}

void vysion_CloseAllWindows(struct optix_context *context) {
    int i = DESKTOP_ELEMENTS;
    while (*(context->stack) && (*(context->stack))[i]) {
        optix_FreeElement(&((*(cont ext->stack))[i]));
        i++;
    }
}
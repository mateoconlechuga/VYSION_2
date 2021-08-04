#include "window_manager.h"
#include "control.h"
#include "optix/gui_control.h"
#include "optix/init.h"
#include "optix/util.h"
#include <debug.h>
#include <tice.h>
#include <graphx.h>
//window structs
#include "elements/file_explorer.h"
#include "elements/desktop.h"


char *window_title_bar_menu_text[WINDOW_TITLE_BAR_MENU_OPTIONS + 1] = {WINDOW_MINIMIZE_STRING, WINDOW_MAXIMIZE_STRING, WINDOW_CLOSE_STRING, NULL};
char *window_name[] = {WINDOW_FILE_EXPLORER_NAME};
gfx_sprite_t *window_icon[] = {WINDOW_FILE_EXPLORER_ICON};

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
    vysion_FormatWindowTitleBar(widget->window_title_bar);
    //align everything
    optix_RecursiveAlign((struct optix_widget *) widget->window_title_bar);
    optix_CopyElement(&((*stack)[index]), widget->window_title_bar);
    //now switch the selection to it if in box-based mode
    if (!current_context->settings->cursor_active) {
        optix_SetCurrentSelection((*stack)[index]);
        (*stack)[index]->state.selected = true;
        current_context->cursor->direction = OPTIX_CURSOR_FORCE_UPDATE;
    }
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
    char *str = window_name[((struct vysion_window_widget *) window_title_bar)->type];
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
        .text = window_title_bar_menu_text,
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
        optix_FreeElement(&((*(context->stack))[i]));
        i++;
    }
}

void vysion_WindowManagerMenuClickAction(void *args) {
    //this will be the selection
    int selection = ((struct optix_menu *) args)->selection;
    int num_elements = optix_GetNumElementsInStack(*(current_context->stack));
    dbg_sprintf(dbgout, "Running the function...%d\n", selection);
    //so essentially what we want to do is if the window is not selected move it to the top
    //if it is selected it should be minimized
    //if it is minimized it should be moved to the top
    int index = 0;
    int i = 0;
    struct optix_widget **stack = *(current_context->stack);
    for (i = 0; i < num_elements; i++) {
        //desktop elements don't count
        int num_children = optix_GetNumElementsInStack(stack[i]->child);
        //one byte for null, then the second one will be the magic number
        if (stack[i]->child[num_children + 1] != DESKTOP_ELEMENT_MAGIC_SIGNIFIER && (stack[i]->type == OPTIX_WINDOW_TITLE_BAR_TYPE || stack[i]->type == OPTIX_WINDOW_TYPE)) {
            if (index == selection) break;
            index++;
        }
    }
    struct optix_window_title_bar *window_title_bar = (struct optix_window_title_bar *) stack[i];
    struct optix_window *window = (struct optix_window *) window_title_bar;
    dbg_sprintf(dbgout, "%d num: %d\n", i, num_elements);
    struct optix_widget *widget = (struct optix_widget *) window_title_bar;
    //or it's the last selected window
    if (widget->state.visible && (widget->state.selected || i == num_elements - 1)) {
        dbg_sprintf(dbgout, "1\n");
        //minimize it
        window_title_bar->window->widget.state.visible = false;
        optix_IntelligentRecursiveSetNeedsRedraw((*(current_context->stack)), widget);
    } else {
        dbg_sprintf(dbgout, "2 %d\n", widget->type);
        window_title_bar->window->needs_focus = true;
        widget->state.needs_redraw = true;
        widget->state.selected = true;
        window_title_bar->window->widget.state.visible = true;
        dbg_sprintf(dbgout, "Made it here.\n");
        if (!current_context->settings->cursor_active) {
            optix_SetCurrentSelection(widget);
            current_context->cursor->direction = OPTIX_CURSOR_FORCE_UPDATE;
        }
    }

}

//the graphical window manager stuff
void vysion_UpdateWindowManagerMenu(struct optix_widget *widget) {
    struct vysion_window_manager_menu *window_manager_menu = (struct vysion_window_manager_menu *) widget;
    struct optix_menu *menu = (struct optix_menu *) widget;
    int num_windows = optix_GetNumElementsInStack(*(current_context->stack)) - DESKTOP_ELEMENTS;
    bool selected_save = widget->state.selected;
    if (num_windows != window_manager_menu->last_num_windows || window_manager_menu->needs_update) {
        //we need to update it
        window_manager_menu->text_save = realloc(window_manager_menu->text_save, num_windows + 1);
        window_manager_menu->text_save[num_windows] = NULL;
        menu->text = widget->state.selected ? window_manager_menu->text_save : NULL;
        menu->spr = realloc(menu->spr, num_windows + 1);
        menu->spr[num_windows] = NULL;
        //now we populate the text
        for (int i = 0; i < num_windows; i++) {
            window_manager_menu->text_save[i] = window_name[((struct vysion_window_widget *) (*(current_context->stack))[i + DESKTOP_ELEMENTS])->type];
            menu->spr[i] = window_icon[((struct vysion_window_widget *) (*(current_context->stack))[i + DESKTOP_ELEMENTS])->type];
        }
        //update the transform
        menu->num_options = num_windows;
        //we'll set the x position to be just above the taskbar
        widget->transform.height = num_windows * WINDOW_MANAGER_MENU_ROW_HEIGHT;
        optix_AlignTransformToTransform(widget, NULL);
        menu->rows = num_windows;
        widget->state.needs_redraw = true;
        widget->state.selectable = num_windows > 0;
        optix_IntelligentRecursiveSetNeedsRedraw(*(current_context->stack), widget);
        window_manager_menu->needs_update = false;
        window_manager_menu->last_num_windows = num_windows;
    }
    //we need to do this too
    optix_UpdateMenu_default(widget);
    //in this part, we'll handle the expanding/collapsing of the menu when the menu is selected/unselected
    if (widget->state.selected != selected_save) {
        widget->state.needs_redraw = true;
        if (widget->state.selected) {
            //we'll expand the menu
            menu->text = window_manager_menu->text_save;
            menu->transparent_background = false;
            menu->hide_selection_box = false;
            widget->transform.width = WINDOW_MANANGER_MENU_WIDTH;
            widget->transform.x = 1;
        } else {
            //collapse it
            menu->text = NULL;
            menu->transparent_background = true;
            menu->hide_selection_box = true;
            optix_IntelligentRecursiveSetNeedsRedraw(*(current_context->stack), widget);
            widget->transform.width = WINDOW_MANAGER_MENU_ROW_HEIGHT;
            widget->transform.x = 0;
        }
    }
}

void vysion_RenderWindowManagerMenu(struct optix_widget *widget) {
    //essentially what I want this to do is turn into a window when it is selected (otherwise it will be transparent)
    if (widget->state.selected && widget->state.needs_redraw) {
        //draw the background for it (the update function should handle everything else, setting the width and so on)
        optix_OutlinedRectangle_WithBevel(widget->transform.x - 1, widget->transform.y - 1, widget->transform.width + 2, widget->transform.height + 2, 
            WINDOW_BG_COLOR_INDEX, WINDOW_BORDER_BEVEL_LIGHT_INDEX, WINDOW_BORDER_BEVEL_DARK_INDEX);
    }
    optix_RenderMenu_default(widget);
}
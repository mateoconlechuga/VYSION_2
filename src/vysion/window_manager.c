#include "window_manager.h"
#include "control.h"
#include "../optix/gui_control.h"
#include "../optix/init.h"
#include "../optix/util.h"
#include "../optix/cursor.h"
#include <debug.h>
#include <tice.h>
#include <graphx.h>
//window structs
#include "elements/file_explorer.h"
#include "elements/settings.h"
#include "elements/desktop.h"


char *window_title_bar_menu_text[WINDOW_TITLE_BAR_MENU_OPTIONS + 1] = {WINDOW_MINIMIZE_STRING, WINDOW_MAXIMIZE_STRING, WINDOW_CLOSE_STRING, NULL};
char *window_name[] = {WINDOW_FILE_EXPLORER_NAME, WINDOW_SETTINGS_NAME};
gfx_sprite_t *window_icon[] = {WINDOW_FILE_EXPLORER_ICON, WINDOW_SETTINGS_ICON};
gfx_sprite_t *window_title_bar_action_icon[WINDOW_TITLE_BAR_MENU_OPTIONS + 1] = {window_minimize, window_maximize, window_close, NULL};

//actual code
void vysion_AddWindow(struct vysion_window_widget *widget) {
    int index = 0;
    int index_b = 0;
    struct optix_widget *(*stack)[] = current_context->stack;
    struct vysion_window_widget *new_window_ptr = NULL;
    while ((*stack)[index]) index++;
    //so we have the index now
    //so just copy it
    //this is cool
    //make the stack one bigger
    //vysion_current_context->stack = realloc(vysion_current_context->stack, sizeof(struct optix_widget *) * (index + 2));
    (*stack)[index + 1] = NULL;
    widget->window_title_bar.widget.state.override_size = true;
    widget->window_title_bar.widget.state.size = vysion_GetWindowSize(widget->type);
    //new_window_ptr->icon = widget->icon;
    //let's do this here
    //align everything
    optix_RecursiveAlign((struct optix_widget *) &widget->window_title_bar);
    optix_CopyElement(&(*stack)[index], &widget->window_title_bar);
    new_window_ptr = (*stack)[index];
    new_window_ptr->type = widget->type;
    vysion_FormatWindowTitleBar((*stack)[index]);
    //now switch the selection to it if in box-based mode
    if (!current_context->settings->cursor_active) {
        optix_SetCurrentSelection((*stack)[index]);
        (*stack)[index]->state.selected = true;
        current_context->cursor->direction = OPTIX_CURSOR_FORCE_UPDATE;
    }
    //also add it to the other struct because it'll break otherwise
    //get the next open slot
    while (vysion_current_context->window[index_b] && index_b < MAX_NUM_WINDOWS) index_b++;
    dbg_sprintf(dbgout, "Index: %d Type: %d Window type: %d\n", index_b, (*stack)[index]->type, widget->type);
    //so now we have the first null one
    vysion_current_context->window[index_b] = (*stack)[index];
    vysion_current_context->window[index_b + 1] = NULL;
    dbg_sprintf(dbgout, "This part finished.\n");
}

size_t vysion_GetWindowSize(uint8_t type) {
    switch (type) {
        case WINDOW_FILE_EXPLORER:
            return sizeof(struct vysion_file_explorer_window);
            break;
        case WINDOW_SETTINGS:
            return sizeof(struct vysion_settings_window);
            break;
        default:
            return 0;
            break;
    }
}

//generic function, just adds a window title and minimize/close buttons to it
void vysion_FormatWindowTitleBar(struct optix_window_title_bar *window_title_bar) {
    uint8_t type = ((struct vysion_window_widget *) window_title_bar)->type;
    char *str = window_name[type];
    dbg_sprintf(dbgout, "Other type: %d Type: %d String: %s\n", window_title_bar->widget.type, type, str);
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
    dbg_sprintf(dbgout, "What\n");
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
        .sprite_args = {
            .widget = {
                .centering = {
                    .x_centering = OPTIX_CENTERING_CENTERED,
                    .y_centering = OPTIX_CENTERING_CENTERED,
                },
            },
            .transparent = true,
            .transparent_color = myimages_palette_offset,
        },
        .rows = 1,
        .columns = 3,
        //.text = window_title_bar_menu_text,
        .spr = window_title_bar_action_icon,
        .transparent_background = true,
    };
    dbg_sprintf(dbgout, "Who knows what's going on\n");
    optix_InitializeWidget(&template_menu.widget, OPTIX_MENU_TYPE);
    template_menu.widget.centering.x_centering = OPTIX_CENTERING_RIGHT;
    template_menu.widget.centering.y_centering = OPTIX_CENTERING_CENTERED;
    struct optix_widget *child[] = {&template_text.widget, &template_menu.widget};
    //so we need an array now
    window_title_bar->widget.child = malloc((WINDOW_TITLE_BAR_ELEMENTS + 1) * sizeof(struct optix_widget *));
    for (int i = 0; i < WINDOW_TITLE_BAR_ELEMENTS; i++) optix_CopyElement(&window_title_bar->widget.child[i], child[i]);
    window_title_bar->widget.child[WINDOW_TITLE_BAR_ELEMENTS] = NULL;
    optix_RecursiveAlign((struct optix_widget *) window_title_bar);
}

void vysion_CloseWindow(struct vysion_window **window) {
    void **temp = window;
    int vysion_index = optix_GetElementInStackByAddress(vysion_current_context->window, *temp);
    int optix_index = optix_GetElementInStackByAddress(*current_context->stack, *temp);
    //so we have to clear the main window, take it out of the render stack, and clear the window title bar
    optix_FreeElement((struct optix_widget **) window);
    optix_RemoveElementInStack(*current_context->stack, optix_index);
    optix_RemoveElementInStack(vysion_current_context->window, vysion_index);
    dbg_sprintf(dbgout, "That was successful as well.\n");
    dbg_sprintf(dbgout, "Freeing was successful.\n");
    //free(*window);
    //*window = NULL;
}

void vysion_CloseAllWindows(struct optix_context *context) {
    dbg_sprintf(dbgout, "Getting num elements..");
    int num_windows = optix_GetNumElementsInStack(vysion_current_context->window);
    dbg_sprintf(dbgout, "%d found\n", num_windows);
    dbg_sprintf(dbgout, "Freeing everything...\n");
    for (int i = 0; i < num_windows; i++) {
        dbg_sprintf(dbgout, "i: %d, num_windows: %d\n", i, optix_GetNumElementsInStack(vysion_current_context->window));
        dbg_sprintf(dbgout, "Address: %d\n", vysion_current_context->window[0]);
        vysion_CloseWindow(&(vysion_current_context->window[0]));
    }
}

void vysion_MoveWindowToTop(struct vysion_window **window) {
    int optix_index = optix_GetElementInStackByAddress(current_context->stack, *window);
    int num_elements = optix_GetNumElementsInStack(current_context->stack);
    //first remove it from its current position
    optix_RemoveElementInStack(current_context->stack, optix_index);
    //then put it at the top
    (*current_context->stack)[num_elements - 1] = *window;
    (*current_context->stack)[num_elements] = NULL;
}

void vysion_WindowManagerMenuClickAction(void *args) {
    //this will be the selection
    int selection = ((struct optix_menu *) args)->selection;
    int num_elements = optix_GetNumElementsInStack(*(current_context->stack));
    dbg_sprintf(dbgout, "Running the function...%d\n", selection);
    //so essentially what we want to do is if the window is not selected move it to the top
    //if it is selected it should be minimized
    //if it is minimized it should be moved to the top
    int i = 0;
    struct optix_widget **stack = *(current_context->stack);
    i = optix_GetElementInStackByAddress(stack, vysion_current_context->window[selection]);
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
        //widget->state.selected = true;
        window_title_bar->window->widget.state.visible = true;
        dbg_sprintf(dbgout, "Made it here.\n");
        /*if (!current_context->settings->cursor_active) {
            optix_SetCurrentSelection(widget);
            current_context->cursor->direction = OPTIX_CURSOR_FORCE_UPDATE;
        }*/
    }

}

//the graphical window manager stuff
void vysion_UpdateWindowManagerMenu(struct optix_widget *widget) {
    struct vysion_window_manager_menu *window_manager_menu = (struct vysion_window_manager_menu *) widget;
    struct optix_menu *menu = (struct optix_menu *) widget;
    int num_windows = optix_GetNumElementsInStack(*(current_context->stack)) - DESKTOP_ELEMENTS;
    bool selected_save = widget->state.selected;
    if (num_windows != window_manager_menu->last_num_windows || window_manager_menu->needs_update) {
        int i = 0;
        struct optix_widget **stack = vysion_current_context->window;
        //we need to update it
        menu->text[num_windows] = NULL;
        menu->spr[num_windows] = NULL;
        while (stack[i]) {
            dbg_sprintf(dbgout, "%d\n", ((struct vysion_window_widget *) stack[i])->type);
            menu->text[i] = window_name[((struct vysion_window_widget *) stack[i])->type];
            menu->spr[i] = window_icon[((struct vysion_window_widget *) stack[i])->type];
            i++;
        }
        //update the transform
        menu->num_options = num_windows;
        //we'll set the x position to be just above the taskbar
        widget->transform.width = num_windows * WINDOW_MANANGER_MENU_WIDTH;
        optix_AlignTransformToTransform(widget, NULL);
        menu->columns = num_windows;
        widget->state.needs_redraw = true;
        widget->state.selectable = num_windows > 0;
        optix_IntelligentRecursiveSetNeedsRedraw(*(current_context->stack), widget);
        window_manager_menu->needs_update = false;
        window_manager_menu->last_num_windows = num_windows;
    }
    //we need to do this too
    optix_UpdateMenu_default(widget);
    //in this part, we'll handle the expanding/collapsing of the menu when the menu is selected/unselected
    /*if (widget->state.selected != selected_save) {
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
    }*/
}

void vysion_RenderWindowManagerMenu(struct optix_widget *widget) {
    //essentially what I want this to do is turn into a window when it is selected (otherwise it will be transparent)
    if (widget->state.selected && widget->state.needs_redraw) {
        //draw the background for it (the update function should handle everything else, setting the width and so on)
        /*optix_OutlinedRectangle_WithBevel(widget->transform.x - 1, widget->transform.y - 1, widget->transform.width + 2, widget->transform.height + 2, 
            WINDOW_BG_COLOR_INDEX, WINDOW_BORDER_BEVEL_LIGHT_INDEX, WINDOW_BORDER_BEVEL_DARK_INDEX);*/
    }
    optix_RenderMenu_default(widget);
}
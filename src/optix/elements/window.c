#include "window.h"

#include <debug.h>
#include <stdint.h>
#include <graphx.h>
#include <tice.h>
#include "../gui_control.h"
#include "../shapes.h"
#include "../loop.h"
#include "../util.h"
#include "../cursor.h"
#include "../colors.h"
#include "../input.h"
#include "menu.h"
#include "text.h"
#include "button.h"

void optix_UpdateWindow_default(struct optix_widget *widget) {
    struct optix_window *window = (struct optix_window *) widget;
    //quick notes here: I believe that the window should be inactive if a) it's not the topmost window or b) if there's a click outside of the window
    //that's how it works in most OSes, as far as I can tell
    if (window->active) {
        if (widget->child) optix_UpdateStack(widget->child);
    } else if (widget->state.selected) {
        dbg_sprintf(dbgout, "Window was selected.\n");
        //we know that if this happens, the cursor will be over the window
        if (optix_DefaultKeyIsDown(KEY_ENTER) & KEY_PRESSED) {
            int i = 0;
            optix_MoveWidgetToTop(widget);
            window->active = true;
            //this is true, probably
            widget->state.needs_redraw = true;
            //we could save the last selection here too
            if (!current_context->settings->cursor_active) {
                if (widget->child) {
                    while (widget->child[i]) {
                        if (widget->child[i]->state.selectable) {
                            optix_SetCurrentSelection(widget->child[i]);
                            break;
                        }
                        i++;
                    }
                }
            }
        }
    }
}

void optix_RenderWindow_default(struct optix_widget *widget) {
    //struct optix_window *window = (struct optix_window *) widget;
    //int title_bar_side_padding = 2;
    //int element_size = 10;
    if (widget->state.visible) {
        if (widget->state.needs_redraw) {
            optix_OutlinedRectangle_WithBevel(widget->transform.x - 1, widget->transform.y - 1, widget->transform.width + 2, widget->transform.height + 2, 
            WINDOW_BG_COLOR_INDEX, WINDOW_BORDER_BEVEL_LIGHT_INDEX, WINDOW_BORDER_BEVEL_DARK_INDEX);
        }
        //render everything in it
        if (widget->child) optix_RenderStack(widget->child);
    }
}

void optix_UpdateWindowTitleBar_default(struct optix_widget *widget) {
    struct optix_window_title_bar *window_title_bar = (struct optix_window_title_bar *) widget;
    struct optix_window *window = window_title_bar->window;
    //redraw it if the active status changes
    if (widget->state.needs_redraw) window->widget.state.needs_redraw = true;
    if (window->active || widget->state.selected) {
        int x_change = 0;
        int y_change = 0;
        int new_x_pos = 0;
        int new_y_pos = 0;
        switch (optix_DefaultKeyIsDown(KEY_ENTER)) {
            case KEY_PRESSED:
                if (!window->active) {
                    int i = 0;
                    optix_MoveWidgetToTop(widget);
                    window->active = true;
                    widget->state.needs_redraw = window->widget.state.needs_redraw = true;
                    //we could save the last selection here too
                    if (!current_context->settings->cursor_active) {
                        if (window->widget.child) {
                            while (window->widget.child[i]) {
                                if (window->widget.child[i]->state.selectable) {
                                    optix_SetCurrentSelection(window->widget.child[i]);
                                    break;
                                }
                                i++;
                            }
                        }
                    }
                } else {
                    //iniitalize the drag location
                    window_title_bar->drag_active = true;
                    window_title_bar->drag_start_x = current_context->cursor->widget.transform.x;
                    window_title_bar->drag_start_y = current_context->cursor->widget.transform.y;
                }
                break;
            case KEY_RELEASED:
                if (window_title_bar->drag_active) {
                    x_change = current_context->cursor->widget.transform.x - window_title_bar->drag_start_x;
                    y_change = current_context->cursor->widget.transform.y - window_title_bar->drag_start_y;
                    //this means that we moved the window somewhere new
                    if (x_change || y_change) {
                        new_x_pos = window_title_bar->window->widget.transform.x + x_change;
                        new_y_pos = window_title_bar->window->widget.transform.y + y_change;
                        if (new_x_pos < 0) new_x_pos = 1;
                        if (new_y_pos < window_title_bar->widget.transform.height) new_y_pos = window_title_bar->widget.transform.height;
                        if (new_x_pos > LCD_WIDTH - window_title_bar->window->widget.transform.width - 1)
                            new_x_pos = LCD_WIDTH - window_title_bar->window->widget.transform.width - 1;
                        if (new_y_pos > LCD_HEIGHT - (window_title_bar->window->widget.transform.height + 1))
                            new_y_pos = LCD_HEIGHT - (window_title_bar->window->widget.transform.height + 1);
                        optix_SetPosition(&window_title_bar->window->widget, new_x_pos, new_y_pos);
                        optix_RefreshWindowTitleBarTransform(window_title_bar);
                        current_context->data->gui_needs_full_redraw = true;
                    }
                    window_title_bar->drag_active = false;
                }
                break;
            default:
                break;
        }
        if (widget->child) optix_UpdateStack(widget->child);
        dbg_sprintf(dbgout, "Finished updating children.\n");
    } else window_title_bar->drag_active = false;
    //the window will only update itself if it's active anyway so who cares
    //there is a possibility that these could be null
    if (window && window_title_bar) {
        if (window->widget.update) window->widget.update((struct optix_widget *) window);
        if (window->widget.state.needs_redraw) widget->state.needs_redraw = true;
        if (!window_title_bar->active_save && window->active) optix_MoveWidgetToTop(widget);
        window_title_bar->active_save = window->active;
    }
}

void optix_RenderWindowTitleBar_default(struct optix_widget *widget) {
    struct optix_window_title_bar *window_title_bar = (struct optix_window_title_bar *) widget;
    struct optix_window *window = window_title_bar->window;
    //those settextcolors are in there in case there's a title, which should change color on select
    if (widget->state.visible) {
        if (window_title_bar->window) {
            window_title_bar->window->widget.render((struct optix_widget *) window_title_bar->window);
            window_title_bar->window->widget.state.needs_redraw = false;
        }
        if (widget->state.needs_redraw) {
            if (window_title_bar->window->active) {
                optix_OutlinedRectangle_WithBevel(widget->transform.x - 1, widget->transform.y, widget->transform.width + 2, widget->transform.height, //transform
                WINDOW_TITLE_BAR_COLOR_SELECTED_INDEX, WINDOW_BORDER_BEVEL_LIGHT_INDEX, WINDOW_BORDER_BEVEL_DARK_INDEX);                               //color
                optix_SetTextColor(WINDOW_TITLE_TEXT_FG_COLOR_SELECTED_INDEX, WINDOW_TITLE_TEXT_BG_COLOR_SELECTED_INDEX);
            } else {
                optix_OutlinedRectangle_WithBevel(widget->transform.x - 1, widget->transform.y, widget->transform.width + 2, widget->transform.height, //transform
                WINDOW_TITLE_BAR_COLOR_UNSELECTED_INDEX, WINDOW_BORDER_BEVEL_LIGHT_INDEX, WINDOW_BORDER_BEVEL_DARK_INDEX);                             //color
                optix_SetTextColor(WINDOW_TITLE_TEXT_FG_COLOR_UNSELECTED_INDEX, WINDOW_TITLE_TEXT_BG_COLOR_UNSELECTED_INDEX);
            }
        }
        if (widget->child) optix_RenderStack(widget->child);
        optix_SetTextColor(TEXT_FG_COLOR_INDEX, TEXT_BG_COLOR_INDEX);
    }
}

//utilities
/*About:
    - resizes a window to a new size
    - this will probably need to be rewritten later, because the first iteration is probably going to be awful
    - at the moment, only menus will be able to have dynamic sizes, and besides that, it'll just adjust the sizes if they're larger than the window

*/
void optix_ResizeWindow(struct optix_widget *widget, uint16_t width, uint8_t height) {
    struct optix_window *window = (struct optix_window *) widget;
    int i = 0;
    if (window->resize_info.resizable) {
        if (width < window->resize_info.min_width) width = window->resize_info.min_width;
        if (height < window->resize_info.min_height) height = window->resize_info.min_height;
        widget->transform.width = width;
        widget->transform.height = height;
        if (widget->child) {
            while (widget->child[i]) {
                struct optix_widget *child = widget->child[i];
                //resize it to a new sizef
                if (child->type == OPTIX_MENU_TYPE) {
                    struct optix_menu *menu = (struct optix_menu *) child;
                    if (!menu->resize_info.x_lock) {
                        menu->columns = width / menu->resize_info.min_width;
                        menu->widget.transform.width = width;
                    }
                    if (!menu->resize_info.y_lock) {
                        menu->rows = height / menu->resize_info.min_height;
                        menu->widget.transform.height = height;
                    }
                } else {
                    //tf is this
                    child->transform.width = width;
                    child->transform.height = height;
                }
                i++;
            }
        }
        optix_RecursiveAlign(widget);
        widget->state.needs_redraw = true;
    }
}


//just refreshes the transform of a window title bar, to be consistent with its window's width
void optix_RefreshWindowTitleBarTransform(struct optix_window_title_bar *window_title_bar) {
    struct optix_transform *transform = &window_title_bar->widget.transform;
    transform->x =      window_title_bar->window->widget.transform.x;
    transform->y =      window_title_bar->window->widget.transform.y - TITLE_BAR_HEIGHT;
    transform->width =  window_title_bar->window->widget.transform.width;
    transform->height = TITLE_BAR_HEIGHT;
    optix_RecursiveAlign(&window_title_bar->widget);
}

void optix_MoveWidgetToTop(struct optix_widget *widget) {
    int optix_index = optix_GetElementInStackByAddress(current_context->stack, widget);
    int num_elements = optix_GetNumElementsInStack(current_context->stack);
    uint8_t last_element_type = current_context->stack[num_elements - 1]->type;
    //if it wasn't found just return
    if (optix_index == -1) return;
    //take care of the condition in which the topmost element could be a window or window title bar
    if (current_context->stack[num_elements - 1] != widget) {
        if (last_element_type == OPTIX_WINDOW_TYPE || last_element_type == OPTIX_WINDOW_TITLE_BAR_TYPE) {
            struct optix_widget *last_element = current_context->stack[num_elements - 1];
            struct optix_window *window = last_element->type == OPTIX_WINDOW_TYPE ? (struct optix_window *) last_element : ((struct optix_window_title_bar *) last_element)->window;
            window->active = false;
            window->widget.state.needs_redraw = true;
            //and this as well
            current_context->stack[num_elements - 1]->state.needs_redraw = true;
        }
    }
    //first remove it from its current position
    optix_RemoveElementInStack(current_context->stack, optix_index, num_elements);
    //then put it at the top
    current_context->stack[num_elements - 1] = widget;
    current_context->stack[num_elements] = NULL;
}
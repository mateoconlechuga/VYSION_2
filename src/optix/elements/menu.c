#include "menu.h"
//includes
#include <stdint.h>
#include <stdbool.h>
#include <graphx.h>
#include <keypadc.h>
//OPTIX includes
#include "../gui_control.h"
#include "../cursor.h"
#include "../init.h"

//Menu rewrite 1
//curr_selection must be curr_selection - menu_min, or the option's place in the grid
uint16_t optix_GetMenuOptionWidth(int curr_selection, int rows, int columns, uint16_t width, uint8_t height) {
    return (width / columns) + ((curr_selection + 1) % columns == 0) * (width % columns);
}

uint8_t optix_GetMenuOptionHeight(int curr_selection, int rows, int columns, uint16_t width, uint8_t height) {
    return (height / rows) + (curr_selection / columns == rows - 1) * (height % rows);
}

void optix_UpdateMenu_default(struct optix_widget *widget) {
    struct optix_menu *menu = (struct optix_menu *) widget;
    //if we should check if we want to jump out of the menu (tries to access inapplicable option, like too far up or down or left or right)
    bool needs_jump = false;
    if (menu->selection != MENU_NO_SELECTION) menu->last_selection = menu->selection;
    //check if it overlaps with the cursor
    if (current_context->cursor->current_selection == widget || (optix_CheckTransformOverlap(&current_context->cursor->widget, widget) && current_context->settings->cursor_active)) {
        //handle if it was pressed
        menu->needs_partial_redraw = false;
        if (kb_Data[6] & kb_Enter || kb_Data[1] & kb_2nd) {
            dbg_sprintf(dbgout, "Please no.\n");
            if (!menu->pressed && (current_context->settings->cursor_active || (widget->state.selected && current_context->data->can_press))) {
                dbg_sprintf(dbgout, "Menu click action...\n");
                if (menu->click_action.click_action) menu->click_action.click_action(menu->pass_self ? widget : menu->click_action.click_args);
                //button->state.color = 224;
                menu->pressed = true;
                if (!current_context->settings->cursor_active) current_context->data->can_press = false;
            }
        } else {
            if (menu->pressed) widget->state.needs_redraw = true;
            menu->pressed = false;
        }
        //restore the previous selection if cursor was off
        if (!widget->state.selected && !current_context->settings->cursor_active) {
            menu->selection = menu->last_selection;
            widget->state.needs_redraw = true;
        }
        widget->state.selected = true;
        //only do this if we have to
        if (current_context->settings->cursor_active) {
            uint16_t option_width = optix_GetMenuOptionWidth(0, menu->rows, menu->columns, widget->transform.width, widget->transform.height);
            uint8_t option_height = optix_GetMenuOptionHeight(0, menu->rows, menu->columns, widget->transform.width, widget->transform.height);
            //handle scrolling
            if (current_context->data->can_press && kb_Data[6] & kb_Add) {
                menu->min = menu->min + menu->columns < menu->num_options - (menu->rows - 1) * menu->columns ? menu->min + menu->columns : menu->min;
                current_context->data->can_press = false;
                widget->state.needs_redraw = true;
            } else if (current_context->data->can_press && kb_Data[6] & kb_Sub) {
                menu->min = menu->min - menu->columns > 0 ? menu->min - menu->columns : 0;
                current_context->data->can_press = false;
                widget->state.needs_redraw = true;
            }
            current_context->data->can_press = !kb_AnyKey();
            menu->selection = menu->min + (((current_context->cursor->widget.transform.y - widget->transform.y) / option_height) * menu->columns + 
            ((current_context->cursor->widget.transform.x - widget->transform.x) / option_width));
        } else if (current_context->cursor->current_selection == widget) {
            if (current_context->data->can_press && kb_Data[7]) {
                if (current_context->data->can_press && kb_Data[7] & kb_Up) {
                    menu->selection = (needs_jump = (menu->selection == 0)) ? 0 : menu->selection - menu->columns;
                    current_context->data->can_press = false;
                //we'll initialize the num_options field in the InitializeWidget method, probably
                } else if (current_context->data->can_press && kb_Data[7] & kb_Down) {
                    menu->selection = (needs_jump = (menu->selection == menu->num_options - 1)) ? menu->num_options - 1 : menu->selection + menu->columns;
                    current_context->data->can_press = false;
                //for both of these, don't do it if at the edge
                } else if (current_context->data->can_press && kb_Data[7] & kb_Left && !(needs_jump = menu->selection % menu->columns == 0)) {
                    menu->selection--;
                    current_context->data->can_press = false;
                } else if (current_context->data->can_press && kb_Data[7] & kb_Right && !(needs_jump = (((menu->selection + 1) % menu->columns) == 0))) {
                    menu->selection++;
                    current_context->data->can_press = false;
                }
                //if it's a transparent background we're going to need to redraw things
                if (menu->transparent_background && !menu->hide_selection_box) optix_IntelligentRecursiveSetNeedsRedraw(current_context->stack, widget);
                if (needs_jump) current_context->data->can_press = true;
            } //else current_context->data->can_press = !kb_AnyKey();
        }
        //scroll if we need to
        //handle out of bounds
        menu->selection = menu->selection < 0 ? 0 : ((menu->selection >= menu->num_options) ? menu->num_options - 1 : menu->selection);
        if (menu->selection < menu->min) {
            menu->min = ((int) menu->selection / menu->columns) * menu->columns;
            widget->state.needs_redraw = true;
        } else if (menu->selection > menu->min + (menu->rows - 1) * menu->columns) {
            menu->min = ((int) menu->selection / menu->columns - (menu->rows - 1)) * menu->columns;
            widget->state.needs_redraw = true;
        }
    } else {
        //if it was selected the last loop, signal for the last selection to be redrawn as unselected
        if (widget->state.selected) menu->needs_partial_redraw = true;
        menu->selection = MENU_NO_SELECTION;
        widget->state.selected = false;
        //make sure we don't run into issues with this
        //current_context->data->can_press = true;
    }
}

void optix_RenderMenu_default(struct optix_widget *widget) {
    struct optix_menu *menu = (struct optix_menu *) widget;
    //we're going to be handling this by not handling it (just make the menu be a glorified collection of buttons, and rendered the same way)
    //each has some text and some sprite
    struct optix_text text = {.text = NULL};
    struct optix_sprite sprite = {.spr = NULL, .x_scale = 1, .y_scale = 1};
    struct optix_widget *button_children[] = {&text.widget, &sprite.widget, NULL};
    struct optix_button button = {.widget = {.child = button_children}};
    optix_InitializeWidget(&button.widget, OPTIX_BUTTON_TYPE);
    //return if we don't have to do anything
    if (!(widget->state.needs_redraw || (menu->selection != menu->last_selection && widget->state.selected) || menu->needs_partial_redraw)) return;
    //if ((!(widget->state.needs_redraw || menu->selection != menu->last_selection))) return;
    //just draw it
    for (int i = menu->min; i < menu->min + menu->rows * menu->columns; i++) {
        if (widget->state.needs_redraw || i == menu->selection || i == menu->last_selection) {
            if (!widget->state.needs_redraw && !widget->state.selected) continue;
            button.widget.child = (i < menu->num_options) ? button_children : NULL;
            button.pressed = (i == menu->selection && menu->pressed);
            button.transparent_background = menu->transparent_background || (i == menu->selection && menu->hide_selection_box);
            //create that button widget
            //text
            if (button.widget.child && menu->text && menu->text[i]) {
                text.text = menu->text[i];
                optix_InitializeTextTransform(&text);
                optix_InitializeWidget(&text.widget, OPTIX_TEXT_TYPE);
                text.background_rectangle = false;
                text.widget.centering.x_centering = menu->text_centering.x_centering;
                text.widget.centering.y_centering = menu->text_centering.y_centering;
                text.widget.centering.x_offset = menu->text_centering.x_offset;
                text.widget.centering.y_offset = menu->text_centering.y_offset;
                //text.widget.state.needs_redraw = true;
            } else text.text = NULL;
            //same for sprites
            if (button.widget.child && menu->spr && menu->spr[i]) {
                sprite.spr = menu->spr[i];
                sprite.x_scale = menu->spr_x_scale;
                sprite.y_scale = menu->spr_y_scale;
                optix_InitializeWidget(&sprite.widget, OPTIX_SPRITE_TYPE);
                sprite.widget.centering.x_centering = menu->sprite_centering.x_centering;
                sprite.widget.centering.y_centering = menu->sprite_centering.y_centering;
                sprite.widget.centering.x_offset = menu->sprite_centering.x_offset;
                sprite.widget.centering.y_offset = menu->sprite_centering.y_offset;
                //sprite.widget.state.needs_redraw = true;
            } else sprite.spr = NULL;
            //place the null
            if (button.widget.child) {
                button.widget.child[0] = &text.widget;
                button.widget.child[(text.text != NULL)] = &sprite.widget;
                button.widget.child[(text.text != NULL) + (sprite.spr != NULL)] = NULL;
            }
            //align everything (please don't stab me)
            //x
            button.widget.transform.x = widget->transform.x + (i % menu->columns * 
            (button.widget.transform.width = optix_GetMenuOptionWidth(menu->selection, menu->rows, menu->columns, widget->transform.width, widget->transform.height)));
            //y
            button.widget.transform.y = widget->transform.y + ((i - menu->min) / menu->columns * 
            (button.widget.transform.height = optix_GetMenuOptionHeight(menu->selection, menu->rows, menu->columns, widget->transform.width, widget->transform.height)));
            optix_RecursiveAlign(&button.widget);
            //set whether it's selected
            button.widget.state.selected = widget->state.selected && i == menu->selection && !menu->hide_selection_box;
            optix_RecursiveSetNeedsRedraw(button.widget.child);
            button.widget.render(&button.widget);
        }
    }
    //this is true, I think?
    current_context->data->needs_blit = true;
    menu->needs_partial_redraw = false;
}
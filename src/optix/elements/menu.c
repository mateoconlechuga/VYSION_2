#include "menu.h"
//includes
#include <stdint.h>
#include <stdbool.h>
#include <graphx.h>
#include <keypadc.h>
#include <debug.h>
//OPTIX includes
#include "../gui_control.h"
#include "../cursor.h"
#include "../init.h"
#include "../util.h"

//Menu rewrite 1
//curr_selection must be curr_selection - menu_min, or the option's place in the grid
uint16_t optix_GetMenuOptionWidth(int option, struct optix_menu *menu) {
    struct optix_transform *t = &(menu->widget.transform);
    return (t->width / menu->columns) + ((option + 1) % menu->columns == 0) * (t->width % menu->columns);
}

uint8_t optix_GetMenuOptionHeight(int option, struct optix_menu *menu) {
    struct optix_transform *t = &(menu->widget.transform);
    return (t->height / menu->rows) + (option / menu->columns == menu->rows - 1) * (t->height % menu->rows);
}

int optix_GetMenuOptionRelativeX(int option, struct optix_menu *menu) {
    return option % menu->columns * optix_GetMenuOptionWidth(option, menu);
}

int optix_GetMenuOptionRelativeY(int option, struct optix_menu *menu) {
    return (option - menu->min) / menu->columns * optix_GetMenuOptionHeight(option, menu);
}

void optix_SetMenuOptionTransform(int option, struct optix_button *button, struct optix_menu *menu) {
    struct optix_widget *widget = (struct optix_widget *) menu;
    struct optix_transform *transform = &(button->widget.transform);
    transform->x = widget->transform.x + optix_GetMenuOptionRelativeX(option, menu);
    transform->y = widget->transform.y + optix_GetMenuOptionRelativeY(option, menu);
    transform->width = optix_GetMenuOptionWidth(option, menu);
    transform->height = optix_GetMenuOptionHeight(option, menu);
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
        if (optix_DefaultKeyIsDown(KEY_ENTER) & KEY_PRESSED) {
            if (menu->click_action.click_action) menu->click_action.click_action(menu->pass_self ? widget : menu->click_action.click_args);
            widget->state.needs_redraw = true;
        } else if (optix_DefaultKeyIsDown(KEY_ENTER) & KEY_RELEASED)
            widget->state.needs_redraw = true;
        //restore the previous selection if cursor was off
        if (!widget->state.selected && !current_context->settings->cursor_active) {
            menu->selection = menu->last_selection;
            widget->state.needs_redraw = true;
        }
        widget->state.selected = true;
        //only do this if we have to
        if (current_context->settings->cursor_active) {
            uint16_t option_width = optix_GetMenuOptionWidth(0, menu);
            uint8_t option_height = optix_GetMenuOptionHeight(0, menu);
            //handle scrolling
            if (optix_DefaultKeyIsDown(KEY_ADD) & KEY_PRESSED) {
                menu->min = menu->min + menu->columns < menu->num_options - (menu->rows - 1) * menu->columns ? menu->min + menu->columns : menu->min;
                widget->state.needs_redraw = true;
            } else if (optix_DefaultKeyIsDown(KEY_SUB) & KEY_PRESSED) {
                menu->min = menu->min - menu->columns > 0 ? menu->min - menu->columns : 0;
                widget->state.needs_redraw = true;
            }
            menu->selection = menu->min + (((current_context->cursor->widget.transform.y - widget->transform.y) / option_height) * menu->columns + 
            ((current_context->cursor->widget.transform.x - widget->transform.x) / option_width));
        } else if (current_context->cursor->current_selection == widget) {
            bool key_pressed = false;
            if (optix_DefaultKeyIsDown(KEY_UP) & KEY_PRESSED) {
                menu->selection = (needs_jump = (menu->selection == 0)) ? 0 : menu->selection - menu->columns;
                key_pressed = true;
            }
            if (optix_DefaultKeyIsDown(KEY_DOWN) & KEY_PRESSED) {
                menu->selection = (needs_jump = (menu->selection == menu->num_options - 1)) ? menu->num_options - 1 : menu->selection + menu->columns;
                key_pressed = true;
            }
            //for both of these, don't do it if at the edge
            if (optix_DefaultKeyIsDown(KEY_LEFT) & KEY_PRESSED && !(needs_jump = menu->selection % menu->columns == 0)) {
                menu->selection--;
                key_pressed = true;
            }
            if (optix_DefaultKeyIsDown(KEY_RIGHT) & KEY_PRESSED && !(needs_jump = (((menu->selection + 1) % menu->columns) == 0))) {
                menu->selection++;
                key_pressed = true;
            }
            //if it's a transparent background we're going to need to redraw things
            if (key_pressed)
                if (menu->transparent_background && !menu->hide_selection_box) optix_IntelligentRecursiveSetNeedsRedraw((struct optix_widget **) current_context->stack, widget);
            //make sure we don't jump out of the menu where applicable
            if (!needs_jump) current_context->cursor->direction = OPTIX_CURSOR_NO_DIR;
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
        //so if there is an element, only the one within the currently selected box will be updated
        //I think this makes sense?
        if (menu->element && menu->element[menu->selection]) menu->element[menu->selection]->update(menu->element[menu->selection]);
    } else {
        //if it was selected the last loop, signal for the last selection to be redrawn as unselected
        if (widget->state.selected) menu->needs_partial_redraw = true;
        menu->selection = MENU_NO_SELECTION;
        widget->state.selected = false;
        //make sure we don't run into issues with this
    }
}

//offload some of this here for simplicity
void optix_RenderMenuOption(int option, struct optix_menu *menu, char *option_text, gfx_sprite_t *option_spr) {
    struct optix_widget *widget = (struct optix_widget *) menu;
    //we're going to be handling this by not handling it (just make the menu be a glorified collection of buttons, and rendered the same way)
    //each has some text and some sprite
    struct optix_text text;
    struct optix_sprite sprite;
    struct optix_widget *button_children[] = {&text.widget, &sprite.widget, NULL, NULL};
    struct optix_button button = {.widget = {.child = button_children}};
    optix_InitializeWidget(&button.widget, OPTIX_BUTTON_TYPE);
    if (widget->state.needs_redraw || option == menu->selection || option == menu->last_selection) {
        button.widget.child = button_children;
        button.pressed = (option == menu->selection && optix_DefaultKeyIsDown(KEY_ENTER) & KEY_HELD);
        button.transparent_background = menu->transparent_background || (option == menu->selection && menu->hide_selection_box);
        if (button.widget.child && option_text) {
            memcpy(&text.widget, &(menu->text_args), sizeof(struct optix_text));
            text.text = option_text;
            optix_InitializeTextTransform(&text);
            optix_InitializeWidget(&text.widget, OPTIX_TEXT_TYPE);
        } else text.text = NULL;
        //same for sprites
        if (button.widget.child && option_spr) {
            memcpy(&sprite.widget, &(menu->sprite_args), sizeof(struct optix_sprite));
            sprite.spr = option_spr;
            optix_InitializeWidget(&sprite.widget, OPTIX_SPRITE_TYPE);
        } else sprite.spr = NULL;
        //place the null, add the special element if applicable
        if (button.widget.child) {
            button.widget.child[0] = &text.widget;
            button.widget.child[(text.text != NULL)] = &sprite.widget;
            //put the element in there to be rendered as well
            if (menu->element && menu->element[menu->selection])
                button.widget.child[(text.text != NULL && sprite.spr != NULL)] = menu->element[menu->selection];
            button.widget.child[(text.text != NULL) + (sprite.spr != NULL) + (menu->element != NULL && menu->element[menu->selection] != NULL)] = NULL;
        }
        //handle the transform
        optix_SetMenuOptionTransform(option, &button, menu);
        //this option will be shown as selected if it is selected and the selection box is not hidden
        optix_RecursiveAlign(&button);
        button.widget.state.selected = option == menu->selection && !menu->hide_selection_box;
        optix_RecursiveSetNeedsRedraw(button.widget.child);
        button.widget.render(&button.widget);
    }

}


void optix_RenderMenu_default(struct optix_widget *widget) {
    struct optix_menu *menu = (struct optix_menu *) widget;
    //return if we don't have to do anything
    if (!(widget->state.needs_redraw || (menu->selection != menu->last_selection && widget->state.selected) || menu->needs_partial_redraw)) return;
    //if ((!(widget->state.needs_redraw || menu->selection != menu->last_selection))) return;
    //just draw it
    for (int i = menu->min; i < menu->min + menu->rows * menu->columns; i++) {
        char *option_text = menu->text && menu->text[i] ? menu->text[i] : NULL;
        gfx_sprite_t *option_spr = menu->spr && menu->spr[i] ? menu->spr[i] : NULL;
        if (i >= menu->num_options) break;
        optix_RenderMenuOption(i, menu, option_text, option_spr);
    }
    //this will only be run if we didn't return earlier, which will happen if the selection is different or we need a redraw of some sort
    current_context->data->needs_blit = true;
    menu->needs_partial_redraw = false;
}
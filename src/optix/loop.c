#include "loop.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <keypadc.h>
#include <tice.h>
#include <string.h>

#include "shortcuts.h"
#include "cursor.h"
#include "input.h"

#include "elements/window.h"

//takes an array of optix_widgets as an argument
//please have a NULL as the last entry in this array, so we'll know when to stop
//this should be a pointer to an array of pointers
void optix_UpdateGUI(void) {
    //we're assuming this is called in a loop
    //so this is the time since it was last called
    current_context->data->ticks = (long) timer_2_Counter;
    timer_2_Counter = 0;
    //input things
    optix_UpdateInput();
    //we need this here unfortunately
    //start with this I suppose
    current_context->cursor->widget.update((struct optix_widget *) current_context->cursor);
    optix_HandleShortcuts(*current_context->stack);
    optix_UpdateStack_TopLevel(current_context->stack);
}

void optix_UpdateStack(struct optix_widget *stack[]) {
    int i = 0;
    while (stack[i]) {
        if (stack[i]->update) stack[i]->update(stack[i]);
        if (stack[i]->state.needs_redraw && stack[i]->child) {
            optix_RecursiveSetNeedsRedraw(stack[i]->child);
            if (stack[i]->type == OPTIX_WINDOW_TITLE_BAR_TYPE) optix_RecursiveSetNeedsRedraw(((struct optix_window_title_bar *) stack[i])->window->widget.child);
        }
        //if this has been selected, we want to loop through and make sure nothing else is selected
        //this is so that what's on top will be selected, or what is rendered last
        if (stack[i]->state.selected) {
            for (int j = 0; j < i; j++) stack[j]->state.selected = false;
            //break;
        }
        i++;
    }
}

//use only for the top level stack, it will reorganize the windows in the render queue and things
void optix_UpdateStack_TopLevel(struct optix_widget *(*stack)[]) {
    int i = 0;
    struct optix_widget *curr_window = NULL;
    int curr_window_index = 0;
    bool found_window = false;
    bool window_needs_focus = false;
    bool window_selected = false;
    bool found_window_with_focus = false;
    bool found_selected_window = false;
    //start things out by doing the thing
    //the value of i at the end will also be the number of elements in the stack
    while ((*stack)[i]) {
        //if this has been selected, we want to loop through and make sure nothing else is selected
        //this is so that what's on top will be selected, or what is rendered last
        if ((*stack)[i]->update) (*stack)[i]->update((*stack)[i]);
        //only one window can be selected at a time
        //if it needs to be redrawn handle that
        if ((*stack)[i]->state.needs_redraw && (*stack)[i]->child) {
            optix_RecursiveSetNeedsRedraw((*stack)[i]->child);
            if ((*stack)[i]->type == OPTIX_WINDOW_TITLE_BAR_TYPE) optix_RecursiveSetNeedsRedraw(((struct optix_window_title_bar *) (*stack)[i])->window->widget.child);
        }
        //if we've found a focused window, just continue
        if (found_window_with_focus) {
            i++;
            continue;
        }
        //if it's a window, and reports it needs to be moved to the top, stop everything and do that
        if ((*stack)[i]->type == OPTIX_WINDOW_TITLE_BAR_TYPE) {
            struct optix_window_title_bar *window_title_bar = (struct optix_window_title_bar *) (*stack)[i];
            window_needs_focus = window_title_bar->window->needs_focus;
            window_selected = window_title_bar->window->widget.state.selected;
            window_title_bar->window->needs_focus = false;
        } else if ((*stack)[i]->type == OPTIX_WINDOW_TYPE) {
            struct optix_window *window = (struct optix_window *) (*stack)[i];
            window_needs_focus = window->needs_focus;
            window_selected = window->widget.state.selected;
            window->needs_focus = false;
        }
        if (window_needs_focus || window_selected) {
            if (window_needs_focus) found_window_with_focus = true;
            if (window_selected) found_selected_window = true;
            found_window = true;
            curr_window_index = i;
            curr_window = (*stack)[i];
        }
        i++;
    }
    //handle this as well
    //start with this, because why not
    if (current_context->data->gui_needs_full_redraw) optix_RecursiveSetNeedsRedraw(*stack);
    //cursor stuff
    if (current_context->cursor->direction == OPTIX_CURSOR_FORCE_UPDATE || (!current_context->settings->cursor_active && current_context->cursor->direction != OPTIX_CURSOR_NO_DIR)) {
        //formerly another condition || curr_window_index + 1 != i)
        struct optix_widget *possible_selection = NULL;
        struct optix_widget **search_stack = NULL;
        if (curr_window) {
            if (curr_window->type == OPTIX_WINDOW_TYPE) search_stack = curr_window->child;
            else if (curr_window->type == OPTIX_WINDOW_TITLE_BAR_TYPE) search_stack = ((struct optix_window_title_bar *) curr_window)->window->widget.child;
        } else search_stack = *(current_context->stack);
        if (current_context->cursor->direction == OPTIX_CURSOR_FORCE_UPDATE || (!current_context->cursor->current_selection) || (curr_window_index + 1 != i && found_window)) {
            int i = 0;
            while (search_stack[i] && !search_stack[i]->state.selectable) i++;
            possible_selection = search_stack[i];
        } else possible_selection = optix_FindNearestElement(current_context->cursor->direction, current_context->cursor->current_selection, search_stack);
        //move the cursor to that position
        if (possible_selection) optix_SetCurrentSelection(possible_selection);
        if (current_context->cursor->direction == OPTIX_CURSOR_FORCE_UPDATE) current_context->cursor->direction = OPTIX_CURSOR_NO_DIR;
    }
    //if it's already the last entry don't bother
    if (!found_window) return;
    //set everything in the array to unselected, except for the current window
    for (int j = 0; j < i; j++) {
        struct optix_window_title_bar *window_title_bar = (struct optix_window_title_bar *) (*stack)[j];
        if ((*stack)[j]->type == OPTIX_WINDOW_TITLE_BAR_TYPE) (*stack)[j]->state.selected = window_title_bar->window->widget.state.selected = (j == curr_window_index && (!found_window_with_focus || found_selected_window));
        else if ((*stack)[j]->type == OPTIX_WINDOW_TYPE) (*stack)[j]->state.selected = (j == curr_window_index && (!found_window_with_focus || found_selected_window));
    }
    if (i == curr_window_index + 1) return;
    memmove((void *) stack + (curr_window_index * sizeof(struct optix_widget ***)), (void *) stack + ((curr_window_index + 1) * sizeof(struct optix_widget ***)),
    (i - curr_window_index) * sizeof(struct optix_widget ***));
    (*stack)[i - 1] = curr_window;
}


//takes an array of optix_widgets
//please have a NULL as the last entry in this array, so we'll know when to stop
void optix_RenderGUI(void) {
    //do this first
    optix_RenderCursorBackground((struct optix_widget *) current_context->cursor);
    optix_RenderStack(*(current_context->stack));
    //blit if necessary
    if (current_context->data->needs_blit) gfx_BlitBuffer();
    //the cursor should be on top of everything else
    current_context->cursor->widget.render((struct optix_widget *) current_context->cursor);
    current_context->data->gui_needs_full_redraw = false;
    current_context->data->needs_blit = false;
}

void optix_RenderStack(struct optix_widget *stack[]) {
    int i = 0;
    while (stack && stack[i]) {
        if (stack[i]->render) {
            //make sure we blit when needed
            if (stack[i]->state.needs_redraw) current_context->data->needs_blit = true;
            stack[i]->render(stack[i]);
            stack[i]->state.needs_redraw = false;
        }
        i++;
    }
}
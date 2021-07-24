#include "loop.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <keypadc.h>
#include "shortcuts.h"
#include "cursor.h"

//takes an array of optix_widgets as an argument
//please have a NULL as the last entry in this array, so we'll know when to stop
//this should be a pointer to an array of pointers
void optix_UpdateGUI(void) {
    //we're assuming this is called in a loop
    //so this is the time since it was last called
    current_context->data->ticks = (long) timer_3_Counter;
    timer_3_Counter = 0;
    current_context->data->key = os_GetCSC();
    //we need this here unfortunately
    current_context->cursor->state = OPTIX_CURSOR_NORMAL;
    kb_Scan();
    //if (!kb_AnyKey()) current_context->data->can_press = true;
    //start with this I suppose
    current_context->cursor->widget.update((struct optix_widget *) current_context->cursor);
    optix_HandleShortcuts(*current_context->stack);
    optix_UpdateStack_TopLevel(current_context->stack);
}

void optix_UpdateStack(struct optix_widget *stack[]) {
    int i = 0;
    while (stack[i]) {
        //dbg_sprintf(dbgout, "Level type: %d\n", stack[i]->type);
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
    bool found_window_with_focus = false;
    //start things out by doing the thing
    //the value of i at the end will also be the number of elements in the stack
    while ((*stack)[i]) {
        //dbg_sprintf(dbgout, "Top level type: %d Updating...\n", (*stack)[i]->type);
        //if this has been selected, we want to loop through and make sure nothing else is selected
        //this is so that what's on top will be selected, or what is rendered last
        //only one window can be selected at a time
        if ((*stack)[i]->update) (*stack)[i]->update((*stack)[i]);
        //dbg_sprintf(dbgout, "Success.\n");
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
            window_title_bar->window->needs_focus = false;
        } else if ((*stack)[i]->type == OPTIX_WINDOW_TYPE) {
            struct optix_window *window = (struct optix_window *) (*stack)[i];
            window_needs_focus = window->needs_focus;
            window->needs_focus = false;
        }
        if (window_needs_focus || ((*stack)[i]->state.selected && ((*stack)[i]->type == OPTIX_WINDOW_TITLE_BAR_TYPE || (*stack)[i]->type == OPTIX_WINDOW_TYPE))) {
            if (window_needs_focus) found_window_with_focus = true;
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
    if (current_context->cursor->direction == OPTIX_CURSOR_FORCE_UPDATE || (!current_context->settings->cursor_active && current_context->data->can_press && current_context->cursor->direction != OPTIX_CURSOR_NO_DIR)) {
        //formerly another condition || curr_window_index + 1 != i)
        dbg_sprintf(dbgout, "Please work.\n");
        struct optix_widget *possible_selection = NULL;
        struct optix_widget **temp = NULL;
        struct optix_widget **search_stack = NULL;
        if (curr_window) {
            if (curr_window->type == OPTIX_WINDOW_TYPE) search_stack = curr_window->child;
            else if (curr_window->type == OPTIX_WINDOW_TITLE_BAR_TYPE) search_stack = ((struct optix_window_title_bar *) curr_window)->window->widget.child;
        } else {
            dbg_sprintf(dbgout, "Fuck off Eric.\n");
            search_stack = current_context->stack;
        }
        if (current_context->cursor->direction == OPTIX_CURSOR_FORCE_UPDATE || (!current_context->cursor->current_selection) || (curr_window_index + 1 != i && found_window)) {
            int i = 0;
            while (search_stack[i] && !search_stack[i]->state.selectable) i++;
            possible_selection = search_stack[i];
        } else possible_selection = optix_FindNearestElement(current_context->cursor->direction, current_context->cursor->current_selection, search_stack);
        //move the cursor to that position
        if (possible_selection) {
            current_context->data->can_press = false;
            optix_SetCurrentSelection(possible_selection);
            /*current_context->cursor->current_selection = possible_selection;
            current_context->cursor->widget.transform.x = current_context->cursor->current_selection->transform.x;
            current_context->cursor->widget.transform.y = current_context->cursor->current_selection->transform.y;*/
        }
        if (current_context->cursor->direction == OPTIX_CURSOR_FORCE_UPDATE) current_context->cursor->direction = OPTIX_CURSOR_NO_DIR;
    }
    //if it's already the last entry don't bother
    if (!found_window) return;
    //set everything in the array to unselected, except for the current window
    for (int j = 0; j < i; j++) {
        struct optix_window_title_bar *window_title_bar = (struct optix_window_title_bar *) (*stack)[j];
        if ((*stack)[j]->type == OPTIX_WINDOW_TITLE_BAR_TYPE) (*stack)[j]->state.selected = window_title_bar->window->widget.state.selected = (j == curr_window_index);
        else if ((*stack)[j]->type == OPTIX_WINDOW_TYPE) (*stack)[j]->state.selected = (j == curr_window_index);
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
    //dbg_sprintf(dbgout, "Rendering stack...\n");
    optix_RenderStack(current_context->stack);
    //dbg_sprintf(dbgout, "Finished.\n");
    //the cursor should be on top of everything else
    current_context->cursor->widget.render((struct optix_widget *) current_context->cursor);
    current_context->data->gui_needs_full_redraw = false;
}

void optix_RenderStack(struct optix_widget *stack[]) {
    int i = 0;
    while (stack[i]) {
        //dbg_sprintf(dbgout, "Render type: %d\n", stack[i]->type);
        if (stack[i]->render) {
            //dbg_sprintf(dbgout, "Rendering...\n");
            stack[i]->render(stack[i]);
            //dbg_sprintf(dbgout, "We made it back.\n");
            stack[i]->state.needs_redraw = false;
        }
        i++;
    }
    //dbg_sprintf(dbgout, "Finished rendering stack.\n");
}
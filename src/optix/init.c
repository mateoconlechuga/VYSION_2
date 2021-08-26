#include "init.h"

//initializes a widget, and also sets its state to non-selected and visible (which will be updated elsewhere in the GUI)
//initializes the callbacks and things too
void optix_InitializeWidget(struct optix_widget *widget, uint8_t type) {
    struct optix_window_title_bar *window_title_bar = (struct optix_window_title_bar *) widget;
    struct optix_window *window = (struct optix_window *) window_title_bar->window;
    struct optix_menu *menu = (struct optix_menu *) widget;
    struct optix_text *text = (struct optix_text *) widget;
    struct optix_check_box *check_box = (struct optix_check_box *) widget;
    struct optix_sprite *sprite = (struct optix_sprite *) widget;
    void (*update[OPTIX_NUM_TYPES])(struct optix_widget *) = {
        //text
        optix_UpdateText_default,
        //sprite
        NULL,
        //button
        optix_UpdateButton_default,
        //menu
        optix_UpdateMenu_default,
        //window
        optix_UpdateWindow_default,
        //window title bar
        optix_UpdateWindowTitleBar_default,
        //divider
        NULL,
        //rectangle
        NULL,
        //input box
        optix_UpdateInputBox_default,
        //scroll bar
        optix_UpdateScrollBar_default,
        //check box
        optix_UpdateButton_default,
        //slider
        optix_UpdateSlider_default,
    };
    void (*render[OPTIX_NUM_TYPES])(struct optix_widget *) = {
        //text
        optix_RenderText_default,
        //sprite
        optix_RenderSprite_default,
        //button
        optix_RenderButton_default,
        //menu
        optix_RenderMenu_default,
        //window
        optix_RenderWindow_default,
        //window title bar
        optix_RenderWindowTitleBar_default,
        //divider
        optix_RenderDivider_default,
        //rectangle
        optix_RenderRectangle_default,
        //input box
        optix_RenderInputBox_default,
        //scroll bar
        optix_RenderScrollBar_default,
        //check box
        optix_RenderCheckBox_default,
        //slider
        optix_RenderSlider_default,
    };
    bool selectable[OPTIX_NUM_TYPES] = {
        //text
        true,
        //sprite
        false,
        //button
        true,
        //menu
        true,
        //window
        true,
        //window title bar
        true,
        //divider
        false,
        //rectangle
        false,
        //input box
        false,
        //scroll bar
        false,
        //check box
        true,
        //slider
        true,
    };
    //if we're adding stuff we probably want this
    //current_context->data->gui_needs_full_redraw = true;
    widget->type = type;
    widget->state.selected = false;
    widget->state.needs_redraw = true;
    widget->state.visible = true;
    widget->update = update[type];
    widget->render = render[type];
    widget->state.selectable = selectable[type];
    //element-specific things
    switch (type) {
        case OPTIX_MENU_TYPE:
            menu->min = menu->last_selection = menu->selection = menu->num_options = 0;
            menu->spr_x_scale = menu->spr_y_scale = 1;
            while ((menu->spr && menu->spr[menu->num_options]) || (menu->text && menu->text[menu->num_options])) 
                menu->num_options++;
            break;
        case OPTIX_TEXT_TYPE:
            widget->child = NULL;
            text->num_lines = 0;
            text->background_rectangle = true;
            text->min = 0;
            optix_WrapText(widget);
            //no break here, we want it to fall through
        case OPTIX_BUTTON_TYPE:
        case OPTIX_SPRITE_TYPE:
            //hopefully it's safe to assume these have been set properly
            if (widget->type == OPTIX_SPRITE_TYPE) {
                widget->transform.width = sprite->spr->width * sprite->x_scale;
                widget->transform.height = sprite->spr->height * sprite->y_scale;
            }
            widget->centering.x_centering = OPTIX_CENTERING_CENTERED;
            widget->centering.y_centering = OPTIX_CENTERING_CENTERED;
            break;
        case OPTIX_WINDOW_TITLE_BAR_TYPE:
            //initialize the transform for this as well
            widget->transform.x = window->widget.transform.x;
            widget->transform.y = window->widget.transform.y - TITLE_BAR_HEIGHT;
            widget->transform.width = window->widget.transform.width;
            widget->transform.height = TITLE_BAR_HEIGHT;
            break;
        case OPTIX_CHECK_BOX_TYPE:
            check_box->button.click_action.click_action = optix_CheckBoxValueXOR;
            check_box->button.click_action.click_args = (void *) widget;
            break;
    }
}

uint16_t optix_GetSize(struct optix_widget *widget) {
    if (widget->state.override_size) {
        dbg_sprintf(dbgout, "Has custom size of %d.\n", widget->state.size);
        return widget->state.size;
    } else {
        switch (widget->type) {
            case OPTIX_TEXT_TYPE:
                return sizeof(struct optix_text);
                break;
            case OPTIX_SPRITE_TYPE:
                return sizeof(struct optix_sprite);
                break;
            case OPTIX_BUTTON_TYPE:
                return sizeof(struct optix_button);
                break;
            case OPTIX_MENU_TYPE:
                return sizeof(struct optix_menu);
                break;
            case OPTIX_WINDOW_TYPE:
                return sizeof(struct optix_window);
                break;
            case OPTIX_WINDOW_TITLE_BAR_TYPE:
                return sizeof(struct optix_window_title_bar);
                break;
            case OPTIX_DIVIDER_TYPE:
                return sizeof(struct optix_divider);
                break;
            case OPTIX_RECTANGLE_TYPE:
                return sizeof(struct optix_rectangle);
                break;
            case OPTIX_INPUT_BOX_TYPE:
                return sizeof(struct optix_input_box);
                break;
            case OPTIX_SCROLL_BAR_TYPE:
                return sizeof(struct optix_scroll_bar);
                break;
            case OPTIX_CHECK_BOX_TYPE:
                return sizeof(struct optix_check_box);
                break;
            case OPTIX_SLIDER_TYPE:
                return sizeof(struct optix_slider);
                break;
            default:
                return 0;
                break;
        }
    }


}

void optix_CopyElementHandleSpecialCase(struct optix_widget **widget, struct optix_widget *reference) {
    if (reference->type == OPTIX_WINDOW_TITLE_BAR_TYPE) {
        struct optix_window_title_bar *window_title_bar_reference = (struct optix_window_title_bar *) reference;
        struct optix_window_title_bar *window_title_bar = (struct optix_window_title_bar *) *widget;
        optix_CopyElement((struct optix_widget **) &(window_title_bar->window), (struct optix_widget *) window_title_bar_reference->window);
    } /*else if (reference->type == OPTIX_INPUT_BOX_TYPE) {
        struct optix_input_box *input_box_reference = (struct optix_input_box *) reference;
        struct optix_input_box *input_box = (struct optix_input_box *) *widget;
        optix_CopyElement(&(input_box->text.widget), input_box_reference->text);
    }*/
}


//copy all of the things within a given stack, allocating out as needed
void optix_CopyElement(struct optix_widget **widget, struct optix_widget *reference) {
    int i = 0;
    //dbg_sprintf(dbgout, "Getting size...\n");
    uint16_t reference_size = optix_GetSize(reference);
    //we need a + 1 here because the NULL at the end has to be included as well
    //dbg_sprintf(dbgout, "Getting num elements in stack...\n");
    uint16_t num_children = optix_GetNumElementsInStack(reference->child);
    //dbg_sprintf(dbgout, "Fnished that part.\n");
    //we'll start by allocating out the given size of the element and copying it
    *widget = malloc(reference_size);
    memcpy(*widget, reference, reference_size);
    //dbg_sprintf(dbgout, "Maybe it breaks here\n");
    //there are a few special cases we have to handle, unfortunately
    optix_CopyElementHandleSpecialCase(widget, reference);
    //alloc out the array, including the null at the end
    //dbg_sprintf(dbgout, "Type: %d\n", reference->type);
    if (num_children) {
        //dbg_sprintf(dbgout, "Num children: %d\n", num_children);
        (*widget)->child = malloc((num_children + 1) * sizeof(struct optix_widget *));
        //memcpy((*widget)->child, reference->child, num_children * sizeof(struct optix_widget *));
        (*widget)->child[num_children] = NULL;
    } else (*widget)->child = NULL;
    //dbg_sprintf(dbgout, "Beginning loop.\n");
    while (num_children && i < num_children) {
        //uint16_t child_size = optix_GetSize(reference->child[i]);
        optix_CopyElement(&((*widget)->child[i]), reference->child[i]);
        //dbg_sprintf(dbgout, "Child type: %d\n", (*widget)->child[i]->type);
        //that's it, I think?
        //optix_CopyElementHandleSpecialCase(&((*widget)->child[i]), reference->child[i]);
        //don't forget this
        i++;
    }
    //dbg_sprintf(dbgout, "Function finished.\n");
}

//this assumes it has been copied with optix_CopyElement, so keep that in mind please
void optix_FreeElementHandleSpecialCase(struct optix_widget **widget) {
    struct optix_menu *menu = (struct optix_menu *) (*widget);
    switch ((*widget)->type) {
        case OPTIX_WINDOW_TITLE_BAR_TYPE:
            optix_FreeElement((struct optix_widget **) &((struct optix_window_title_bar *) (*widget))->window);
            break;
        case OPTIX_MENU_TYPE:
            if (menu->dynamic) {
                if (menu->text) free(menu->text);
                if (menu->spr) free(menu->spr);
            }
            break;
        default:
            break;
    }
}


void optix_FreeElement(struct optix_widget **widget) {
    //so I think that all we have to really do is just free all the subelements, and then free the main one after that
    int i = 0;
    optix_FreeElementHandleSpecialCase(widget);
    //then this part
    while ((*widget)->child && (*widget)->child[i]) {
        optix_FreeElement(&((*widget)->child[i]));
        i++;
    }
    //do the children too you idiot
    if ((*widget)->child) {
        free((*widget)->child);
        (*widget)->child = NULL;
    }
    free(*widget);
    *widget = NULL;
}
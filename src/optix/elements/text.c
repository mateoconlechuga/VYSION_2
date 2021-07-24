#include "text.h"
#include <debug.h>
#include "input_box.h"

//sets a text object's width and height according to the text's width and height
void optix_InitializeTextTransform(struct optix_text *text) {
    text->widget.transform.x = 0;
    text->widget.transform.y = 0;
    if (current_context->data->font_valid) {
        text->widget.transform.width = fontlib_GetStringWidth(text->text);
        text->widget.transform.height = fontlib_GetCurrentFontHeight();
    } else {
        text->widget.transform.width = gfx_GetStringWidth(text->text);
        text->widget.transform.height = 8;
    }
    //text->widget.centering.x_centering = OPTIX_CENTERING_CENTERED;
    //text->widget.centering.y_centering = OPTIX_CENTERING_CENTERED;
    //text->widget.centering.x_offset = text->widget.centering.y_offset = 0;
}

//generic string length getting function, with a max number of characters
//why isn't this a default in graphx
//obviously the length doesn't include the null
size_t optix_GetStringWidthL(char *str, size_t max_chars) {
    if (current_context->data->font_valid) return fontlib_GetStringWidthL(str, max_chars);
    else if (strlen(str) < max_chars) return gfx_GetStringWidth(str);
    else {
        char temp = str[max_chars + 1];
        size_t size;
        str[max_chars + 1] = NULL;
        size = gfx_GetStringWidth(str);
        str[max_chars + 1] = temp;
        return size;
    }
}

//stolen and modified from DrDnar
//link: https://github.com/drdnar/open-adventure-ce/blob/db326b80753669cddf4a86697e14cdade8f49899/style.c#L186
char *optix_PrintStringWrapped_fontlibc(const char *string, bool fake_print) {
    char old_stop = fontlib_GetAlternateStopCode();
    unsigned int left = fontlib_GetWindowXMin();
    unsigned int width = fontlib_GetWindowWidth();
    unsigned int right = left + width;
    unsigned int str_width;
    unsigned int x = fontlib_GetCursorX();
    unsigned char first_printable = (unsigned char)fontlib_GetFirstPrintableCodePoint();
    unsigned char c;
    unsigned int space_width = fontlib_GetGlyphWidth(' ');
    if (first_printable == '\0') first_printable = '\1';
    fontlib_SetAlternateStopCode(' ');
    do {
        /* Check if the next word can fit on the current line */
        str_width = fontlib_GetStringWidth(string);
        if (x + str_width < right)
            if (!fake_print) x = fontlib_DrawString(string);
            else x += str_width;
        else {
            /* If the word is super-long such that it won't fit in the window,
             * then forcibly print it starting on a new line. */
            if (str_width != 0) {
                if (str_width > width && x == left)
                    if (!fake_print) x = fontlib_DrawString(string);
                    else {
                        do x += (str_width = fontlib_GetGlyphWidth(*string++));
                        while (x < right);
                        string--;
                        break;
                    }
                else break;
            }
            /* If the width returned was zero, that means either another space
             * is waiting to be printed, which will be handled below; or a
             * control code is next, which also will be handled below.  This can
             * occur, for example, if a control code immediately follows a
             * space. */
        }
        /* FontLibC will kindly tell us exactly where it left off. */
        string = fontlib_GetLastCharacterRead();
        /* Now we need to deal with why the last word was terminated. */
        c = (unsigned char) (*string);
        /* If it's a control code, we either process a newline or give up. */
        if (c < first_printable) {
            if (c == ZERO_WIDTH_SPACE) string++;
            else if (c == '\t') {
                string++;
                x += 16;
                x &= 0xFFFFF0;
                if (!fake_print) {
                    fontlib_ClearEOL();
                    fontlib_SetCursorPosition(x, fontlib_GetCursorY());
                }
            }
            else break;
        }
        /* If it's a space, we need to process that manually since DrawString
         * won't handle it because we set space as a stop code. */
        if (c == ' ') {
            string++;
            /* We do actually need to check if there's space to print the
             * space. */
            if (x + space_width < right) {
                if (!fake_print) fontlib_DrawGlyph(' ');
                x += space_width;
            }
            else {
                /* If there isn't room, we need to eat the space; it would look
                 * weird to print a space at the start of the next line. 
                 * However, we do not eat ALL the spaces if there's more than
                 * one, just the first one or two. */
                if (*string == ' ') /* Take care of possible second space. */
                    string++;
                break;
            }
        }
    } while (true);
    if (!fake_print) fontlib_ClearEOL();
    fontlib_SetAlternateStopCode(old_stop);
    return string;
}

void optix_UpdateText_default(struct optix_widget *widget) {
    struct optix_text *text = (struct optix_text *) widget;
    if (current_context->cursor->current_selection == widget || (current_context->settings->cursor_active && optix_CheckTransformOverlap(&current_context->cursor->widget, widget))) {
        int max_lines = widget->transform.height / TEXT_SPACING;
        int lines_to_render = max_lines < text->num_lines ? max_lines : text->num_lines;
        //handle the scrolling of the text
        bool up_pressed = current_context->settings->cursor_active ? (kb_Data[6] & kb_Sub) : (kb_Data[7] & kb_Up);
        bool down_pressed = current_context->settings->cursor_active ? (kb_Data[6] & kb_Add) : (kb_Data[7] & kb_Down);
        if (current_context->data->can_press && down_pressed) {
            text->min = text->min < text->num_lines - lines_to_render ? text->min + 1 : text->min;
            current_context->data->can_press = false;
            widget->state.needs_redraw = true;
        } else if (current_context->data->can_press && up_pressed) {
            text->min = text->min > 0 ? text->min - 1 : 0;
            current_context->data->can_press = false;
            widget->state.needs_redraw = true;
        }
    }
    //all we need to do here is to update the wrap if necessary
    if (current_context->settings->cursor_active) current_context->data->can_press = !kb_AnyKey();
    if (text->needs_offset_update) optix_WrapText(text);
}

void optix_RenderText_default(struct optix_widget *widget) {
    struct optix_text *text = (struct optix_text *) widget;
    if (widget->state.visible && widget->state.needs_redraw) {
        if (current_context->data->font_valid) {
            int max_lines = widget->transform.height / TEXT_SPACING;
            int lines_to_render = max_lines < text->num_lines ? max_lines : text->num_lines;
            fontlib_SetWindow(widget->transform.x, widget->transform.y, widget->transform.width, widget->transform.height);
            if (text->background_rectangle)
                optix_OutlinedRectangle_WithBevel(widget->transform.x, widget->transform.y, widget->transform.width, widget->transform.height,           //x, y, width, height
                BUTTON_BG_COLOR_UNSELECTED_INDEX, WINDOW_BORDER_BEVEL_DARK_INDEX, WINDOW_BORDER_BEVEL_LIGHT_INDEX);                                      //bevel (make it look depressed)
            for (int i = text->min; i < text->min + lines_to_render; i++) {
                size_t num_chars = i == text->num_lines - 1 ? strlen(text->text) : (size_t) (text->offsets[i + 1] - text->offsets[i]);
                size_t line_width = optix_GetStringWidthL(text->offsets[i], num_chars);
                //fontlib_SetCursorPosition((widget->transform.x + line_width / 2) * text->alignment + text->x_offset, widget->transform.y + i * TEXT_SPACING);
                fontlib_SetCursorPosition(widget->transform.x + text->x_offset, widget->transform.y + (i - text->min) * TEXT_SPACING);
                if (i < text->num_lines) fontlib_DrawStringL(text->offsets[i], num_chars);
                else fontlib_DrawString(text->offsets[i]);
            }
            //fontlib_SetCursorPosition(widget->transform.x, widget->transform.y);
            //fontlib_DrawString(text->text);
        } else gfx_PrintStringXY(text->text, widget->transform.x, widget->transform.y);
    }
}

void optix_WrapText(struct optix_widget *widget) {
    struct optix_text *text = (struct optix_text *) widget;
    struct optix_input_box *input_box = (struct optix_input_box *) widget;
    //all we need to do here is to update the wrap if necessary
    int num_lines = 0;
    char *str = text->text;
    bool multiple_lines = true;
    bool found_current_line = false;
    fontlib_SetWindow(widget->transform.x, widget->transform.y, widget->transform.width, widget->transform.height);
    fontlib_SetCursorPosition(widget->transform.x, widget->transform.y);
    //basically we don't want to do any of this is the string is already going to fit
    if (optix_GetStringWidthL(text->text, strlen(text->text)) <= widget->transform.width) multiple_lines = false;
    do {
        //current line is the first one that's greater than input_box->text.text + input_box->cursor_offset, I think
        if (!found_current_line && widget->type == OPTIX_INPUT_BOX_TYPE && str > input_box->text.text + input_box->cursor_offset) {
            found_current_line = true;
            input_box->current_line = num_lines - 1;
        }
        num_lines++;
        //dbg_sprintf(dbgout, "Reallocating...%d\n", num_lines);
        if (num_lines > text->num_lines) text->offsets = realloc(text->offsets, sizeof(char *) * num_lines);
        ////dbg_sprintf(dbgout, "Success.\n");
        text->offsets[num_lines - 1] = str;
        //dbg_sprintf(dbgout, "%s\n", str);
    } while (multiple_lines && (*(str = optix_PrintStringWrapped_fontlibc(str, true)) != '\0') && str != text->text);
    //just do this again
    if (!found_current_line && widget->type == OPTIX_INPUT_BOX_TYPE && str >= text->text + input_box->cursor_offset) {
        input_box->current_line = num_lines - 1;
    }
    //} //else if (num_lines != text->num_lines) text->offsets = realloc(text->offsets, sizeof(char *) * num_lines);
    //cut it down if we need to
    //or if it has too few lines (if it's one line, maybe)
    if (text->num_lines > num_lines) text->offsets = realloc(text->offsets, sizeof(char *) * num_lines);
    text->num_lines = num_lines;
    text->offsets[0] = text->text;
    //have this too
    text->needs_offset_update = false;
}


//sets the default font, which is in an appvar
bool optix_InitializeFont(void) {
    fontlib_font_t *font_pack;
    if (font_pack = fontlib_GetFontByIndex(DEFAULT_FONT_PACK_NAME, 0)) {
        fontlib_SetFont(font_pack, 0);
        fontlib_SetColors(TEXT_FG_COLOR_INDEX, TEXT_BG_COLOR_INDEX);
        fontlib_SetTransparency(true);
    }
    current_context->data->font_valid = (bool) font_pack;
    return (bool) font_pack;
}



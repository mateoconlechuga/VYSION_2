#include "clock.h"

#include <string.h>
#include <tice.h>


#include "../../optix/gui_control.h"
#include "../../optix/elements/text.h"

char clock_text_buffer[9];

//NOTE: make it an option eventually to show seconds too, or use 24 hour time
//also don't use sprintf here because it's making the program size much bigger (4k, jeez!)
void vysion_UpdateClockText(struct optix_widget *widget) {
    struct optix_text *text = (struct optix_text *) widget;
    struct vysion_clock_text *clock_text = (struct vysion_clock_text *) widget;
    uint8_t seconds, minutes, hours;
    uint8_t last_minutes = clock_text->minutes;
    char temp_buffer[3] = "00";
    char *am_pm[] = {"AM", "PM"};
    boot_GetTime(&seconds, &minutes, &hours);
    if (minutes != last_minutes) {
        if (!hours) hours = 12;
        if (minutes < 10) {
            temp_buffer[0] = '0';
            temp_buffer[1] = minutes + '0';
        } else sprintf(temp_buffer, "%d", minutes);
        sprintf(text->text, "%d:%s %s", hours % 12, temp_buffer, am_pm[boot_IsAfterNoon()]);
        clock_text->minutes = minutes;
        widget->state.needs_redraw = true;
    }
}
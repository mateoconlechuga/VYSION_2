#ifndef CLOCK_H
#define CLOCK_H

#include "../../optix/elements/text.h"

//basically just a function that updates a text object to have the time
struct vysion_clock_text {
    struct optix_text text;
    //just update every minute (or more accurately, when minutes changes)
    uint8_t minutes;
};


//the max time would be something like "12:00 PM", which has 8 characters, plus 1 for the null terminator is 9
extern char clock_text_buffer[9];

void vysion_UpdateClockText(struct optix_widget *widget);

#endif
#ifndef BUTTON_H
#define BUTTON_H

//includes
#include <stdbool.h>
#include "../gui_control.h"

//typedefs and things
struct optix_button {
    struct optix_widget widget;
    //the idea with this part is that you could pass whatever kind of arguments you want into that
    //probably a struct
    void (*click_action)(void *);
    void *click_args;
    //whether or not it has been pressed (falling edge)
    bool pressed;
    //will be activated if this key is pressed too
    uint8_t alternate_key;
    //if this option is enabled, there will not be a rectangle drawn in the unselected BG color when unselected
    //so the background will show behind it
    bool transparent_background;
};

struct optix_click_action {
    void (*click_action)(void *);
    void *click_args;
};

//so this will probably be in an array within the button/menu, and every loop, if it is selected,
//the button/menu will check to see if any of these were pressed and if so run the action
//should be fairly fast, I hope
struct optix_alternate_key_click_action {
    struct optix_click_action click_action;
    uint8_t alternate_key;
};


//functions
//default update/render functions for the buttons
void optix_UpdateButton_default(struct optix_widget *widget);
void optix_RenderButton_default(struct optix_widget *widget);

#endif
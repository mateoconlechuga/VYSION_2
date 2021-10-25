#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

#include "../../optix/gui_control.h"
#include "../../optix/elements/sprite.h"

struct vysion_battery_icon {
    struct optix_sprite sprite;
    uint8_t status;
    bool charging;
    //point this to a clock minutes
    uint8_t last_minutes;
    uint8_t *minutes;
};

void vysion_UpdateBatteryIcon(struct optix_widget *widget);

#endif
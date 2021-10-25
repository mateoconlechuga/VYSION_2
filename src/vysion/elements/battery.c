#include "battery.h"

#include <tice.h>
#include <graphx.h>

#include "../gfx/output/vysion_gfx.h"

void vysion_UpdateBatteryIcon(struct optix_widget *widget) {
    struct optix_sprite *sprite = (struct optix_sprite *) widget;
    struct vysion_battery_icon *battery_icon = (struct vysion_battery_icon *) sprite;
    gfx_sprite_t *spr[] = {battery_0, battery_25, battery_50, battery_75, battery_100};
    //so this will update every minute now
    if (*(battery_icon->minutes) != battery_icon->last_minutes) {
        battery_icon->status = boot_GetBatteryStatus();
        sprite->spr = spr[battery_icon->status];
        widget->state.needs_redraw = true;
    }
    battery_icon->last_minutes = *(battery_icon->minutes);
}
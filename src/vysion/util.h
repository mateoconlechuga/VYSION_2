#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <tice.h>
#include <graphx.h>

uint24_t vysion_util_GetStringHash(char *str);
void vysion_ConvertXlibcToPalette(gfx_sprite_t *spr);

#endif
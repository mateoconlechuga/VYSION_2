#include "util.h"

#include <stdint.h>
#include <graphx.h>
#include <tice.h>

#include "gfx/output/vysion_gfx.h"

//hashing functions (add an option to choose the desired one eventually)
//djb2
uint24_t vysion_util_GetStringHash_djb2(char *str) {
    uint24_t hash = 5381;
    uint24_t c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

//one I made
uint24_t vysion_util_GetStringHash(char *str) {
    uint24_t hash = 0;
    while (*str) hash = (hash << 3) + *str++;
    return hash;
}

//FNV
uint24_t vysion_util_GetStringHash_default(char *str) {
    uint24_t hash = 0x811c9dc5;
    while (*str) {
        hash ^= (char) *str++;
        hash *= 0x01000193;
    }
    return hash;
}

void vysion_ConvertXlibcToPalette(gfx_sprite_t *spr) {
    //I think this is pretty smart
    for (int i = 0; i < spr->width * spr->height; i++) spr->data[i] = xlibc_condensed_data[spr->data[i] + 2];
}
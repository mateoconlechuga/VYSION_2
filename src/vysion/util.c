#include "util.h"

//hashing functions (add an option to choose the desired one eventually)
//djb2
uint24_t vysion_util_GetStringHash(char *str) {
    uint24_t hash = 5381;
    uint24_t c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

//one I made
uint24_t vysion_util_GetStringHash_default(char *str) {
    uint24_t hash = 0;
    while (*str) hash = (hash << 3) + *str++;
    return hash;
}

//FNV
uint24_t vysion_util_GetStringHash_fnv(char *str) {
    uint24_t hash = 0x811c9dc5;
    while (*str) {
        hash ^= (char) *str++;
        hash *= 0x01000193;
    }
    return hash;
}